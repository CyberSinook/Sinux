#pragma once
#include "types.h"

ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, const void *buf, size_t n);
int     close(int fd);
pid_t   getpid(void);
pid_t   getppid(void);
pid_t   fork(void);
int     execve(const char *path, char *const argv[], char *const envp[]);
void    _exit(int code);
int     pipe(int fds[2]);
int     kill(pid_t pid, int sig);
unsigned int sleep(unsigned int secs);
