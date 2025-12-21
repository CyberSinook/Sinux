[bits 32]

global gdt_long_jump
gdt_long_jump:
    jmp 0x08:gdt_long_jump_done
gdt_long_jump_done:
    ret