#include <stdint.h>
#include <lib/multiboot.h>
#include <lib/string.h>

#define PAGE_FREE         0x0
#define PAGE_USED         0x1
#define KERNEL_PAGE_USED  0x2

uint16_t pmm_memory_map_length;
uint32_t pmm_memory_size;
uint32_t pmm_pages_count;
uint32_t pmm_free_pages;

uint8_t *pmm_memory_pages_map = (uint8_t*)0xFFFFFFFF;

void pmm_set_reserved(uint32_t start, uint32_t end){
    if(end > pmm_memory_size){
        while(1){}
    }

    for(uint32_t i = start / 4096; i < (end + 4095) / 4096; i++){
        pmm_memory_pages_map[i] = KERNEL_PAGE_USED;
        pmm_free_pages--;
    }
}

void pmm_set_free(uint32_t start, uint32_t end){
    if(end > pmm_memory_size){
        while(1){}
    }

    for(uint32_t i = start / 4096; i < (end + 4095) / 4096; i++){
        pmm_memory_pages_map[i] = PAGE_FREE;
        pmm_free_pages++;
    }
}

uint32_t pmm_reserve_page(uint32_t count){
    uint32_t frees = 0;
    uint32_t start = 0;

    for(uint32_t i = 0; i < pmm_pages_count; i++){
        if(pmm_memory_pages_map[i] == PAGE_FREE){
            if(frees == 0) start = i;
            frees++;
        }else{
            frees = 0;
        }

        if(frees == count){
            for(uint32_t x = start; x < start + frees; x++){
                pmm_memory_pages_map[x] = PAGE_USED;
            }
            pmm_free_pages -= frees;
            return start * 4096;
        }
    }

    return 0xFFFFFFFF;
}

uint32_t pmm_reserve_kernel_page(uint32_t count){
    uint32_t frees = 0;
    uint32_t start = 0;

    for(uint32_t i = 0; i < pmm_pages_count; i++){
        if(pmm_memory_pages_map[i] == PAGE_FREE){
            if(frees == 0) start = i;
            frees++;
        }else{
            frees = 0;
        }

        if(frees == count){
            for(uint32_t x = start; x < start + frees; x++){
                pmm_memory_pages_map[x] = KERNEL_PAGE_USED;
            }
            pmm_free_pages -= frees;
            return start * 4096;
        }
    }

    return 0xFFFFFFFF;
}

extern char _bss_end;
uint8_t* find_space_for_pmm_map(multiboot_info_t* mbi, uint32_t pmm_pages_count) {
    multiboot_memory_map_t* entry = (multiboot_memory_map_t*)mbi->mmap_addr;

    uint32_t need_bytes = pmm_pages_count;

    uintptr_t bss_end = (uintptr_t)&_bss_end;

    while ((uintptr_t)entry < mbi->mmap_addr + mbi->mmap_length) {

        if (entry->type == 1) {

            uintptr_t start = entry->addr;
            uintptr_t end   = entry->addr + entry->len;

            if (end <= bss_end) {
                entry = (multiboot_memory_map_t*)((uintptr_t)entry + entry->size + sizeof(entry->size));
                continue;
            }

            if (start < bss_end)
                start = bss_end;

            start = (start + 4095) & ~4095;

            if (start + need_bytes <= end) {
                return (uint8_t*)start;
            }
        }

        entry = (multiboot_memory_map_t*)((uintptr_t)entry + entry->size + sizeof(entry->size));
    }

    return NULL;
}

extern char _multiboot_start;
void pmm_init(multiboot_info_t *mbi){
    pmm_memory_map_length = mbi->mmap_length;

    multiboot_memory_map_t *entry = (multiboot_memory_map_t*)mbi->mmap_addr;
    uint32_t max_size = 0;

    while((uintptr_t)entry < mbi->mmap_addr + mbi->mmap_length) {
        uint32_t end = entry->addr + entry->len;
        if(end > max_size){
            max_size = end;
        }
        entry = (multiboot_memory_map_t*)((uintptr_t)entry + entry->size + sizeof(entry->size));
    }

    pmm_memory_size = max_size;

    pmm_pages_count = pmm_memory_size/4096;
    pmm_free_pages = pmm_pages_count;

    pmm_memory_pages_map = find_space_for_pmm_map(mbi, pmm_pages_count);

    memset(pmm_memory_pages_map, 0, pmm_pages_count);

    pmm_set_reserved((uintptr_t)pmm_memory_pages_map, ((uintptr_t)pmm_memory_pages_map)+pmm_pages_count);

    pmm_set_reserved((uintptr_t)&_multiboot_start, (uintptr_t)&_bss_end);

    entry = (multiboot_memory_map_t*)mbi->mmap_addr;
    while((uintptr_t)entry < mbi->mmap_addr + mbi->mmap_length) {
        if(entry->type != 1){
            pmm_set_reserved(entry->addr, entry->addr + entry->len);
        }
        entry = (multiboot_memory_map_t*)((uintptr_t)entry + entry->size + sizeof(entry->size));
    }

    pmm_set_reserved((uintptr_t)mbi, (uintptr_t)mbi + 111);
    pmm_set_reserved((uintptr_t)mbi->mods_addr, (uintptr_t)mbi->mods_count*16);
    multiboot_module_t *mods = (multiboot_module_t*)mbi->mods_addr;
    for(uint32_t i = 0; i < mbi->mods_count; i++){
        pmm_set_reserved(mods[i].mod_start, mods[i].mod_end);
    }
    pmm_set_reserved(0xB8000, 0xB8000+4000);
}