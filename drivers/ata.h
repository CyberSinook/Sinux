#pragma once
#include <stdint.h>
#include <stdbool.h>

#define ATA_SECTOR_SIZE 512
#define ATA_PRIMARY_BASE 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

typedef struct {
    uint16_t base;
    uint16_t ctrl;
    int      slave;
    bool     present;
    uint32_t sectors;
    char     model[41];
} ata_drive_t;

void     ata_init(void);
bool     ata_read(ata_drive_t *drv, uint32_t lba, uint8_t count, void *buf);
bool     ata_write(ata_drive_t *drv, uint32_t lba, uint8_t count, const void *buf);
ata_drive_t *ata_get_drive(int idx);
