#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} IDT_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;   
    uint32_t base;    
} IDT_ptr_t;

extern IDT_entry_t *IDT;

void set_IDT_entry(int num, uint32_t base, uint16_t selector, uint8_t type_attr);
void pic_unmask_irq(uint8_t irq);
void idt_init();

#endif