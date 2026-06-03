#include "panic.h"
#include "../../drivers/tty.h"
#include "../../lib/printk.h"

void
kernel_panic(const char *msg) {
    tty_setcolor_err();
    printk("\n\n*** KERNEL PANIC: %s ***\n", msg);
    printk("System halted.\n");
    __asm__ volatile ("cli");
    for (;;) __asm__ volatile ("hlt");
}

void
handle_exception(uint64_t vec, uint64_t err) {
    static const char *names[] = {
        "#DE","#DB","NMI","#BP","#OF","#BR",
        "#UD","#NM","#DF","??", "#TS","#NP",
        "#SS","#GP","#PF"
    };
    tty_setcolor_err();
    printk("\n\n[EXCEPTION] vec=%u (%s) err=0x%x\n",
        vec,
        vec < 15 ? names[vec] : "??",
        err);
    if (vec == 14) {
        uint64_t cr2;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
        printk("  CR2 (fault addr): 0x%016x\n", cr2);
    }
    kernel_panic("unhandled exception");
}
