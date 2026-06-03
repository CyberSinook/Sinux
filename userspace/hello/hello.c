#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("Hello from userspace!\n");
    printf("PID: %d\n", (int)getpid());
    printf("argc: %d\n", argc);
    return 0;
}
