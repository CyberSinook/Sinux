#include <stdint.h>
#include <lib/multiboot.h>
#include <kernel/arch/x86/mm/pmm.h>
#include <kernel/arch/x86/mm/heap.h>
#include <kernel/arch/x86/cpu/gdt.h>
#include <kernel/arch/x86/interrupts/idt.h>

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000007
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_HEADER_MAGIC,
    MULTIBOOT_HEADER_FLAGS,
    CHECKSUM,
    0,
    0,
    0,
    0,
    0,
    0,    // type 0=vbe 1=text mode
    1024, // width
    768,  // height
    32    // depth
};

uint32_t stack[1024];

void loader(multiboot_info_t *mbi) {
	pmm_init(mbi);
    heap_init();

    gdt_init();
    idt_init();

    asm volatile("sti");
}

__attribute__((naked)) void _start() {
    asm volatile (
        "mov $stack + 4096, %esp\n"
        "push %ebx\n"
        "call loader\n"
        "cli\n"
        "hlt\n"
    );
}
