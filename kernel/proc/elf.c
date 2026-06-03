#include "elf.h"
#include "../../mm/pmm.h"
#include "../../mm/vmm.h"
#include "../../lib/string.h"
#include "../../lib/printk.h"

bool
elf_validate(const void *data, size_t size)
{
    if (size < sizeof(elf64_ehdr_t)) return false;
    const elf64_ehdr_t *hdr = (const elf64_ehdr_t *)data;
    if (*(const uint32_t *)hdr->e_ident != ELF_MAGIC) return false;
    if (hdr->e_ident[4] != 2)       return false;
    if (hdr->e_ident[5] != 1)       return false;
    if (hdr->e_type    != ET_EXEC)  return false;
    if (hdr->e_machine != EM_X86_64) return false;
    return true;
}

elf_info_t
elf_load(uint64_t *pml4, const void *data, size_t size)
{
    elf_info_t info = { .entry = 0, .load_end = 0, .valid = false };
    if (!elf_validate(data, size)) {
        printk(KERN_ERR "elf: invalid binary\n");
        return info;
    }

    const elf64_ehdr_t *hdr  = (const elf64_ehdr_t *)data;
    const uint8_t      *base = (const uint8_t *)data;

    for (int i = 0; i < hdr->e_phnum; i++) {
        const elf64_phdr_t *ph = (const elf64_phdr_t *)
            (base + hdr->e_phoff + (size_t)i * hdr->e_phentsize);

        if (ph->p_type != PT_LOAD) continue;
        if (ph->p_memsz == 0)      continue;

        if (ph->p_vaddr < USER_LOAD_BASE) {
            printk(KERN_ERR "elf: segment below user base 0x%x\n",
                   ph->p_vaddr);
            return info;
        }

        uint64_t vstart = ph->p_vaddr & ~(uint64_t)(PAGE_SIZE - 1);
        uint64_t vend   = (ph->p_vaddr + ph->p_memsz + PAGE_SIZE - 1)
                          & ~(uint64_t)(PAGE_SIZE - 1);

        uint64_t vmm_flags = VMM_PRESENT | VMM_USER;
        if (ph->p_flags & PF_W) vmm_flags |= VMM_WRITABLE;

        for (uint64_t va = vstart; va < vend; va += PAGE_SIZE) {
            void *page = pmm_alloc();
            if (!page) {
                printk(KERN_ERR "elf: out of memory\n");
                return info;
            }
            kmemset(page, 0, PAGE_SIZE);

            if (va >= ph->p_vaddr && ph->p_filesz > 0) {
                uint64_t off  = va - ph->p_vaddr;
                if (off < ph->p_filesz) {
                    size_t copy = PAGE_SIZE;
                    if (copy > ph->p_filesz - off)
                        copy = (size_t)(ph->p_filesz - off);
                    kmemcpy(page, base + ph->p_offset + off, copy);
                }
            } else if (ph->p_vaddr > va) {
                uint64_t off  = ph->p_vaddr - va;
                size_t   copy = PAGE_SIZE - (size_t)off;
                if (copy > ph->p_filesz) copy = (size_t)ph->p_filesz;
                if (copy > 0)
                    kmemcpy((uint8_t *)page + off,
                            base + ph->p_offset, copy);
            }

            vmm_map(pml4, va, (uint64_t)page, vmm_flags);
        }

        if (vend > info.load_end) info.load_end = vend;

        printk(KERN_INFO "elf: loaded segment vaddr=0x%x sz=%u flags=%u\n",
               ph->p_vaddr, (uint64_t)ph->p_memsz, (uint64_t)ph->p_flags);
    }

    info.entry = hdr->e_entry;
    info.valid = true;
    printk(KERN_INFO "elf: entry=0x%x\n", info.entry);
    return info;
}
