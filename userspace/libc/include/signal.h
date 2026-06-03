#pragma once
#define SIGINT  2
#define SIGKILL 9
#define SIGTERM 15
typedef void (*sighandler_t)(int);
sighandler_t signal(int sig, sighandler_t handler);
