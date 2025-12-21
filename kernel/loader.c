#include <stdint.h>

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

void loader(){
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
