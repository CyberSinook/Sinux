#ifndef VMM_H
#define VMM_H

#include <stdint.h>

typedef union {
    uint32_t value;

    struct {
        uint32_t present           : 1;
        uint32_t rw                : 1;
        uint32_t user              : 1;
        uint32_t write_through     : 1;
        uint32_t cache_disable     : 1;
        uint32_t accessed          : 1;
        uint32_t reserved          : 1;
        uint32_t page_size         : 1;
        uint32_t global            : 1;
        uint32_t available         : 3;
        uint32_t frame             : 20;
    } __attribute__((packed)) bits;

} PDE32;

typedef union {
    struct {
        uint32_t present    : 1;
        uint32_t rw         : 1;
        uint32_t user       : 1;
        uint32_t pwt        : 1;
        uint32_t pcd        : 1;
        uint32_t accessed   : 1;
        uint32_t dirty      : 1;
        uint32_t pat        : 1;
        uint32_t global     : 1;
        uint32_t available  : 3;
        uint32_t frame      : 20;
    } __attribute__((packed)) bits;
    uint32_t value;
} PTE32;

void vmm_init();

#endif