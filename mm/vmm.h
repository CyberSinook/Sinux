#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VMM_PRESENT   (1ULL << 0)
#define VMM_WRITABLE  (1ULL << 1)
#define VMM_USER      (1ULL << 2)
#define VMM_NX        (1ULL << 63)
#define VMM_HUGE      (1ULL << 7)

#define KERNEL_PHYS_BASE  0x100000ULL
#define KERNEL_VIRT_BASE  0xFFFFFFFF80000000ULL
#define KERNEL_MAP_SIZE   (64 * 1024 * 1024)

#define USER_LOAD_BASE    0x400000ULL
#define USER_STACK_TOP    0x7FFFFFF00000ULL
#define USER_STACK_SIZE   (2 * 1024 * 1024)

void      vmm_init(void);
uint64_t *vmm_kernel_pml4(void);
uint64_t *vmm_new_pml4(void);
void      vmm_destroy_pml4(uint64_t *pml4);

void      vmm_map(uint64_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void      vmm_unmap(uint64_t *pml4, uint64_t virt);
uint64_t  vmm_get_phys(uint64_t *pml4, uint64_t virt);

void      vmm_map_range(uint64_t *pml4, uint64_t virt, uint64_t phys,
                        size_t size, uint64_t flags);

void      vmm_switch(uint64_t *pml4);
uint64_t *vmm_current(void);

void  *kmalloc(size_t size);
void  *kmalloc_zero(size_t size);
void   kfree(void *ptr);
