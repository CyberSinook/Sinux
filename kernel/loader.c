#include <stdint.h>
#include <lib/multiboot.h>
#include <kernel/arch/x86/mm/pmm.h>
#include <kernel/arch/x86/mm/vmm.h>
#include <kernel/arch/x86/mm/heap.h>
#include <kernel/arch/x86/cpu/gdt.h>
#include <kernel/arch/x86/interrupts/idt.h>
#include <kernel/arch/x86/schedular/schedular.h>
#include <lib/inout.h>
#include <kernel/drivers/ide.h>
#include <kernel/arch/x86/pci/pci.h>
#include <kernel/sys/init.h>

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

/* Global init system */
static init_system_t init_system;

/* Pacman service functions (declared in init.c) */
extern int pacman_service_start(void);
extern int pacman_service_stop(void);
extern int pacman_service_run(void);

void loader(multiboot_info_t *mbi) {
	pmm_init(mbi);
    vmm_init();
    heap_init();

    gdt_init();
    idt_init();

    schedular_init();

    ide_init();
    pci_init();

    asm volatile("sti");

    /* Print boot message */
    const char *boot_msg = "\n=== SINUX OS BOOTING ===\n";
    for (const char *p = boot_msg; *p; p++) {
        outb(0x3F8, *p);
    }
    
    /* Initialize init system */
    init_system_init(&init_system);

    /* Register Pacman game service */
    init_service_t pacman_service = {
        .id = 0,
        .name = "Pacman Game",
        .type = SERVICE_GAME,
        .start = pacman_service_start,
        .stop = pacman_service_stop,
        .state = 0
    };
    init_register_service(&init_system, &pacman_service);

    /* Set Pacman as default service */
    init_set_default_service(&init_system, 0);

    /* Run default service (Pacman game) */
    init_run_default_service(&init_system);

    /* After service completes, return to idle */
    const char *idle_msg = "\n[INIT] Returning to idle mode\n";
    for (const char *p = idle_msg; *p; p++) {
        outb(0x3F8, *p);
    }

    wait_process(1);

    for(;;){
        asm volatile("hlt");
        outb(0x3F8, 'P');
    }
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
