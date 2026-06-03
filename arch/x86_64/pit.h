#pragma once
#include <stdint.h>

#define PIT_HZ 1000

void     pit_init(void);
uint64_t pit_ticks(void);
uint64_t pit_uptime_ms(void);
void     pit_sleep_ms(uint64_t ms);
