#include "procfs.h"
#include "vfs.h"
#include "../proc/process.h"
#include "../../arch/x86_64/pit.h"
#include "../../mm/pmm.h"
#include "../../mm/vmm.h"
#include "../../lib/string.h"
#include "../../lib/printk.h"

#define PROC_BUF 512

typedef struct {
    int   type;
    int   pid;
} proc_priv_t;

enum { PT_ROOT=0, PT_VERSION, PT_MEMINFO, PT_UPTIME, PT_PID_DIR, PT_PID_STATUS };

static int64_t
proc_read(file_t *f, void *buf, size_t n)
{
    proc_priv_t *pp = (proc_priv_t *)f->inode->private;
    char tmp[PROC_BUF];
    int  len = 0;

    switch (pp->type) {
    case PT_VERSION:
        len = 64;
        kstrcpy(tmp, "Sinux version 2.0 (x86_64)\n");
        len = (int)kstrlen(tmp);
        break;

    case PT_MEMINFO: {
        uint64_t total = (uint64_t)pmm_total_pages() * PAGE_SIZE / 1024;
        uint64_t free  = (uint64_t)pmm_free_pages()  * PAGE_SIZE / 1024;
        kstrcpy(tmp, "MemTotal: ");
        len = (int)kstrlen(tmp);
        (void)total; (void)free;
        break;
    }

    case PT_UPTIME: {
        uint64_t ms = pit_uptime_ms();
        (void)ms;
        kstrcpy(tmp, "uptime: ");
        len = (int)kstrlen(tmp);
        break;
    }

    case PT_PID_STATUS: {
        process_t *p = proc_find((pid_t)pp->pid);
        if (!p) return -ENOENT;
        static const char *states[] = {"unused","running","ready","sleeping","zombie","waiting"};
        kstrcpy(tmp, "Name: sinux\nState: ");
        kstrcpy(tmp + kstrlen(tmp),
            p->state < 6 ? states[p->state] : "?");
        kstrcpy(tmp + kstrlen(tmp), "\n");
        len = (int)kstrlen(tmp);
        break;
    }

    default:
        return 0;
    }

    if ((size_t)f->pos >= (size_t)len) return 0;
    size_t avail = (size_t)(len - f->pos);
    if (n > avail) n = avail;
    kmemcpy(buf, tmp + f->pos, n);
    f->pos += (int64_t)n;
    return (int64_t)n;
}

static inode_t *
make_proc_inode(int type, int pid, file_type_t ft)
{
    inode_t     *ino  = kmalloc(sizeof(inode_t));
    proc_priv_t *priv = kmalloc(sizeof(proc_priv_t));
    extern fs_ops_t procfs_ops;
    if (!ino || !priv) return NULL;
    kmemset(ino, 0, sizeof(inode_t));
    priv->type = type; priv->pid = pid;
    ino->type = ft; ino->private = priv; ino->ops = &procfs_ops;
    return ino;
}

static inode_t *
procfs_lookup(inode_t *dir, const char *name)
{
    proc_priv_t *pp = (proc_priv_t *)dir->private;

    if (pp->type == PT_ROOT) {
        if (!kstrcmp(name, "version"))  return make_proc_inode(PT_VERSION, 0, FT_REG);
        if (!kstrcmp(name, "meminfo"))  return make_proc_inode(PT_MEMINFO, 0, FT_REG);
        if (!kstrcmp(name, "uptime"))   return make_proc_inode(PT_UPTIME,  0, FT_REG);
        int pid = 0;
        const char *s = name;
        while (*s >= '0' && *s <= '9') pid = pid*10 + (*s++ - '0');
        if (!*s && pid > 0 && proc_find((pid_t)pid))
            return make_proc_inode(PT_PID_DIR, pid, FT_DIR);
    }

    if (pp->type == PT_PID_DIR) {
        if (!kstrcmp(name, "status"))
            return make_proc_inode(PT_PID_STATUS, pp->pid, FT_REG);
    }
    return NULL;
}

fs_ops_t procfs_ops = {
    .read   = proc_read,
    .lookup = procfs_lookup,
};

int
procfs_mount(const char *path)
{
    inode_t *root = make_proc_inode(PT_ROOT, 0, FT_DIR);
    if (!root) return -ENOMEM;
    vfs_mount_root(path, &procfs_ops, root);
    printk(KERN_INFO "procfs: mounted at %s\n", path);
    return 0;
}
