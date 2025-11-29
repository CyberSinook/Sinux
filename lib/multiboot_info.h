#ifndef MULTIBOOT_INFO_H
#define MULTIBOOT_INFO_H
#include <stdint.h>

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info_t;

typedef struct __attribute__((packed)) {
    uint32_t size;   
    uint64_t addr;   
    uint64_t len;    
    uint32_t type;   
} multiboot_memory_map_t;

#endif