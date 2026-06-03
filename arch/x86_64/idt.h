#pragma once
#include <stdint.h>

void idt_init(void);
void idt_set_gate(int n, uint64_t handler, uint8_t attr);

#define IDT_INT_GATE  0x8E
#define IDT_TRAP_GATE 0x8F
