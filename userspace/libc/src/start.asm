bits 64
extern main
extern exit
global _start

_start:
    xor rbp, rbp
    pop rdi
    mov rsi, rsp
    lea rdx, [rsp + rdi*8 + 8]
    call main
    mov rdi, rax
    call exit
.halt:
    hlt
    jmp .halt
