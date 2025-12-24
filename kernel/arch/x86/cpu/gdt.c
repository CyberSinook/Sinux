#include <kernel/arch/x86/cpu/gdt.h>
#include <stdint.h>

GDT_entry_t gdt[3] = {
    {0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00}, // Null segment
    {0xFFFF, 0x0000, 0x00, 0x9A, 0xCF, 0x00}, // Code segment
    {0xFFFF, 0x0000, 0x00, 0x92, 0xCF, 0x00}  // Data segment
};

GDT_Descriptor_t gdt_desc = {
    .limit = sizeof(gdt) - 1,
    .base  = (uint32_t)&gdt
};

void gdt_init(){
    asm volatile ("lgdt (%0)" : : "r" (&gdt_desc) : "memory");

    gdt_long_jump();
}