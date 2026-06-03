#include "gdt.h"
#include "../../lib/io.h"
#include <stdint.h>

#define MSR_EFER   0xC0000080
#define MSR_STAR   0xC0000081
#define MSR_LSTAR  0xC0000082
#define MSR_FMASK  0xC0000084

extern void syscall_asm_entry(void);

static void wrmsr(uint32_t msr, uint64_t val) {
    __asm__ volatile ("wrmsr"
        :: "c"(msr), "a"((uint32_t)val), "d"((uint32_t)(val>>32)));
}
static uint64_t rdmsr(uint32_t msr) {
    uint32_t lo, hi;
    __asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

void syscall_arch_init(void) {

    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);

    wrmsr(MSR_STAR, ((uint64_t)0x0013 << 48) | ((uint64_t)0x0008 << 32));

    wrmsr(MSR_LSTAR, (uint64_t)syscall_asm_entry);

    wrmsr(MSR_FMASK, 0x200);
}
