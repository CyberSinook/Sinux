#ifndef __XFS_H__
#define __XFS_H__

#include <stdint.h>
#include <stddef.h>

/* XFS Superblock Magic */
#define XFS_SB_MAGIC 0x58465342  /* "XFSB" */

/* XFS Block Sizes */
#define XFS_BLOCKSIZE_MIN 512
#define XFS_BLOCKSIZE_MAX 65536

/* XFS Inode Sizes */
#define XFS_DINODE_MIN_SIZE 256
#define XFS_DINODE_MAX_SIZE 2048

/* XFS Directory Entry Flags */
#define XFS_DIR2_DATA_FREE_TAG 0xFFFF
#define XFS_DIR2_NULL_DATAPTR 0xFFFF

typedef struct {
    uint32_t magic;           /* Superblock magic number */
    uint32_t blocksize;       /* Filesystem blocksize in bytes */
    uint64_t dblocks;         /* Number of data blocks */
    uint64_t rblocks;         /* Number of realtime blocks */
    uint64_t rextents;        /* Number of realtime extents */
    uint8_t  uuid[16];        /* Unique ID of the filesystem */
    uint64_t logstart;        /* Block address of log if internal */
    uint64_t rootino;         /* Root inode number */
    uint64_t rbmino;          /* Realtime bitmap inode no. */
    uint64_t rsumino;         /* Realtime summary inode no. */
    uint32_t rextsize;        /* Realtime extent size in blocks */
    uint32_t agblocks;        /* Size of an allocation group */
    uint32_t agcount;         /* Number of allocation groups */
    uint32_t rbmblocks;       /* Number of realtime bitmap blocks */
    uint32_t logblocks;       /* Number of log blocks */
    uint16_t versionnum;      /* Version number of the filesystem */
    uint16_t sectsize;        /* Sector size in bytes */
    uint16_t inodesize;       /* Inode size in bytes */
    uint16_t inopblock;       /* Inodes per block */
    char     fname[12];       /* Filesystem name */
    uint8_t  blocklog;        /* Log base 2 of blocksize */
    uint8_t  sectlog;         /* Log base 2 of sectsize */
    uint8_t  inodelog;        /* Log base 2 of inodesize */
    uint8_t  inopblog;        /* Log base 2 of inopblock */
    uint8_t  agblklog;        /* Log base 2 of agblocks */
    uint8_t  rextslog;        /* Log base 2 of rtextents */
    uint8_t  inprogress;      /* Flag: 1 if filesystem being upgraded */
    uint8_t  imax_pct;        /* Max % of filesystem used by inode space */
    uint64_t icount;          /* Number of allocated inodes */
    uint64_t ifree;           /* Number of free inodes */
    uint64_t fdblocks;        /* Number of free data blocks */
    uint64_t frextents;       /* Number of free realtime extents */
} xfs_superblock_t;

typedef struct {
    uint64_t di_magic;        /* Inode magic number */
    uint16_t di_mode;         /* Inode mode/type */
    int8_t   di_version;      /* Inode version */
    int8_t   di_format;       /* Inode data format */
    uint16_t di_onlink;       /* Old next unlinked inode */
    uint32_t di_uid;          /* Owner's user id */
    uint32_t di_gid;          /* Owner's group id */
    uint32_t di_nlink;        /* Number of links to inode */
    uint16_t di_projid;       /* Project id */
    uint16_t di_pad;          /* Unused, formerly pad */
    uint32_t di_aformat;      /* Attribute format */
    uint64_t di_size;         /* Number of bytes used by file */
    uint64_t di_nblocks;      /* Number of data blocks used */
    uint32_t di_extsize;      /* Basic/minimum extent size for file */
    uint32_t di_nextents;     /* Number of extents in data fork */
    uint16_t di_anextents;    /* Number of extents in attribute fork */
    uint8_t  di_dmevmask;     /* DMIG event mask */
    uint8_t  di_dmstate;      /* DMIG state info */
    uint16_t di_flags;        /* Random flags */
    uint32_t di_gen;          /* Generation count */
    uint32_t di_nextlink;     /* Next unlinked inode */
    uint32_t di_crtime;       /* Creation time */
} xfs_dinode_t;

typedef struct {
    uint64_t offset;          /* Offset of extent */
    uint64_t length;          /* Length of extent */
} xfs_extent_t;

/* XFS Inode Mode Bits */
#define XFS_IFMT    0170000
#define XFS_IFREG   0100000
#define XFS_IFDIR   0040000
#define XFS_IFLNK   0120000
#define XFS_IFBLK   0060000
#define XFS_IFCHR   0020000
#define XFS_IFIFO   0010000
#define XFS_IFSOCK  0140000

/* XFS Functions */
int32_t xfs_init();
int32_t xfs_mount(void *device);
int32_t xfs_unmount();
int32_t xfs_read_superblock();
int32_t xfs_read_inode(uint64_t ino, xfs_dinode_t *inode);
int32_t xfs_read_block(uint64_t block, void *buf);
int32_t xfs_write_block(uint64_t block, const void *buf);
int32_t xfs_read_file(uint64_t ino, uint64_t offset, uint32_t size, void *buf);
int32_t xfs_write_file(uint64_t ino, uint64_t offset, uint32_t size, const void *buf);
int32_t xfs_list_directory(uint64_t dir_ino);
int32_t xfs_create_file(uint64_t parent_ino, const char *name, uint16_t mode);
int32_t xfs_delete_file(uint64_t parent_ino, const char *name);

#endif /* __XFS_H__ */
