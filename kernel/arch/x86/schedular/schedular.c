#include <stdint.h>
#include <kernel/arch/x86/mm/heap.h>
#include <kernel/arch/x86/schedular/schedular.h>
#include <kernel/arch/x86/mm/pmm.h>
#include <kernel/arch/x86/interrupts/idt.h>
#include <lib/inout.h>

#define MAX_PROCESSES 128

process_t *processes;
uint32_t current_process_stack;
uint32_t current_process_cr3;
uint32_t current_process_index;

void schedular_initialize_kernel_process(uint16_t process_index, uint32_t eip, uint32_t esp){
    process_stack_t *pstack = (process_stack_t *)(esp-sizeof(process_stack_t));

    pstack->eip = eip;
    pstack->cs = 0x08;
    pstack->eflags = 0x202;
    pstack->esp_dummy = esp-12;

    processes[process_index].stack_pointer = (uint32_t)pstack;
    processes[process_index].last_node = processes[0].last_node;
    processes[process_index].next_node = 0;
    processes[processes[0].last_node].next_node = process_index;
    processes[0].last_node = process_index;
    processes[process_index].status = PROCESS_READY;
    processes[process_index].used = 1;
}

void schedular_halt(){
    for(;;){
        asm volatile("hlt");
    }
}

void schedule(){
    processes[current_process_index].stack_pointer = (uint32_t)current_process_stack;
    if(processes[current_process_index].next_node == 0 && processes[0].next_node != 0){
        current_process_index = 0;
    }

    current_process_index = processes[current_process_index].next_node;
    current_process_stack = processes[current_process_index].stack_pointer;
}

void wait_process(uint16_t process_index){
    processes[processes[process_index].last_node].next_node = processes[process_index].next_node;
    processes[processes[process_index].next_node].last_node = processes[process_index].last_node;
}

extern void schedular_irq0_interrupt();
void schedular_init(){
    processes = (process_t*)kmalloc(sizeof(process_t) * MAX_PROCESSES);

    uint32_t esp = pmm_reserve_kernel_page(1);

    process_stack_t *pstack = (process_stack_t *)(esp-sizeof(process_stack_t));

    pstack->eip = (uint32_t)schedular_halt;
    pstack->cs = 0x08;
    pstack->eflags = 0x202;
    pstack->esp_dummy = esp-12;

    processes[0].stack_pointer = (uint32_t)pstack;
    processes[0].last_node = 1;
    processes[0].next_node = 1;
    processes[0].status = PROCESS_READY;
    processes[0].used = 1;

    processes[1].last_node = 0;
    processes[1].next_node = 0;
    processes[1].status = PROCESS_READY;
    processes[1].used = 1;
    
    current_process_index = 1;

    set_IDT_entry(32, (uint32_t)schedular_irq0_interrupt, 0x08, 0x8E);
    pic_unmask_irq(0);
}