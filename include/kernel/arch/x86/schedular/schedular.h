#ifndef SCHEDULAR_H
#define SCHEDULAR_H

#include <stdint.h>

#define PROCESS_WAIT 0
#define PROCESS_READY 1

typedef struct{
    uint16_t process_id;
    uint32_t pd_address;
    uint32_t stack_pointer;
    uint16_t last_node;
    uint16_t next_node;
    uint8_t used;
    uint8_t status;
} process_t;

typedef struct __attribute__((packed)){
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} process_stack_t;

void schedular_initialize_kernel_process(uint16_t process_index, uint32_t eip, uint32_t esp);
void wait_process(uint16_t process_index);
void schedular_init();

#endif