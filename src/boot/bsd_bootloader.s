.section .text
.code16
.global _start
.extern _start32

/* BSD-compatible bootloader for Sinux OS */

#define BOOTLOADER_BASE 0x7C00
#define BOOTARGS_BASE   0x7000
#define KERNEL_BASE     0x100000

_start:
    /* Save boot drive */
    mov %dl, boot_drive

    /* Initialize segment registers */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    /* Setup stack */
    mov $0x7000, %sp
    mov $0x7000, %bp

    /* Print welcome message */
    mov $welcome_msg, %si
    call print_string

    /* Enable A20 line */
    call enable_a20
    jc .error_a20

    /* Load kernel from disk */
    mov $kernel_msg, %si
    call print_string

    call load_kernel
    jc .error_load

    /* Prepare BSD boot arguments */
    mov $bootargs_msg, %si
    call print_string

    call prepare_boot_args

    /* Enter protected mode */
    mov $protected_msg, %si
    call print_string

    call enter_protected_mode
    jmp . /* Should not reach here */

.error_a20:
    mov $error_a20_msg, %si
    call print_string
    jmp .

.error_load:
    mov $error_load_msg, %si
    call print_string
    jmp .

/* Print string to console */
print_string:
    push %ax
    push %bx
    mov $0x0E, %ah
    xor %bx, %bx
.print_loop:
    lodsb
    test %al, %al
    jz .print_done
    int $0x10
    jmp .print_loop
.print_done:
    pop %bx
    pop %ax
    ret

/* Enable A20 line using BIOS */
enable_a20:
    /* Try BIOS method first */
    mov $0x2401, %ax
    int $0x15
    jnc .a20_done

    /* Fallback to keyboard controller method */
    call .wait_input
    mov $0xAD, %al
    out %al, $0x64

    call .wait_input
    mov $0xD0, %al
    out %al, $0x64

    call .wait_output
    in $0x60, %al
    mov %al, %bl

    call .wait_input
    mov $0xD1, %al
    out %al, $0x64

    call .wait_input
    mov %bl, %al
    or $0x02, %al
    out %al, $0x60

    call .wait_input
    mov $0xAE, %al
    out %al, $0x64

    xor %ax, %ax
.a20_done:
    ret

.wait_input:
    in $0x64, %al
    test $0x02, %al
    jnz .wait_input
    ret

.wait_output:
    in $0x64, %al
    test $0x01, %al
    jz .wait_output
    ret

/* Load kernel from disk */
load_kernel:
    /* Load kernel.elf from disk */
    mov boot_drive, %dl
    mov $0x02, %ah        /* Read sectors */
    mov $0x01, %al        /* Number of sectors */
    mov $0x00, %ch        /* Cylinder */
    mov $0x02, %cl        /* Sector (after bootloader) */
    mov $0x00, %dh        /* Head */
    mov $0x1000, %bx      /* Segment */
    mov %bx, %es
    xor %bx, %bx          /* Offset */
    int $0x13
    jc .load_error
    xor %ax, %ax
    ret
.load_error:
    mov $0x01, %ax
    stc
    ret

/* Prepare BSD boot arguments structure */
prepare_boot_args:
    mov $BOOTARGS_BASE, %edi

    /* BSD boot info magic */
    mov $0x57494E42, (%edi)  /* 'WINB' magic */
    add $4, %edi

    /* Boot device */
    mov boot_drive, %al
    mov %al, (%edi)
    add $1, %edi

    /* Boot flags */
    mov $0x00, %al
    mov %al, (%edi)
    add $1, %edi

    /* Kernel entry point */
    mov $KERNEL_BASE, (%edi)
    add $4, %edi

    /* Memory info - lower memory */
    mov $0x0413, %ax
    mov (%ax), %ax
    mov %ax, (%edi)
    add $2, %edi

    /* Memory info - upper memory */
    mov $0x0415, %ax
    mov (%ax), %ax
    mov %ax, (%edi)
    add $2, %edi

    /* Extended memory */
    mov $0x88, %ah
    int $0x15
    mov %ax, (%edi)

    ret

/* Enter protected mode and jump to kernel */
enter_protected_mode:
    /* Save boot args pointer */
    mov $BOOTARGS_BASE, %esi

    /* Load GDT */
    lgdt gdt_ptr

    /* Set PE bit in CR0 */
    mov %cr0, %eax
    or $0x00000001, %eax
    mov %eax, %cr0

    /* Jump to 32-bit code */
    ljmp $0x08, $._start32_trampoline

._start32_trampoline:
    .code32
    /* Setup 32-bit segments */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    /* Setup stack in higher memory */
    mov $0x200000, %esp

    /* Boot arguments in EDX */
    mov $BOOTARGS_BASE, %edx

    /* Jump to kernel */
    jmp $KERNEL_BASE

/* Global Descriptor Table */
.align 16
gdt:
    /* Null descriptor */
    .quad 0x0000000000000000

    /* Code descriptor (0x08) */
    .quad 0x00cf9a000000ffff

    /* Data descriptor (0x10) */
    .quad 0x00cf92000000ffff

gdt_ptr:
    .word (gdt_ptr - gdt - 1)
    .long gdt

/* Data section */
boot_drive:
    .byte 0x00

welcome_msg:
    .asciz "\r\nSinux OS BSD Bootloader v1.0\r\n"

kernel_msg:
    .asciz "Loading kernel...\r\n"

bootargs_msg:
    .asciz "Preparing boot arguments...\r\n"

protected_msg:
    .asciz "Entering protected mode...\r\n"

error_a20_msg:
    .asciz "ERROR: Failed to enable A20 line\r\n"

error_load_msg:
    .asciz "ERROR: Failed to load kernel\r\n"

/* Bootloader signature */
.org 0x1FE
.word 0xAA55
