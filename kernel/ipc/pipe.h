#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../fs/vfs.h"

#define PIPE_BUF_SIZE 4096

typedef struct pipe {
    uint8_t  buf[PIPE_BUF_SIZE];
    size_t   head;
    size_t   tail;
    size_t   count;
    bool     read_closed;
    bool     write_closed;
    int      refcount;
} pipe_t;

int    pipe_create(int fds[2]);
pipe_t *pipe_alloc(void);
