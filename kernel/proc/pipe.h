#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../fs/vfs.h"

#define PIPE_BUF 4096

typedef struct {
    uint8_t  buf[PIPE_BUF];
    size_t   head, tail, count;
    int      readers, writers;
} pipe_t;

int pipe_create(file_t **read_end, file_t **write_end);
