#pragma once
#include <stdint.h>

static inline void outb(uint16_t p, uint8_t v) {
    __asm__ volatile ("outb %0,%1" :: "a"(v),"Nd"(p) : "memory");
}
static inline uint8_t inb(uint16_t p) {
    uint8_t v;
    __asm__ volatile ("inb %1,%0" : "=a"(v) : "Nd"(p) : "memory");
    return v;
}
static inline void io_wait(void) { outb(0x80, 0); }

static inline void outw(uint16_t p, uint16_t v) {
    __asm__ volatile ("outw %0,%1" :: "a"(v),"Nd"(p) : "memory");
}
static inline uint16_t inw(uint16_t p) {
    uint16_t v;
    __asm__ volatile ("inw %1,%0" : "=a"(v) : "Nd"(p) : "memory");
    return v;
}
