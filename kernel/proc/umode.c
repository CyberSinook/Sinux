#include "umode.h"
#include "process.h"
#include "../../arch/x86_64/gdt.h"

void
umode_enter(process_t *p)
{
    gdt_set_kernel_stack(p->rsp);
    umode_jump(p->entry, p->ustack_top, (uint64_t)p->pml4);
}

__attribute__((naked))
void
umode_jump(uint64_t entry, uint64_t stack, uint64_t cr3)
{
    __asm__ volatile (
        "mov %2, %%cr3\n"
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushq $0x23\n"
        "pushq %1\n"
        "pushq $0x202\n"
        "pushq $0x1B\n"
        "pushq %0\n"
        "iretq\n"
        :: "r"(entry), "r"(stack), "r"(cr3)
    );
}
