#include <stdint.h>
#include "../../../lib/string.h"


__attribute__((noinline)) void clear_screen();

#define SYSCALL(syscal_number, arg1, arg2) ({ \
    long ret; \
    asm volatile ( \
        "int $0x80" \
        : "=a"(ret) \
        : "a"(syscal_number), "b"(arg1), "c"(arg2) \
        : "memory"); \
    ret; \
})


#define READ_REG(reg, var) \
    asm volatile("mov %%" #reg ", %0" : "=r"(var) :: "memory")


#define WRITE_REG(reg, val) \
    asm volatile("mov %0, %%" #reg :: "r"(val) : "memory")

void handle_syscal(){
    uint32_t eax_val;

    READ_REG(eax, eax_val);
    outb(0x3F8, 'I');

    if(eax_val == 0){
        clear_screen();
        eax_val = 1;
    }

    WRITE_REG(eax, eax_val);
}