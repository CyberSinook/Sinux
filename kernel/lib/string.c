#include <stddef.h>
#include <stdint.h>

void *memset(void *dst, int c, size_t n) {
    uint8_t *p = (uint8_t *)dst;
    uint8_t v8 = (uint8_t)c;

    while (n && ((uintptr_t)p & 7)) {
        *p++ = v8;
        n--;
    }

    uint64_t v64 = v8;
    v64 |= v64 << 8;
    v64 |= v64 << 16;
    v64 |= v64 << 32;

    uint64_t *p64 = (uint64_t *)p;
    while (n >= 8) {
        *p64++ = v64;
        n -= 8;
    }

    p = (uint8_t *)p64;
    while (n--) {
        *p++ = v8;
    }

    return dst;
}