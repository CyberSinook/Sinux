#include "../include/stdlib.h"
#include "../include/types.h"
#include "../include/unistd.h"
#include "../include/syscall.h"

void exit(int code) { _exit(code); }

static uint64_t heap_ptr = 0;

void *malloc(size_t size) {
    if (!heap_ptr) {
        heap_ptr = (uint64_t)syscall(SYS_BRK, 0);
        heap_ptr = (heap_ptr + 0xFFF) & ~0xFFFULL;
    }
    size = (size + 15) & ~(size_t)15;
    void *ptr = (void *)heap_ptr;
    heap_ptr += size;
    syscall(SYS_BRK, heap_ptr);
    return ptr;
}

void free(void *ptr) { (void)ptr; }

int atoi(const char *s) {
    int n = 0, neg = 0;
    while (*s == ' ') s++;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') n = n*10 + (*s++ - '0');
    return neg ? -n : n;
}

long atol(const char *s) { return (long)atoi(s); }
