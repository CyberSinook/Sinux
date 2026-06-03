#pragma once
#include "types.h"

#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_BRK     12
#define SYS_PIPE    22
#define SYS_GETPID  39
#define SYS_FORK    57
#define SYS_EXECVE  59
#define SYS_EXIT    60
#define SYS_WAIT4   61
#define SYS_KILL    62
#define SYS_GETPPID 110
#define SYS_SLEEP   162

int64_t syscall(uint64_t nr, ...);
