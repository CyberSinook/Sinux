#include "pipe.h"
#include "signal.h"
#include "../proc/process.h"
#include "../../mm/vmm.h"
#include "../../lib/string.h"

static int64_t
pipe_read(file_t *f, void *buf, size_t n)
{
    pipe_t *p = (pipe_t *)f->inode->private;
    if (!p) return -EBADF;

    while (p->count == 0) {
        if (p->write_closed) return 0;
        __asm__ volatile ("pause");
    }

    size_t read = 0;
    uint8_t *dst = (uint8_t *)buf;
    while (read < n && p->count > 0) {
        dst[read++] = p->buf[p->tail];
        p->tail = (p->tail + 1) % PIPE_BUF_SIZE;
        p->count--;
    }
    return (int64_t)read;
}

static int64_t
pipe_write(file_t *f, const void *buf, size_t n)
{
    pipe_t *p = (pipe_t *)f->inode->private;
    if (!p) return -EBADF;
    if (p->read_closed) {
        signal_send(proc_current()->pid, 13);
        return -EINVAL;
    }

    size_t written = 0;
    const uint8_t *src = (const uint8_t *)buf;
    while (written < n) {
        while (p->count >= PIPE_BUF_SIZE)
            __asm__ volatile ("pause");
        p->buf[p->head] = src[written++];
        p->head = (p->head + 1) % PIPE_BUF_SIZE;
        p->count++;
    }
    return (int64_t)written;
}

static int
pipe_close(file_t *f)
{
    pipe_t *p = (pipe_t *)f->inode->private;
    if (!p) return 0;
    if (f->flags & O_WRONLY) p->write_closed = true;
    else                     p->read_closed  = true;
    p->refcount--;
    if (p->refcount <= 0) kfree(p);
    return 0;
}

static fs_ops_t pipe_ops = {
    .read  = pipe_read,
    .write = pipe_write,
    .close = pipe_close,
};

pipe_t *
pipe_alloc(void)
{
    pipe_t *p = kmalloc(sizeof(pipe_t));
    if (!p) return NULL;
    kmemset(p, 0, sizeof(pipe_t));
    p->refcount = 2;
    return p;
}

int
pipe_create(int fds[2])
{
    pipe_t *p = pipe_alloc();
    if (!p) return -ENOMEM;

    inode_t *ri = kmalloc(sizeof(inode_t));
    inode_t *wi = kmalloc(sizeof(inode_t));
    file_t  *rf = kmalloc(sizeof(file_t));
    file_t  *wf = kmalloc(sizeof(file_t));
    if (!ri || !wi || !rf || !wf) return -ENOMEM;

    kmemset(ri, 0, sizeof(inode_t));
    kmemset(wi, 0, sizeof(inode_t));
    ri->type = wi->type = FT_FIFO;
    ri->private = wi->private = p;
    ri->ops = wi->ops = &pipe_ops;

    kmemset(rf, 0, sizeof(file_t));
    kmemset(wf, 0, sizeof(file_t));
    rf->inode = ri; rf->ops = &pipe_ops; rf->flags = O_RDONLY; rf->refcount = 1;
    wf->inode = wi; wf->ops = &pipe_ops; wf->flags = O_WRONLY; wf->refcount = 1;

    process_t *proc = proc_current();
    fds[0] = proc_alloc_fd(proc, rf);
    fds[1] = proc_alloc_fd(proc, wf);
    if (fds[0] < 0 || fds[1] < 0) return -EMFILE;
    return 0;
}
