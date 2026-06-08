#include "vmm.h"
#include "pmm.h"
#include "slab.h"
#include "../lib/string.h"

static uint64_t *kernel_pml4 = NULL;
static bool slab_ready = false;

static uint64_t *
get_or_alloc(uint64_t *table, int idx, uint64_t flags)
{
    if (!(table[idx] & VMM_PRESENT)) {
        void *page = pmm_alloc();
        if (!page) return NULL;
        kmemset(page, 0, PAGE_SIZE);
        table[idx] = (uint64_t)page | flags;
    }
    return (uint64_t *)(table[idx] & ~0xFFFULL);
}

void
vmm_map(uint64_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags)
{
    int p4 = (virt >> 39) & 0x1FF;
    int p3 = (virt >> 30) & 0x1FF;
    int p2 = (virt >> 21) & 0x1FF;
    int p1 = (virt >> 12) & 0x1FF;

    uint64_t tbl_flags = VMM_PRESENT | VMM_WRITABLE;
    if (flags & VMM_USER) tbl_flags |= VMM_USER;

    uint64_t *pdpt = get_or_alloc(pml4, p4, tbl_flags);
    uint64_t *pdt  = get_or_alloc(pdpt, p3, tbl_flags);
    uint64_t *pt   = get_or_alloc(pdt,  p2, tbl_flags);
    pt[p1] = (phys & ~0xFFFULL) | (flags & 0xFFF) | VMM_PRESENT;
    if (flags & VMM_NX) pt[p1] |= VMM_NX;

    __asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

void
vmm_unmap(uint64_t *pml4, uint64_t virt)
{
    int p4 = (virt >> 39) & 0x1FF;
    int p3 = (virt >> 30) & 0x1FF;
    int p2 = (virt >> 21) & 0x1FF;
    int p1 = (virt >> 12) & 0x1FF;

    if (!(pml4[p4] & VMM_PRESENT)) return;
    uint64_t *pdpt = (uint64_t *)(pml4[p4] & ~0xFFFULL);
    if (!(pdpt[p3] & VMM_PRESENT)) return;
    uint64_t *pdt  = (uint64_t *)(pdpt[p3] & ~0xFFFULL);
    if (!(pdt[p2]  & VMM_PRESENT)) return;
    uint64_t *pt   = (uint64_t *)(pdt[p2]  & ~0xFFFULL);
    pt[p1] = 0;
    __asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

uint64_t
vmm_get_phys(uint64_t *pml4, uint64_t virt)
{
    int p4 = (virt >> 39) & 0x1FF;
    int p3 = (virt >> 30) & 0x1FF;
    int p2 = (virt >> 21) & 0x1FF;
    int p1 = (virt >> 12) & 0x1FF;

    if (!(pml4[p4] & VMM_PRESENT)) return 0;
    uint64_t *pdpt = (uint64_t *)(pml4[p4] & ~0xFFFULL);
    if (!(pdpt[p3] & VMM_PRESENT)) return 0;
    uint64_t *pdt  = (uint64_t *)(pdpt[p3] & ~0xFFFULL);
    if (!(pdt[p2]  & VMM_PRESENT)) return 0;
    uint64_t *pt   = (uint64_t *)(pdt[p2]  & ~0xFFFULL);
    return pt[p1] & ~0xFFFULL;
}

void
vmm_map_range(uint64_t *pml4, uint64_t virt, uint64_t phys,
              size_t size, uint64_t flags)
{
    for (size_t off = 0; off < size; off += PAGE_SIZE)
        vmm_map(pml4, virt + off, phys + off, flags);
}

uint64_t *
vmm_kernel_pml4(void) { return kernel_pml4; }

uint64_t *
vmm_current(void)
{
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    return (uint64_t *)(cr3 & ~0xFFFULL);
}

void
vmm_switch(uint64_t *pml4)
{
    __asm__ volatile ("mov %0, %%cr3" :: "r"((uint64_t)pml4) : "memory");
}

uint64_t *
vmm_new_pml4(void)
{
    uint64_t *pml4 = pmm_alloc();
    if (!pml4) return NULL;
    kmemset(pml4, 0, PAGE_SIZE);

    if (kernel_pml4) {
        for (int i = 256; i < 512; i++)
            pml4[i] = kernel_pml4[i];
    }
    return pml4;
}

void
vmm_destroy_pml4(uint64_t *pml4)
{
    if (!pml4) return;
    for (int p4 = 0; p4 < 256; p4++) {
        if (!(pml4[p4] & VMM_PRESENT)) continue;
        uint64_t *pdpt = (uint64_t *)(pml4[p4] & ~0xFFFULL);
        for (int p3 = 0; p3 < 512; p3++) {
            if (!(pdpt[p3] & VMM_PRESENT)) continue;
            uint64_t *pdt = (uint64_t *)(pdpt[p3] & ~0xFFFULL);
            for (int p2 = 0; p2 < 512; p2++) {
                if (!(pdt[p2] & VMM_PRESENT)) continue;
                uint64_t *pt = (uint64_t *)(pdt[p2] & ~0xFFFULL);
                for (int p1 = 0; p1 < 512; p1++) {
                    if (pt[p1] & VMM_PRESENT)
                        pmm_free((void *)(pt[p1] & ~0xFFFULL));
                }
                pmm_free(pt);
            }
            pmm_free(pdt);
        }
        pmm_free(pdpt);
    }
    pmm_free(pml4);
}

void
vmm_init(void)
{
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    kernel_pml4 = (uint64_t *)(cr3 & ~0xFFFULL);
    
    slab_init();
    slab_ready = true;
}

void *
kmalloc(size_t size)
{
    if (slab_ready) {
        return kmalloc_slab(size);
    } else {
        static uint8_t *heap_cur = NULL;
        static size_t heap_left = 0;
        
        size = (size + 15) & ~(size_t)15;
        if (heap_left < size) {
            size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
            heap_cur  = pmm_alloc();
            heap_left = pages * PAGE_SIZE;
            if (!heap_cur) return NULL;
            for (size_t i = 1; i < pages; i++) {
                void *extra = pmm_alloc();
                if (!extra) { heap_cur = NULL; heap_left = 0; return NULL; }
            }
        }
        void *ptr = heap_cur;
        heap_cur  += size;
        heap_left -= size;
        return ptr;
    }
}

void *
kmalloc_zero(size_t size)
{
    void *p = kmalloc(size);
    if (p) kmemset(p, 0, size);
    return p;
}

void
kfree(void *ptr)
{
    if (!ptr) return;
    if (slab_ready) {
        kfree_slab(ptr);
    }
}
