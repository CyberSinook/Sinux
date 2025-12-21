#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

typedef struct
{
    uint32_t size;
    uint32_t type; // 1=free 2=reserved
    uint32_t next_unit;
    uint32_t last_unit;
    uint32_t start;
}__attribute__((packed)) heap_unit;

uint32_t kmalloc(uint32_t size);
void kfree(uint32_t address);

void heap_init();

#endif