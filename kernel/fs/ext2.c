#include "ext2.h"
#include "vfs.h"
#include "../../mm/vmm.h"
#include "../../mm/pmm.h"
#include "../../lib/string.h"
#include "../../lib/printk.h"
#include "../../drivers/ata.h"

#define EXT2_MAGIC      0xEF53
#define EXT2_ROOT_INO   2

#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG      1
#define EXT2_FT_DIR      2
#define EXT2_FT_SYMLINK  7

typedef struct __attribute__((packed)) {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks;
    uint16_t bg_free_inodes;
    uint16_t bg_used_dirs;
    uint16_t _pad[7];
} ext2_bgd_t;

typedef struct __attribute__((packed)) {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t _res1;
    uint32_t i_block[15];
    uint8_t  _pad[28];
} ext2_inode_t;

typedef struct __attribute__((packed)) {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[];
} ext2_dirent_t;

typedef struct {
    ata_drive_t *drv;
    uint32_t     lba_start;
    uint32_t     block_size;
    uint32_t     inodes_per_group;
    uint32_t     inode_size;
    uint32_t     first_data_block;
} ext2_fs_t;

static uint8_t block_buf[4096];

static bool
ext2_read_block(ext2_fs_t *fs, uint32_t block, void *buf)
{
    uint32_t sectors_per_block = fs->block_size / ATA_SECTOR_SIZE;
    uint32_t lba = fs->lba_start + block * sectors_per_block;
    return ata_read(fs->drv, lba,
                    (uint8_t)sectors_per_block, buf);
}

static bool
ext2_read_inode(ext2_fs_t *fs, uint32_t ino, ext2_inode_t *out)
{
    uint32_t group = (ino - 1) / fs->inodes_per_group;
    uint32_t index = (ino - 1) % fs->inodes_per_group;

    uint32_t bgd_block = fs->first_data_block + 1;
    uint32_t bgd_lba   = fs->lba_start +
                         bgd_block * (fs->block_size / ATA_SECTOR_SIZE) +
                         group * sizeof(ext2_bgd_t) / ATA_SECTOR_SIZE;

    uint8_t bgd_buf[ATA_SECTOR_SIZE];
    ata_read(fs->drv, bgd_lba, 1, bgd_buf);

    ext2_bgd_t bgd;
    kmemcpy(&bgd,
            bgd_buf + (group * sizeof(ext2_bgd_t)) % ATA_SECTOR_SIZE,
            sizeof(bgd));

    uint32_t inode_lba = fs->lba_start +
                         bgd.bg_inode_table * (fs->block_size / ATA_SECTOR_SIZE) +
                         index * fs->inode_size / ATA_SECTOR_SIZE;

    uint8_t ibuf[ATA_SECTOR_SIZE];
    ata_read(fs->drv, inode_lba, 1, ibuf);
    kmemcpy(out,
            ibuf + (index * fs->inode_size) % ATA_SECTOR_SIZE,
            sizeof(ext2_inode_t));
    return true;
}

typedef struct {
    ext2_fs_t *fs;
    uint32_t   ino_num;
} ext2_private_t;

static inode_t *
ext2_lookup(inode_t *dir, const char *name)
{
    ext2_private_t *priv = (ext2_private_t *)dir->private;
    ext2_fs_t      *fs   = priv->fs;
    ext2_inode_t    ino;

    if (!ext2_read_inode(fs, priv->ino_num, &ino)) return NULL;

    for (int b = 0; b < 12; b++) {
        if (!ino.i_block[b]) break;
        ext2_read_block(fs, ino.i_block[b], block_buf);

        uint32_t off = 0;
        while (off < fs->block_size) {
            ext2_dirent_t *de = (ext2_dirent_t *)(block_buf + off);
            if (!de->inode) { off += de->rec_len; continue; }

            if (de->name_len == kstrlen(name) &&
                kstrncmp(de->name, name, de->name_len) == 0) {

                ext2_inode_t child_ino;
                ext2_read_inode(fs, de->inode, &child_ino);

                inode_t *result = kmalloc(sizeof(inode_t));
                ext2_private_t *cp = kmalloc(sizeof(ext2_private_t));
                if (!result || !cp) return NULL;
                cp->fs = fs; cp->ino_num = de->inode;
                result->ino  = de->inode;
                result->size = child_ino.i_size;
                result->private = cp;

                uint16_t mode = child_ino.i_mode;
                if      ((mode & 0xF000) == 0x4000) result->type = FT_DIR;
                else if ((mode & 0xF000) == 0x8000) result->type = FT_REG;
                else                                result->type = FT_UNKNOWN;

                extern fs_ops_t ext2_ops;
                result->ops = &ext2_ops;
                return result;
            }
            off += de->rec_len;
        }
    }
    return NULL;
}

static int64_t
ext2_read(file_t *f, void *buf, size_t n)
{
    ext2_private_t *priv = (ext2_private_t *)f->inode->private;
    ext2_inode_t    ino;
    ext2_read_inode(priv->fs, priv->ino_num, &ino);

    if (f->pos >= ino.i_size) return 0;
    if (f->pos + n > ino.i_size) n = (size_t)(ino.i_size - f->pos);

    size_t   read = 0;
    uint8_t *dst  = (uint8_t *)buf;

    while (read < n) {
        uint32_t block_idx = (uint32_t)((f->pos + read) / priv->fs->block_size);
        uint32_t block_off = (uint32_t)((f->pos + read) % priv->fs->block_size);

        if (block_idx >= 12) break;
        if (!ino.i_block[block_idx]) break;

        ext2_read_block(priv->fs, ino.i_block[block_idx], block_buf);
        size_t chunk = priv->fs->block_size - block_off;
        if (chunk > n - read) chunk = n - read;
        kmemcpy(dst + read, block_buf + block_off, chunk);
        read += chunk;
    }
    f->pos += read;
    return (int64_t)read;
}

static int
ext2_readdir(file_t *f, dentry_t *out)
{
    ext2_private_t *priv = (ext2_private_t *)f->inode->private;
    ext2_inode_t    ino;
    ext2_read_inode(priv->fs, priv->ino_num, &ino);

    uint32_t abs = (uint32_t)f->pos;
    uint32_t block_idx = abs / priv->fs->block_size;
    uint32_t block_off = abs % priv->fs->block_size;

    if (block_idx >= 12 || !ino.i_block[block_idx]) return 0;

    ext2_read_block(priv->fs, ino.i_block[block_idx], block_buf);

    while (block_off < priv->fs->block_size) {
        ext2_dirent_t *de = (ext2_dirent_t *)(block_buf + block_off);
        if (!de->rec_len) return 0;
        if (de->inode) {
            kstrncpy(out->name, de->name, de->name_len);
            out->name[de->name_len] = '\0';
            out->inode = NULL;
            f->pos += de->rec_len;
            return 1;
        }
        block_off += de->rec_len;
        f->pos    += de->rec_len;
    }
    return 0;
}

static int64_t
ext2_write(file_t *f, const void *buf, size_t n)
{
    ext2_private_t *priv = (ext2_private_t *)f->inode->private;
    ext2_inode_t    ino;
    ext2_read_inode(priv->fs, priv->ino_num, &ino);

    size_t   written = 0;
    const uint8_t *src = (const uint8_t *)buf;

    while (written < n) {
        uint32_t block_idx = (uint32_t)((f->pos + written) / priv->fs->block_size);
        uint32_t block_off = (uint32_t)((f->pos + written) % priv->fs->block_size);

        if (block_idx >= 12) break;

        if (!ino.i_block[block_idx]) {
            void *page = pmm_alloc();
            if (!page) break;
            kmemset(page, 0, PAGE_SIZE);
            ino.i_block[block_idx] = (uint32_t)((uint64_t)page / priv->fs->block_size);
        }

        ext2_read_block(priv->fs, ino.i_block[block_idx], block_buf);
        size_t chunk = priv->fs->block_size - block_off;
        if (chunk > n - written) chunk = n - written;
        kmemcpy(block_buf + block_off, src + written, chunk);

        uint32_t sectors_per_block = priv->fs->block_size / ATA_SECTOR_SIZE;
        uint32_t lba = priv->fs->lba_start +
                       ino.i_block[block_idx] * sectors_per_block;
        ata_write(priv->fs->drv, lba, (uint8_t)sectors_per_block, block_buf);

        written += chunk;
    }

    f->pos += written;
    if (f->pos > f->inode->size) f->inode->size = f->pos;
    return (int64_t)written;
}

fs_ops_t ext2_ops = {
    .lookup  = ext2_lookup,
    .read    = ext2_read,
    .write   = ext2_write,
    .readdir = ext2_readdir,
};

int
ext2_mount(const char *path, ata_drive_t *drv, uint32_t lba_start)
{
    static uint8_t sb_buf[1024] __attribute__((aligned(512)));

    if (!ata_read(drv, lba_start + 2, 2, sb_buf)) {
        printk(KERN_ERR "ext2: failed to read superblock\n");
        return -1;
    }

    uint16_t magic;
    kmemcpy(&magic, sb_buf + 56, 2);
    if (magic != EXT2_MAGIC) {
        printk(KERN_ERR "ext2: bad magic 0x%x\n", (uint64_t)magic);
        return -1;
    }

    uint32_t log_block_size, inodes_per_group, first_data_block;
    kmemcpy(&log_block_size,    sb_buf + 24, 4);
    kmemcpy(&inodes_per_group,  sb_buf + 40, 4);
    kmemcpy(&first_data_block,  sb_buf + 20, 4);

    ext2_fs_t *fs = kmalloc(sizeof(ext2_fs_t));
    if (!fs) return -ENOMEM;
    fs->drv              = drv;
    fs->lba_start        = lba_start;
    fs->block_size       = 1024U << log_block_size;
    fs->inodes_per_group = inodes_per_group;
    fs->inode_size       = 128;
    fs->first_data_block = first_data_block;

    ext2_inode_t root_ino;
    ext2_read_inode(fs, EXT2_ROOT_INO, &root_ino);

    inode_t        *root = kmalloc(sizeof(inode_t));
    ext2_private_t *priv = kmalloc(sizeof(ext2_private_t));
    if (!root || !priv) return -ENOMEM;
    kmemset(root, 0, sizeof(inode_t));
    kmemset(priv, 0, sizeof(ext2_private_t));
    priv->fs = fs; priv->ino_num = EXT2_ROOT_INO;
    root->ino = EXT2_ROOT_INO; root->type = FT_DIR;
    root->size = root_ino.i_size;
    root->ops = &ext2_ops; root->private = priv;

    vfs_mount_root(path, &ext2_ops, root);
    printk(KERN_INFO "ext2: mounted %s (block_size=%u)\n",
           path, (uint64_t)fs->block_size);
    return 0;
}
