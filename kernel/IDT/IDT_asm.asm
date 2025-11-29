[bits 32]

GDT_start:
    null_descriptor:
        dd 0
        dd 0
    code_desciptor:
        dw 0xffff
        dw 0
        db 0
        db 0b10011010
        db 0b11001111
        db 0
    data_desciptor:
        dw 0xffff
        dw 0
        db 0
        db 0b10010010
        db 0b11001111
        db 0
GDT_end:

GDT_Descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start

global init_gdt
init_gdt:
    lgdt [GDT_Descriptor]

    ; Flush CS
    jmp 0x08:flush_cs
flush_cs:
    ret

global isr_default_handler
isr_default_handler:
    pushad
    popad
    iretd

extern handle_cursor
extern handle_timer
global irq0_handler
irq0_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    call handle_cursor
    call handle_timer
    popad
    iretd

extern handle_keyboard
global irq1_keyboard_handler
irq1_keyboard_handler:
    pushad
    call handle_keyboard
    popad
    mov al, 0x20
    out 0x20, al
    iretd

extern handle_syscal
global syscal_handler
syscal_handler:
    pushad
    call handle_syscal
    popad
    iretd

extern red_screen_error
global gpf
gpf:
    cli
    pushad
    mov al, 'G'
    mov dx, 0x3F8
    out dx, al

    call red_screen_error
    popad
    iretd