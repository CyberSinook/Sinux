#ifndef __XFS_MOUNT_H__
#define __XFS_MOUNT_H__

#include <stdint.h>
#include "xfs/xfs.h"

typedef struct xfs_mount {
    void *device;                    /* Block device */
    xfs_superblock_t *superblock;   /* Filesystem superblock */
    uint32_t blocksize;             /* Block size */
    uint32_t sectorsize;            /* Sector size */
    uint32_t state;                 /* Mount state */
} xfs_mount_t;

#define XFS_MOUNT_STATE_READY    1
#define XFS_MOUNT_STATE_UNMOUNT  2
#define XFS_MOUNT_STATE_SHUTDOWN 3

#endif /* __XFS_MOUNT_H__ */
