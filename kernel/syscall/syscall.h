#pragma once
#include <stdint.h>

#define SYS_READ     0
#define SYS_WRITE    1
#define SYS_OPEN     2
#define SYS_CLOSE    3
#define SYS_STAT     4
#define SYS_BRK      12
#define SYS_GETPID   39
#define SYS_FORK     57
#define SYS_EXECVE   59
#define SYS_EXIT     60
#define SYS_WAIT4    61
#define SYS_GETPPID  110
#define SYS_SLEEP    162  

void    syscall_init(void);
int64_t syscall_entry(uint64_t nr,
                      uint64_t a1, uint64_t a2, uint64_t a3,
                      uint64_t a4, uint64_t a5, uint64_t a6);

#define SYS_PIPE     22
#define SYS_KILL     62
#define SYS_SIGACTION 13
