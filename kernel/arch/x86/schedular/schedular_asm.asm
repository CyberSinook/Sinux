[bits 32]

global schedular_irq0_interrupt

extern schedule
extern current_process_stack
schedular_irq0_interrupt:
    cli
    pushad
    mov al, 0x20
    out 0x20, al

    mov [current_process_stack], esp
    call schedule
    mov esp, [current_process_stack]

    popad
    sti
    iretd

schedular_software_interrupt:
    cli
    pushad

    mov [current_process_stack], esp
    call schedule
    mov esp, [current_process_stack]

    popad
    sti
    iretd