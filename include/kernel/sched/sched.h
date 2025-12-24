#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

#define MAX_PROCESSES 16
#define STACK_SIZE    1024

typedef enum { PROCESS_READY, PROCESS_RUNNING, PROCESS_TERMINATED } process_state_t;

typedef struct {
    int pid;
    process_state_t state;
    void (*entry)();       
    uint8_t stack[STACK_SIZE];
    uint32_t sp;           
} process_t;

extern process_t process_table[MAX_PROCESSES];
extern int current_pid;
extern int next_pid;

int create_process(void (*entry)());
void exit_process();
void yield();
void schedule();

#endif
