.section .multiboot
.align 4
.long 0x1BADB002
.long 0x00000007
.long -(0x1BADB002 + 0x00000007)
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 1024
.long 768
.long 32

.section .text
.global _start
.type _start, @function
_start:
    mov $0x200000, %esp
    push %ebx
    push %eax
    call kernelMain
    cli
    hlt
