#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} GDT_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) GDT_Descriptor_t;

void gdt_long_jump();
void gdt_init();

#endif