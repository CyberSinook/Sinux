#include "process.h"
#include "../../mm/pmm.h"
#include "../../mm/vmm.h"
#include "../../lib/string.h"
#include "../../lib/printk.h"

static process_t proc_table[MAX_PROCS];
static process_t *current_proc = NULL;
static pid_t      next_pid = 1;

void
proc_init(void)
{
    kmemset(proc_table, 0, sizeof(proc_table));
    proc_table[0].pid       = 0;
    proc_table[0].state     = PROC_RUNNING;
    proc_table[0].timeslice = 5;
    proc_table[0].pml4      = (uint64_t *)vmm_kernel_pml4();
    kstrcpy(proc_table[0].cwd, "/");
    current_proc = &proc_table[0];
}

process_t *
proc_create(void)
{
    for (int i = 1; i < MAX_PROCS; i++) {
        if (proc_table[i].state != PROC_UNUSED) continue;

        process_t *p = &proc_table[i];
        kmemset(p, 0, sizeof(process_t));

        p->pid       = next_pid++;
        p->ppid      = current_proc ? current_proc->pid : 0;
        p->state     = PROC_READY;
        p->timeslice = 5;

        p->kstack = pmm_alloc();
        if (!p->kstack) return NULL;
        kmemset(p->kstack, 0, PAGE_SIZE);
        p->rsp = (uint64_t)(p->kstack + KSTACK_SIZE);

        p->pml4 = vmm_new_pml4();
        if (!p->pml4) { pmm_free(p->kstack); return NULL; }

        kstrcpy(p->cwd, current_proc ? current_proc->cwd : "/");

        for (int j = 0; j < 3; j++)
            p->fds[j].file = NULL;

        return p;
    }
    return NULL;
}

void
proc_exit(process_t *p, int code)
{
    p->exit_code = code;
    p->state     = PROC_ZOMBIE;

    for (int i = 0; i < MAX_FDS; i++)
        if (p->fds[i].file) proc_free_fd(p, i);

    if (p->pml4 && p->pml4 != (uint64_t *)vmm_kernel_pml4()) {
        vmm_destroy_pml4(p->pml4);
        p->pml4 = NULL;
    }

    printk(KERN_INFO "proc: pid %d exited with %d\n",
           (uint64_t)p->pid, (uint64_t)code);
}

process_t *proc_find(pid_t pid) {
    for (int i = 0; i < MAX_PROCS; i++)
        if (proc_table[i].pid == pid &&
            proc_table[i].state != PROC_UNUSED)
            return &proc_table[i];
    return NULL;
}

process_t *proc_current(void)       { return current_proc; }
void       proc_set_current(process_t *p) { current_proc = p; }

int
proc_alloc_fd(process_t *p, struct file *f)
{
    for (int i = 0; i < MAX_FDS; i++) {
        if (!p->fds[i].file) {
            p->fds[i].file  = f;
            p->fds[i].flags = 0;
            return i;
        }
    }
    return -1;
}

void
proc_free_fd(process_t *p, int fd)
{
    if (fd >= 0 && fd < MAX_FDS) {
        p->fds[fd].file  = NULL;
        p->fds[fd].flags = 0;
    }
}
