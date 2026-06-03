#include "persist.h"
#include "../fs/vfs.h"
#include "../../drivers/ata.h"
#include "../../lib/string.h"
#include "../../lib/printk.h"

#define SINUXFS_MAGIC   0x534E5846u
#define SINUXFS_VERSION 1
#define MAX_NODES       256
#define MAX_DATA        (64 * 1024)
#define NODE_NAME_LEN   256

#define SINUXFS_REG  1
#define SINUXFS_DIR  2

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t version;
    uint32_t node_count;
    uint32_t _pad;
} sinuxfs_header_t;

typedef struct __attribute__((packed)) {
    char     path[NODE_NAME_LEN];
    uint32_t type;
    uint32_t size;
    uint32_t data_offset;
    uint32_t _pad;
} sinuxfs_node_t;

static ata_drive_t *disk = NULL;
static uint32_t     disk_lba = 64;

#define DISK_SECTORS 256
static uint8_t  disk_buf[DISK_SECTORS * ATA_SECTOR_SIZE];

static bool
disk_read(void)
{
    if (!disk) return false;
    return ata_read(disk, disk_lba, DISK_SECTORS, disk_buf);
}

static bool
disk_write(void)
{
    if (!disk) return false;
    return ata_write(disk, disk_lba, DISK_SECTORS, disk_buf);
}

static void
walk_dir(const char *path, int depth)
{
    if (depth > 8) return;
    file_t *f = vfs_open(path, O_RDONLY);
    if (!f) return;

    dentry_t d;
    while (vfs_readdir(f, &d)) {
        if (!kstrcmp(d.name, ".") || !kstrcmp(d.name, "..")) continue;
        if (!d.name[0]) continue;

        char full[256];
        if (kstrlen(path) == 1) {
            full[0] = '/';
            kstrcpy(full + 1, d.name);
        } else {
            kstrncpy(full, path, 200);
            kstrncat(full, "/", 255);
            kstrncat(full, d.name, 255);
        }

        inode_t *ino = vfs_lookup(full);
        if (!ino) continue;

        if (ino->type == FT_DIR) {
            walk_dir(full, depth + 1);
        }
    }
    vfs_close(f);
}

void
persist_sync(void)
{
    if (!disk) return;

    kmemset(disk_buf, 0, sizeof(disk_buf));

    sinuxfs_header_t *hdr = (sinuxfs_header_t *)disk_buf;
    hdr->magic   = SINUXFS_MAGIC;
    hdr->version = SINUXFS_VERSION;

    sinuxfs_node_t *nodes = (sinuxfs_node_t *)(disk_buf + sizeof(sinuxfs_header_t));
    uint8_t *data_area    = disk_buf + sizeof(sinuxfs_header_t) +
                            MAX_NODES * sizeof(sinuxfs_node_t);
    uint32_t data_used    = 0;
    uint32_t node_count   = 0;

    char paths[MAX_NODES][256];
    int  path_count = 0;

    char stack[32][256];
    int  sp = 0;
    kstrcpy(stack[sp++], "/");

    while (sp > 0 && path_count < MAX_NODES) {
        char cur[256];
        kstrcpy(cur, stack[--sp]);

        file_t *f = vfs_open(cur, O_RDONLY);
        if (!f) continue;

        dentry_t d;
        while (vfs_readdir(f, &d) && path_count < MAX_NODES) {
            if (!d.name[0]) continue;

            char full[256];
            if (kstrlen(cur) == 1) {
                full[0] = '/';
                kstrcpy(full+1, d.name);
            } else {
                kstrncpy(full, cur, 200);
                kstrncat(full, "/", 255);
                kstrncat(full, d.name, 255);
            }

            kstrcpy(paths[path_count++], full);

            inode_t *ino = vfs_lookup(full);
            if (ino && ino->type == FT_DIR && sp < 32)
                kstrcpy(stack[sp++], full);
        }
        vfs_close(f);
    }

    for (int i = 0; i < path_count && node_count < MAX_NODES; i++) {
        inode_t *ino = vfs_lookup(paths[i]);
        if (!ino) continue;

        sinuxfs_node_t *n = &nodes[node_count];
        kstrncpy(n->path, paths[i], NODE_NAME_LEN);
        n->type        = (ino->type == FT_DIR) ? SINUXFS_DIR : SINUXFS_REG;
        n->size        = (uint32_t)ino->size;
        n->data_offset = data_used;

        if (ino->type == FT_REG && ino->size > 0) {
            file_t *rf = vfs_open(paths[i], O_RDONLY);
            if (rf) {
                size_t avail = sizeof(disk_buf) -
                               (size_t)(data_area - disk_buf) - data_used;
                size_t to_read = ino->size < avail ? ino->size : avail;
                vfs_read(rf, data_area + data_used, to_read);
                data_used += (uint32_t)to_read;
                vfs_close(rf);
            }
        }
        node_count++;
    }

    hdr->node_count = node_count;

    if (disk_write())
        printk(KERN_INFO "persist: synced %u nodes to disk\n",
               (uint64_t)node_count);
    else
        printk(KERN_ERR "persist: disk write failed\n");
}

static void
persist_load(void)
{
    if (!disk_read()) return;

    sinuxfs_header_t *hdr = (sinuxfs_header_t *)disk_buf;
    if (hdr->magic != SINUXFS_MAGIC) {
        printk(KERN_INFO "persist: no saved state found\n");
        return;
    }

    sinuxfs_node_t *nodes = (sinuxfs_node_t *)(disk_buf + sizeof(sinuxfs_header_t));
    uint8_t *data_area    = disk_buf + sizeof(sinuxfs_header_t) +
                            MAX_NODES * sizeof(sinuxfs_node_t);

    printk(KERN_INFO "persist: loading %u nodes\n",
           (uint64_t)hdr->node_count);

    for (uint32_t i = 0; i < hdr->node_count && i < MAX_NODES; i++) {
        sinuxfs_node_t *n = &nodes[i];
        if (!n->path[0]) continue;

        if (n->type == SINUXFS_DIR) {
            vfs_create(n->path, FT_DIR);
        } else {
            file_t *f = vfs_open(n->path, O_WRONLY | O_CREAT | O_TRUNC);
            if (f && n->size > 0) {
                vfs_write(f, data_area + n->data_offset, n->size);
                vfs_close(f);
            }
        }
    }
    printk(KERN_INFO "persist: restored filesystem\n");
}

void
persist_init(void)
{
    disk = ata_get_drive(0);
    if (!disk) {
        printk(KERN_WARNING "persist: no ATA drive found, data won't persist\n");
        return;
    }
    printk(KERN_INFO "persist: using drive 0 (lba=%u)\n", (uint64_t)disk_lba);
    persist_load();
}
