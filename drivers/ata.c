#include "ata.h"
#include "../lib/io.h"
#include "../lib/string.h"
#include "../lib/printk.h"
#include "../lib/errno.h"

#define ATA_DATA      0
#define ATA_FEATURES  1
#define ATA_SECCOUNT  2
#define ATA_LBA0      3
#define ATA_LBA1      4
#define ATA_LBA2      5
#define ATA_DRIVE     6
#define ATA_CMD       7
#define ATA_STATUS    7

#define ATA_CMD_READ    0x20
#define ATA_CMD_WRITE   0x30
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

#define ATA_TIMEOUT_POLL   500000
#define ATA_TIMEOUT_BUSY   1000000

static ata_drive_t drives[4];
static int         drive_count = 0;

static void
ata_wait(uint16_t base)
{
    for (int i = 0; i < 4; i++) inb(base + ATA_STATUS);
}

static bool
ata_poll(uint16_t base)
{
    for (int i = 0; i < ATA_TIMEOUT_POLL; i++) {
        uint8_t s = inb(base + ATA_STATUS);
        if (s & ATA_SR_ERR) {
            printk(KERN_ERR "ata: error detected (status=0x%x)\n", (uint64_t)s);
            errno = EIO;
            return false;
        }
        if (!(s & ATA_SR_BSY) && (s & ATA_SR_DRQ)) return true;
    }
    printk(KERN_ERR "ata: timeout waiting for DRQ\n");
    errno = ETIMEDOUT;
    return false;
}

static bool
ata_identify(uint16_t base, int slave, ata_drive_t *drv)
{
    outb(base + ATA_DRIVE, slave ? 0xB0 : 0xA0);
    outb(base + ATA_SECCOUNT, 0);
    outb(base + ATA_LBA0, 0);
    outb(base + ATA_LBA1, 0);
    outb(base + ATA_LBA2, 0);
    outb(base + ATA_CMD, ATA_CMD_IDENTIFY);

    uint8_t status = inb(base + ATA_STATUS);
    if (status == 0) {
        errno = ENODEV;
        return false;
    }

    for (int i = 0; i < ATA_TIMEOUT_BUSY; i++) {
        status = inb(base + ATA_STATUS);
        if (!(status & ATA_SR_BSY)) break;
    }
    
    if (status & ATA_SR_BSY) {
        printk(KERN_ERR "ata: timeout waiting for drive\n");
        errno = ETIMEDOUT;
        return false;
    }
    
    if (inb(base + ATA_LBA1) || inb(base + ATA_LBA2)) {
        errno = ENODEV;
        return false;
    }
    
    if (!ata_poll(base)) return false;

    uint16_t buf[256];
    for (int i = 0; i < 256; i++) buf[i] = inw(base + ATA_DATA);

    drv->base    = base;
    drv->slave   = slave;
    drv->present = true;
    drv->sectors = ((uint32_t)buf[61] << 16) | buf[60];

    for (int i = 0; i < 20; i++) {
        drv->model[i*2]   = (char)(buf[27+i] >> 8);
        drv->model[i*2+1] = (char)(buf[27+i] & 0xFF);
    }
    drv->model[40] = '\0';
    return true;
}

void
ata_init(void)
{
    uint16_t bases[] = { ATA_PRIMARY_BASE };
    for (int b = 0; b < 1; b++) {
        for (int s = 0; s < 2; s++) {
            ata_drive_t tmp;
            kmemset(&tmp, 0, sizeof(tmp));
            if (ata_identify(bases[b], s, &tmp)) {
                drives[drive_count++] = tmp;
                printk(KERN_INFO "ata: drive %d: %s (%u sectors)\n",
                    (uint64_t)(drive_count-1),
                    tmp.model,
                    (uint64_t)tmp.sectors);
            }
        }
    }
}

bool
ata_read(ata_drive_t *drv, uint32_t lba, uint8_t count, void *buf)
{
    if (!drv || !drv->present) {
        errno = ENODEV;
        return false;
    }
    
    if (lba + count > drv->sectors) {
        errno = EINVAL;
        printk(KERN_ERR "ata: read beyond disk (lba=%u, sectors=%u)\n",
               (uint64_t)lba, (uint64_t)drv->sectors);
        return false;
    }
    
    outb(drv->base + ATA_DRIVE,
         (uint8_t)(0xE0 | (drv->slave << 4) | ((lba >> 24) & 0xF)));
    outb(drv->base + ATA_FEATURES, 0);
    outb(drv->base + ATA_SECCOUNT, count);
    outb(drv->base + ATA_LBA0, (uint8_t)(lba & 0xFF));
    outb(drv->base + ATA_LBA1, (uint8_t)((lba >> 8) & 0xFF));
    outb(drv->base + ATA_LBA2, (uint8_t)((lba >> 16) & 0xFF));
    outb(drv->base + ATA_CMD, ATA_CMD_READ);

    uint16_t *dst = (uint16_t *)buf;
    for (int s = 0; s < count; s++) {
        ata_wait(drv->base);
        if (!ata_poll(drv->base)) {
            printk(KERN_ERR "ata: read failed at sector %u\n", (uint64_t)(lba + s));
            return false;
        }
        for (int i = 0; i < 256; i++)
            dst[s * 256 + i] = inw(drv->base + ATA_DATA);
    }
    return true;
}

bool
ata_write(ata_drive_t *drv, uint32_t lba, uint8_t count, const void *buf)
{
    if (!drv || !drv->present) {
        errno = ENODEV;
        return false;
    }
    
    if (lba + count > drv->sectors) {
        errno = EINVAL;
        printk(KERN_ERR "ata: write beyond disk (lba=%u, sectors=%u)\n",
               (uint64_t)lba, (uint64_t)drv->sectors);
        return false;
    }
    
    outb(drv->base + ATA_DRIVE,
         (uint8_t)(0xE0 | (drv->slave << 4) | ((lba >> 24) & 0xF)));
    outb(drv->base + ATA_FEATURES, 0);
    outb(drv->base + ATA_SECCOUNT, count);
    outb(drv->base + ATA_LBA0, (uint8_t)(lba & 0xFF));
    outb(drv->base + ATA_LBA1, (uint8_t)((lba >> 8) & 0xFF));
    outb(drv->base + ATA_LBA2, (uint8_t)((lba >> 16) & 0xFF));
    outb(drv->base + ATA_CMD, ATA_CMD_WRITE);

    const uint16_t *src = (const uint16_t *)buf;
    for (int s = 0; s < count; s++) {
        ata_wait(drv->base);
        if (!ata_poll(drv->base)) {
            printk(KERN_ERR "ata: write failed at sector %u\n", (uint64_t)(lba + s));
            return false;
        }
        for (int i = 0; i < 256; i++)
            outw(drv->base + ATA_DATA, src[s * 256 + i]);
    }
    return true;
}

ata_drive_t *ata_get_drive(int idx) {
    if (idx < 0 || idx >= drive_count) return NULL;
    return &drives[idx];
}
