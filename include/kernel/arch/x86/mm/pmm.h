#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <lib/multiboot.h>

extern uint16_t pmm_memory_map_length;
extern uint32_t pmm_memory_size;
extern uint32_t pmm_pages_count;
extern uint32_t pmm_free_pages;

void pmm_set_reserved(uint32_t start, uint32_t end);
void pmm_set_free(uint32_t start, uint32_t end);
uint32_t pmm_reserve_page(uint32_t count);
uint32_t pmm_reserve_kernel_page(uint32_t count);
void pmm_init(multiboot_info_t *mbi);

#endif