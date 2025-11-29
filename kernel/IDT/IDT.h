#ifndef IDT_H
#define IDT_H
#include <stdint.h>

struct IDT_entry {
    uint16_t offset_low;    
    uint16_t selector;      
    uint8_t  zero;          
    uint8_t  type_attr;     
    uint16_t offset_high;   
} __attribute__((packed));

struct IDT_ptr {
    uint16_t limit;   
    uint32_t base;    
} __attribute__((packed));

void init_IDT(void);

void set_IDT_entry(int num, uint32_t base, uint16_t selector, uint8_t type_attr);

#endif 