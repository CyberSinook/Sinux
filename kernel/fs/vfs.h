#pragma once
#include <stdint.h>
#include <stddef.h>

typedef enum {
    FT_UNKNOWN = 0,
    FT_REG,       
    FT_DIR,       
    FT_CHR,       
    FT_BLK,       
    FT_FIFO,      
    FT_LNK,       
} file_type_t;

#define O_RDONLY  0x0000
#define O_WRONLY  0x0001
#define O_RDWR    0x0002
#define O_CREAT   0x0040
#define O_TRUNC   0x0200
#define O_APPEND  0x0400

#define ENOENT    2
#define EBADF     9
#define ENOMEM   12
#define EACCES   13
#define EEXIST   17
#define ENOTDIR  20
#define EISDIR   21
#define EINVAL   22
#define EMFILE   24
#define ENOSPC   28

typedef struct inode inode_t;
typedef struct file  file_t;
typedef struct dentry dentry_t;
typedef struct fs_ops fs_ops_t;
typedef struct vfs_mount vfs_mount_t;

struct fs_ops {
    int    (*open)   (inode_t *ino, file_t *f);
    int    (*close)  (file_t *f);
    int64_t (*read)  (file_t *f, void *buf, size_t n);
    int64_t (*write) (file_t *f, const void *buf, size_t n);
    int64_t (*seek)  (file_t *f, int64_t off, int whence);
    int    (*readdir)(file_t *f, dentry_t *out);
    inode_t *(*lookup)(inode_t *dir, const char *name);
    int    (*create) (inode_t *dir, const char *name, file_type_t type);
    int    (*unlink) (inode_t *dir, const char *name);
};

struct inode {
    uint64_t    ino;
    file_type_t type;
    uint64_t    size;
    uint32_t    mode;
    fs_ops_t   *ops;
    void       *private;   
};

struct file {
    inode_t  *inode;
    uint64_t  pos;
    int       flags;
    int       refcount;
    fs_ops_t *ops;
};

struct dentry {
    char      name[256];
    inode_t  *inode;
};

void      vfs_init(void);
int       vfs_mount(const char *path, fs_ops_t *ops, void *data);
int       vfs_mount_root(const char *path, fs_ops_t *ops, inode_t *root);
int64_t   vfs_readdir(file_t *f, dentry_t *d);
file_t   *vfs_open(const char *path, int flags);
int       vfs_close(file_t *f);
int64_t   vfs_read(file_t *f, void *buf, size_t n);
int64_t   vfs_write(file_t *f, const void *buf, size_t n);
inode_t  *vfs_lookup(const char *path);
int       vfs_create(const char *path, file_type_t type);
int       vfs_unlink(const char *path);
