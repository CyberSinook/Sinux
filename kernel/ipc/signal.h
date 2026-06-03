#pragma once
#include <stdint.h>

#define SIGHUP   1
#define SIGINT   2
#define SIGQUIT  3
#define SIGKILL  9
#define SIGSEGV  11
#define SIGPIPE  13
#define SIGTERM  15
#define SIGCHLD  17
#define SIGSTOP  19
#define NSIG     32

typedef void (*sighandler_t)(int);
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)

typedef struct {
    sighandler_t handler;
    uint64_t     mask;
    int          flags;
} sigaction_t;

int  signal_send(int pid, int sig);
void signal_deliver(void);
int  signal_set_handler(int sig, sigaction_t *act);
