#pragma once
#include "process.h"

void sched_init(void);
void sched_add(process_t *p);
void sched_remove(process_t *p);
void sched_tick(void);      
void sched_yield(void);     
void sched_sleep(uint64_t ms);
void sched_wake(process_t *p);
