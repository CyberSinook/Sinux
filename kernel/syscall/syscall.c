#include "syscall.h"
#include "../ipc/pipe.h"
#include "../ipc/signal.h"
#include "../proc/elf.h"
#include "../proc/usermode.h"
#include "../../mm/vmm.h"
#include "../fs/vfs.h"
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../../mm/vmm.h"
#include "../../lib/string.h"
#include "../../lib/printk.h"
#include "../../arch/x86_64/pit.h"
#include <stdint.h>

extern void syscall_arch_init(void);

static int64_t
sys_write(int fd, const void *buf, size_t n)
{
    process_t *p = proc_current();
    if (fd < 0 || fd >= MAX_FDS) return -EBADF;

    if (fd == 1 || fd == 2) {
        const char *s = (const char *)buf;
        for (size_t i = 0; i < n; i++) {
            extern void tty_putc(char);
            tty_putc(s[i]);
        }
        return (int64_t)n;
    }

    if (!p->fds[fd].file) return -EBADF;
    return vfs_write(p->fds[fd].file, buf, n);
}

static int64_t
sys_read(int fd, void *buf, size_t n)
{
    process_t *p = proc_current();
    if (fd < 0 || fd >= MAX_FDS) return -EBADF;

    if (fd == 0) {
        extern char kbd_getc(void);
        char *b = (char *)buf;
        size_t i;
        for (i = 0; i < n; i++) {
            b[i] = kbd_getc();
            if (b[i] == '\n') { i++; break; }
        }
        return (int64_t)i;
    }

    if (!p->fds[fd].file) return -EBADF;
    return vfs_read(p->fds[fd].file, buf, n);
}

static int64_t
sys_open(const char *path, int flags, int mode)
{
    (void)mode;
    process_t *p = proc_current();
    file_t    *f = vfs_open(path, flags);
    if (!f) return -ENOENT;
    int fd = proc_alloc_fd(p, f);
    if (fd < 0) { vfs_close(f); return -EMFILE; }
    return fd;
}

static int64_t
sys_close(int fd)
{
    process_t *p = proc_current();
    if (fd < 0 || fd >= MAX_FDS || !p->fds[fd].file) return -EBADF;
    vfs_close(p->fds[fd].file);
    proc_free_fd(p, fd);
    return 0;
}

static int64_t sys_getpid(void)  { return proc_current()->pid;  }
static int64_t sys_getppid(void) { return proc_current()->ppid; }

static int64_t
sys_brk(uint64_t addr)
{
    process_t *p = proc_current();
    if (addr == 0) return (int64_t)p->brk;
    if (addr < p->brk_start) return -EINVAL;
    p->brk = addr;
    return (int64_t)addr;
}

static int64_t
sys_exit(int code)
{
    proc_exit(proc_current(), code);
    sched_yield();
    for(;;) __asm__ volatile("hlt");
    return 0;
}

static int64_t
sys_fork(void)
{
    process_t *child = proc_create();
    if (!child) return -ENOMEM;

    process_t *p = proc_current();
    for (int i = 0; i < MAX_FDS; i++) {
        child->fds[i] = p->fds[i];
        if (child->fds[i].file)
            child->fds[i].file->refcount++;
    }

    sched_add(child);
    return child->pid;
}

static int64_t
sys_wait4(pid_t pid, int *status, int options)
{
    (void)options;
    process_t *p = proc_current();
    while (1) {
        process_t *child = proc_find(pid);
        if (!child) return -EINVAL;
        if (child->state == PROC_ZOMBIE) {
            if (status) *status = child->exit_code;
            child->state = PROC_UNUSED;
            return pid;
        }
        p->state = PROC_WAITING;
        sched_yield();
    }
}

static int64_t
sys_nanosleep(uint64_t seconds, uint64_t nanoseconds)
{
    uint64_t ms = seconds * 1000 + nanoseconds / 1000000;
    sched_sleep(ms);
    return 0;
}

typedef int64_t (*syscall_fn_t)(uint64_t,uint64_t,uint64_t,
                                 uint64_t,uint64_t,uint64_t);

#define SYSCALL_ENTRY(fn) ((syscall_fn_t)(void*)(fn))

int64_t
syscall_entry(uint64_t nr,
              uint64_t a1, uint64_t a2, uint64_t a3,
              uint64_t a4, uint64_t a5, uint64_t a6)
{
    (void)a4; (void)a5; (void)a6;
    switch (nr) {
    case SYS_READ:    return sys_read((int)a1, (void*)a2, (size_t)a3);
    case SYS_WRITE:   return sys_write((int)a1, (void*)a2, (size_t)a3);
    case SYS_OPEN:    return sys_open((char*)a1, (int)a2, (int)a3);
    case SYS_CLOSE:   return sys_close((int)a1);
    case SYS_BRK:     return sys_brk(a1);
    case SYS_GETPID:  return sys_getpid();
    case SYS_GETPPID: return sys_getppid();
    case SYS_FORK:    return sys_fork();
    case SYS_EXIT:    return sys_exit((int)a1);
    case SYS_WAIT4:   return sys_wait4((pid_t)a1,(int*)a2,(int)a3);
    case SYS_SLEEP:   return sys_nanosleep(a1, a2);
    case SYS_PIPE:    { int fds[2]; int r=pipe_create(fds);
                        if(!r){*(int*)a1=fds[0];*((int*)a1+1)=fds[1];} return r; }
    case SYS_KILL:    return (int64_t)signal_send((int)a1, (int)a2);
    case SYS_SIGACTION: {
        sigaction_t act = { .handler=(sighandler_t)a2, .mask=0, .flags=0 };
        return signal_set_handler((int)a1, &act);
    }
    case SYS_EXECVE: {
        file_t *f = vfs_open((char*)a1, O_RDONLY);
        if (!f) return -ENOENT;
        size_t sz = (size_t)f->inode->size;
        uint8_t *buf = kmalloc(sz);
        if (!buf) { vfs_close(f); return -ENOMEM; }
        vfs_read(f, buf, sz);
        vfs_close(f);
        process_t *p = proc_current();
        if (!p->pml4) p->pml4 = vmm_new_pml4();
        elf_info_t info = elf_load(p->pml4, buf, sz);
        if (!info.valid) return -EINVAL;
        usermode_exec(p, info.entry, info.load_end);
        return 0;
    }
    default:
        printk(KERN_WARNING "unknown syscall %u\n", nr);
        return -EINVAL;
    }
}

void
syscall_init(void) {
    syscall_arch_init();
    printk(KERN_INFO "syscall: SYSCALL/SYSRET enabled\n");
}
