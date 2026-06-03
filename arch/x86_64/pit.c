#include "pit.h"
#include "pic.h"
#include "../../lib/io.h"
#include <stdint.h>

static volatile uint64_t ticks = 0;

void handle_pit(void) {
    ticks++;

    extern void sched_tick(void);
    sched_tick();
    pic_eoi(0);
}

uint64_t pit_ticks(void)     { return ticks; }
uint64_t pit_uptime_ms(void) { return ticks * (1000 / PIT_HZ); }

void pit_sleep_ms(uint64_t ms) {
    uint64_t end = ticks + ms * PIT_HZ / 1000;
    while (ticks < end) __asm__ volatile ("pause");
}

void pit_init(void) {
    uint16_t div = (uint16_t)(1193182 / PIT_HZ);
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(div & 0xFF));
    outb(0x40, (uint8_t)(div >> 8));
    pic_unmask(0);
}
