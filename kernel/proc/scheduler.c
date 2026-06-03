#include "scheduler.h"
#include "process.h"
#include "../../arch/x86_64/pit.h"
#include "../../lib/string.h"

#define SCHED_QUEUE_SIZE 64

static process_t *queue[SCHED_QUEUE_SIZE];
static int        q_head = 0, q_tail = 0, q_count = 0;

extern void arch_switch(uint64_t *old_rsp, uint64_t new_rsp, uint64_t cr3);
extern void proc_set_current(process_t *p);

void
sched_init(void) {
    q_head = q_tail = q_count = 0;
}

void
sched_add(process_t *p) {
    if (q_count >= SCHED_QUEUE_SIZE) return;
    queue[q_tail] = p;
    q_tail = (q_tail + 1) % SCHED_QUEUE_SIZE;
    q_count++;
}

void
sched_remove(process_t *p) {
    for (int i = 0; i < SCHED_QUEUE_SIZE; i++) {
        if (queue[i] == p) { queue[i] = NULL; q_count--; return; }
    }
}

static process_t *
next_ready(void) {
    for (int i = 0; i < SCHED_QUEUE_SIZE; i++) {
        int idx = (q_head + i) % SCHED_QUEUE_SIZE;
        if (queue[idx] && queue[idx]->state == PROC_READY) {
            q_head = (idx + 1) % SCHED_QUEUE_SIZE;
            return queue[idx];
        }
    }
    return NULL;
}

static void
do_switch(process_t *from, process_t *to) {
    if (from == to) return;
    from->state = (from->state == PROC_RUNNING) ? PROC_READY : from->state;
    to->state   = PROC_RUNNING;
    to->timeslice = 5;
    proc_set_current(to);
    uint64_t cr3 = to->pml4 ? (uint64_t)to->pml4 : 0;
    arch_switch(&from->rsp, to->rsp, cr3);
}

void
sched_tick(void) {
    process_t *cur = proc_current();
    if (!cur) return;

    for (int i = 0; i < SCHED_QUEUE_SIZE; i++) {
        process_t *p = queue[i];
        if (p && p->state == PROC_SLEEPING && pit_ticks() >= p->ticks)
            p->state = PROC_READY;
    }

    if (--cur->timeslice > 0) return;  
    sched_yield();
}

void
sched_yield(void) {
    process_t *cur  = proc_current();
    process_t *next = next_ready();
    if (!next || next == cur) {
        if (cur->state == PROC_RUNNING) cur->timeslice = 5;
        return;
    }
    do_switch(cur, next);
}

void
sched_sleep(uint64_t ms) {
    process_t *cur = proc_current();
    cur->ticks = pit_ticks() + ms * PIT_HZ / 1000;
    cur->state = PROC_SLEEPING;
    sched_yield();
}

void
sched_wake(process_t *p) {
    if (p->state == PROC_SLEEPING || p->state == PROC_WAITING)
        p->state = PROC_READY;
}
