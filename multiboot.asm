section .multiboot
    align 4
multiboot_header:
    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_space: resb 8192
stack_top:
