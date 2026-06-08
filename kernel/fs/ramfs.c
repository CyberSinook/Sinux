#include "ramfs.h"
#include "vfs.h"
#include "../../mm/vmm.h"
#include "../../lib/string.h"

#define RAMFS_MAX_NODES  2048
#define RAMFS_MAX_DATA   (4096 * 16)   
#define RAMFS_MAX_CHILDREN 128

typedef struct ramfs_node {
    char       name[256];
    file_type_t type;
    uint8_t    *data;
    uint64_t   size;
    uint64_t   capacity;
    inode_t    inode;

    struct ramfs_node *children[RAMFS_MAX_CHILDREN];
    int                child_count;
} ramfs_node_t;

static ramfs_node_t nodes[RAMFS_MAX_NODES];
static int          node_count = 0;

static ramfs_node_t *
alloc_node(const char *name, file_type_t type)
{
    if (node_count >= RAMFS_MAX_NODES) return NULL;
    ramfs_node_t *n = &nodes[node_count++];
    kmemset(n, 0, sizeof(ramfs_node_t));
    kstrncpy(n->name, name, 255);
    n->type = type;
    n->inode.ino     = (uint64_t)(uintptr_t)n;
    n->inode.type    = type;
    n->inode.private = n;
    return n;
}

static int64_t
ramfs_read(file_t *f, void *buf, size_t n)
{
    ramfs_node_t *node = (ramfs_node_t *)f->inode->private;
    if (!node->data || f->pos >= node->size) return 0;
    size_t avail = (size_t)(node->size - f->pos);
    if (n > avail) n = avail;
    kmemcpy(buf, node->data + f->pos, n);
    f->pos += n;
    return (int64_t)n;
}

static int64_t
ramfs_write(file_t *f, const void *buf, size_t n)
{
    ramfs_node_t *node = (ramfs_node_t *)f->inode->private;

    if (f->pos + n > node->capacity) {
        size_t new_cap = f->pos + n + 4096;
        uint8_t *new_data = kmalloc(new_cap);
        if (!new_data) return -ENOSPC;
        if (node->data) {
            kmemcpy(new_data, node->data, (size_t)node->size);
            kfree(node->data);
        }
        node->data     = new_data;
        node->capacity = new_cap;
    }

    kmemcpy(node->data + f->pos, buf, n);
    f->pos += n;
    if (f->pos > node->size) {
        node->size = f->pos;
        f->inode->size = node->size;
    }
    return (int64_t)n;
}

static inode_t *
ramfs_lookup(inode_t *dir, const char *name)
{
    ramfs_node_t *d = (ramfs_node_t *)dir->private;
    for (int i = 0; i < d->child_count; i++) {
        if (kstrcmp(d->children[i]->name, name) == 0)
            return &d->children[i]->inode;
    }
    return NULL;
}

static int
ramfs_create(inode_t *dir, const char *name, file_type_t type)
{
    ramfs_node_t *d = (ramfs_node_t *)dir->private;
    if (d->child_count >= RAMFS_MAX_CHILDREN) return -ENOSPC;

    ramfs_node_t *n = alloc_node(name, type);
    if (!n) return -ENOSPC;

    extern fs_ops_t ramfs_ops;
    n->inode.ops = &ramfs_ops;
    d->children[d->child_count++] = n;
    return 0;
}

static int
ramfs_unlink(inode_t *dir, const char *name)
{
    ramfs_node_t *d = (ramfs_node_t *)dir->private;
    for (int i = 0; i < d->child_count; i++) {
        if (kstrcmp(d->children[i]->name, name) == 0) {
            d->children[i] = d->children[--d->child_count];
            return 0;
        }
    }
    return -ENOENT;
}

static int
ramfs_readdir(file_t *f, dentry_t *out)
{
    ramfs_node_t *d = (ramfs_node_t *)f->inode->private;
    int idx = (int)f->pos;
    if (idx >= d->child_count) return 0;
    kstrncpy(out->name, d->children[idx]->name, 255);
    out->inode = &d->children[idx]->inode;
    f->pos++;
    return 1;
}

static int64_t
ramfs_seek(file_t *f, int64_t off, int whence)
{
    uint64_t size = f->inode->size;
    uint64_t new_pos;
    if      (whence == 0) new_pos = (uint64_t)off;
    else if (whence == 1) new_pos = f->pos + (uint64_t)off;
    else if (whence == 2) new_pos = size + (uint64_t)off;
    else return -EINVAL;
    f->pos = new_pos;
    return (int64_t)new_pos;
}

fs_ops_t ramfs_ops = {
    .open    = NULL,
    .close   = NULL,
    .read    = ramfs_read,
    .write   = ramfs_write,
    .seek    = ramfs_seek,
    .readdir = ramfs_readdir,
    .lookup  = ramfs_lookup,
    .create  = ramfs_create,
    .unlink  = ramfs_unlink,
};

int
ramfs_mount(const char *path)
{
    ramfs_node_t *root = alloc_node("/", FT_DIR);
    if (!root) return -ENOMEM;
    root->inode.ops = &ramfs_ops;

    const char *dirs[] = { "bin","dev","etc","tmp","home","proc","var", NULL };
    for (int i = 0; dirs[i]; i++) {
        ramfs_node_t *d = alloc_node(dirs[i], FT_DIR);
        if (d) {
            d->inode.ops = &ramfs_ops;
            root->children[root->child_count++] = d;
        }
    }

    vfs_mount_root(path, &ramfs_ops, &root->inode);

    extern int vfs_set_root(const char *path, inode_t *root);

    (void)path;

    ramfs_node_t *etc = NULL;
    for (int i = 0; i < root->child_count; i++)
        if (kstrcmp(root->children[i]->name, "etc") == 0)
            { etc = root->children[i]; break; }
    if (etc) {
        ramfs_create(&etc->inode, "hostname", FT_REG);
        inode_t *hn = ramfs_lookup(&etc->inode, "hostname");
        if (hn) {
            file_t f = { .inode=hn, .pos=0, .ops=&ramfs_ops };
            const char *hostname = "sinux\n";
            ramfs_write(&f, hostname, kstrlen(hostname));
        }
    }

    return 0;
}
