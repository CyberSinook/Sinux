#pragma once
#include <stdint.h>
#include "vfs.h"
#include "../../drivers/ata.h"

int ext2_mount(const char *path, ata_drive_t *drv, uint32_t lba_start);
