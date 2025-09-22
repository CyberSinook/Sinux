#include <stdint.h>

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0xe85250d6,
    0,
    16,
    -(0xe85250d6 + 0 + 16)
};


void kernel_main() {
    while (1) {}
}
