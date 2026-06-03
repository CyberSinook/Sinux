#include "usermode.h"
#include "../../mm/pmm.h"
#include "../../mm/vmm.h"
#include "../../lib/string.h"
#include "../../arch/x86_64/gdt.h"
#include "../../lib/printk.h"

extern void _enter_usermode(uint64_t entry, uint64_t rsp);

void
usermode_map_stack(uint64_t *pml4)
{
    uint64_t stack_bottom = USER_STACK_TOP - USER_STACK_SIZE;

    for (uint64_t va = stack_bottom; va < USER_STACK_TOP; va += PAGE_SIZE) {
        void *page = pmm_alloc();
        if (!page) return;
        kmemset(page, 0, PAGE_SIZE);
        vmm_map(pml4, va,
                (uint64_t)page,
                VMM_PRESENT | VMM_WRITABLE | VMM_USER);
    }
}

void
usermode_exec(process_t *proc, uint64_t entry, uint64_t load_end)
{
    if (!proc->pml4) return;

    usermode_map_stack(proc->pml4);

    proc->brk_start = (load_end + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);
    proc->brk       = proc->brk_start;

    gdt_set_kernel_stack((uint64_t)(proc->kstack + KSTACK_SIZE));

    vmm_switch(proc->pml4);

    uint64_t user_rsp = USER_STACK_TOP - 16;

    printk(KERN_INFO "usermode: jumping to entry=0x%x rsp=0x%x\n",
           entry, user_rsp);

    _enter_usermode(entry, user_rsp);
}
