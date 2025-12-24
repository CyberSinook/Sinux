#include "../../include/kernel/sched/sched.h"
#include "../../include/lib/string.h"

process_t process_table[MAX_PROCESSES];
int current_pid = -1;
int next_pid = 0;

int create_process(void (*entry)()) {
    if (next_pid >= MAX_PROCESSES) return -1;

    process_t *p = &process_table[next_pid];
    p->pid = next_pid;
    p->state = PROCESS_READY;
    p->entry = entry;
    p->sp = (uint32_t)(p->stack + STACK_SIZE - 4);
    next_pid++;
    return p->pid;
}

void exit_process() {
    if (current_pid < 0) return;
    process_table[current_pid].state = PROCESS_TERMINATED;
    schedule();
}

void yield() {
    if (current_pid >= 0 && process_table[current_pid].state == PROCESS_RUNNING)
        process_table[current_pid].state = PROCESS_READY;
    schedule();
}

void schedule() {
    int start = (current_pid + 1) % next_pid;
    int found = 0;

    for (int i = 0; i < next_pid; i++) {
        int pid = (start + i) % next_pid;
        if (process_table[pid].state == PROCESS_READY) {
            current_pid = pid;
            process_table[pid].state = PROCESS_RUNNING;
            found = 1;
            break;
        }
    }

    if (!found) {
    
        while(1);
    }

    if (process_table[current_pid].entry)
        process_table[current_pid].entry();
}
