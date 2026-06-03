#include "gdt.h"
#include "../../lib/string.h"
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t limit_low, base_low;
    uint8_t  base_mid, access, flags_limit, base_high;
} gdt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit_low, base_low;
    uint8_t  base_mid, access, flags_limit, base_high;
    uint32_t base_upper, _zero;
} gdt_tss_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} gdtr_t;

typedef struct __attribute__((packed)) {
    uint32_t _res0;
    uint64_t rsp0, rsp1, rsp2;
    uint64_t _res1;
    uint64_t ist[7];
    uint64_t _res2;
    uint16_t _res3, iopb_offset;
} tss_t;

static gdt_entry_t gdt[7];
static gdtr_t      gdtr;
static tss_t       tss;
static uint8_t     kernel_stack[8192] __attribute__((aligned(16)));

static void
gdt_set(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    gdt[i].limit_low   = (uint16_t)(limit & 0xFFFF);
    gdt[i].base_low    = (uint16_t)(base  & 0xFFFF);
    gdt[i].base_mid    = (uint8_t)((base  >> 16) & 0xFF);
    gdt[i].access      = access;
    gdt[i].flags_limit = (uint8_t)((flags << 4) | ((limit >> 16) & 0xF));
    gdt[i].base_high   = (uint8_t)((base  >> 24) & 0xFF);
}

static void
gdt_set_tss_entry(int i, uint64_t base, uint32_t limit)
{
    gdt_tss_entry_t *e = (gdt_tss_entry_t *)&gdt[i];
    e->limit_low   = (uint16_t)(limit & 0xFFFF);
    e->base_low    = (uint16_t)(base  & 0xFFFF);
    e->base_mid    = (uint8_t)((base  >> 16) & 0xFF);
    e->access      = 0x89;
    e->flags_limit = (uint8_t)((limit >> 16) & 0xF);
    e->base_high   = (uint8_t)((base  >> 24) & 0xFF);
    e->base_upper  = (uint32_t)(base  >> 32);
    e->_zero       = 0;
}

void
gdt_set_kernel_stack(uint64_t rsp0)
{
    tss.rsp0 = rsp0;
}

void
gdt_init(void)
{
    kmemset(gdt, 0, sizeof(gdt));
    kmemset(&tss, 0, sizeof(tss));

    gdt_set(0, 0, 0,       0x00, 0x0); 
    gdt_set(1, 0, 0xFFFFF, 0x9A, 0xA); 
    gdt_set(2, 0, 0xFFFFF, 0x92, 0xC); 
    gdt_set(3, 0, 0xFFFFF, 0xFA, 0xA); 
    gdt_set(4, 0, 0xFFFFF, 0xF2, 0xC); 

    tss.rsp0       = (uint64_t)(kernel_stack + sizeof(kernel_stack));
    tss.ist[0]     = (uint64_t)(kernel_stack + sizeof(kernel_stack));
    tss.iopb_offset = (uint16_t)sizeof(tss_t);
    gdt_set_tss_entry(5, (uint64_t)&tss, (uint32_t)(sizeof(tss_t) - 1));

    gdtr.limit = (uint16_t)(sizeof(gdt) - 1);
    gdtr.base  = (uint64_t)gdt;

    __asm__ volatile (
        "lgdt %0\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "pushq $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        :: "m"(gdtr) : "rax", "memory"
    );
    __asm__ volatile ("ltr %0" :: "r"((uint16_t)GDT_TSS_SEL) : "memory");
}
