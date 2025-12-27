#include <stdint.h>
#include <kernel/drivers/ide.h>
#include <kernel/arch/x86/schedular/schedular.h>
#include <kernel/arch/x86/mm/heap.h>
#include <lib/inout.h>
#include <lib/string.h>

ide_command_t *ide_queue;

int ide_wait_ready() {
    uint8_t status;
    uint32_t timeout = 0;
    const uint32_t TIMEOUT_LIMIT = 10000000;
    while (1) {
        status = inb(IDE_STATUS);
        if (!(status & IDE_STATUS_BSY) && (status & IDE_STATUS_DRQ)) {
            return 0;
        }
        if (status == 0) return -1;
        if (++timeout > TIMEOUT_LIMIT) {
            return -1;
        }
    }
}

void ide_wait_busy() {
    while (inb(IDE_STATUS) & IDE_STATUS_BSY);
}

void ide_flush_cache() {
    outb(IDE_COMMAND, IDE_CMD_FLUSH);
    while (inb(IDE_STATUS) & 0x80);
}

int ide_identify(uint16_t* buffer) {
    outb(IDE_DRIVE, 0xA0);
    outb(IDE_COMMAND, IDE_CMD_IDENTIFY);

    uint8_t status = inb(IDE_STATUS);
    if (status == 0) return 0;

    if (ide_wait_ready() != 0) return 0;

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(IDE_DATA);
    }
    return 1;
}

void ide_read_sector(uint32_t lba, uint16_t* buffer) {
    outb(IDE_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(IDE_SECCOUNT, 1);
    outb(IDE_LBA_LOW, lba & 0xFF);
    outb(IDE_LBA_MID, (lba >> 8) & 0xFF);
    outb(IDE_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(IDE_COMMAND, IDE_CMD_READ);

    if (ide_wait_ready() != 0) {
        return;
    }

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(IDE_DATA);
    }
}

void ide_write_sector(uint32_t lba, uint16_t* buffer) {
    outb(IDE_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    outb(IDE_SECCOUNT, 1);

    outb(IDE_LBA_LOW,  lba & 0xFF);
    outb(IDE_LBA_MID,  (lba >> 8) & 0xFF);
    outb(IDE_LBA_HIGH, (lba >> 16) & 0xFF);

    outb(IDE_COMMAND, IDE_CMD_WRITE);
    if (ide_wait_ready() != 0) {
        return;
    }

    for (int i = 0; i < 256; i++) {
        outw(IDE_DATA, buffer[i]);
    }

    ide_wait_busy();

    ide_flush_cache();
}

uint16_t ide_find_free(){
    for(int i = 0; i < 256; i++){
        if(ide_queue[i].used == 0){
            return i;
        }
    }

    return 0xFFFF;
}

uint16_t ide_find_last_node(){
    uint16_t node_index = 0;
    while (ide_queue[node_index].next_node != 0xFFFF)
    {
        node_index = ide_queue[node_index].next_node;
    }

    return node_index;
}

uint16_t ide_create_command(uint8_t type, uint32_t memory_address, uint32_t disk_address){
    uint16_t free_slot = ide_find_free();
    if(free_slot == 0xFFFF){
        return 0xFFFF;
    }

    ide_queue[free_slot].used = 1;
    ide_queue[free_slot].type = type;
    ide_queue[free_slot].disk_address = disk_address;
    ide_queue[free_slot].memory_address = memory_address;
    ide_queue[free_slot].next_node = 0xFFFF;
    ide_queue[free_slot].ready = 0;
    uint16_t last_node = ide_find_last_node();
    ide_queue[last_node].next_node = free_slot;

    return free_slot;
}

uint8_t ide_ready(uint16_t slot_index){
    return ide_queue[slot_index].ready;
}

void ide_free_slot(uint16_t slot_index){
    ide_queue[slot_index].used = 0;
}

void ide_process_next(){
    if(ide_queue[0].next_node == 0xFFFF){
        return;
    }
    
    if(ide_queue[ide_queue[0].next_node].type){
        ide_write_sector(ide_queue[ide_queue[0].next_node].disk_address, (uint16_t*)ide_queue[ide_queue[0].next_node].memory_address);
    }else{
        ide_read_sector(ide_queue[ide_queue[0].next_node].disk_address, (uint16_t*)ide_queue[ide_queue[0].next_node].memory_address);   
    }
    
    ide_queue[ide_queue[0].next_node].ready = 1;
    ide_queue[0].next_node = ide_queue[ide_queue[0].next_node].next_node;
}

void ide_init(){
    uint16_t identify_buffer[256];

    if (!ide_identify(identify_buffer)) {
        return;
    }

    ide_queue = (ide_command_t*)kmalloc(256 * sizeof(ide_command_t));
    memset(ide_queue, 0, 256 * sizeof(ide_command_t));
    if ((uint32_t)ide_queue == 0xFFFFFFFF || ide_queue == 0) {
        return;
    }

    for (int i = 0; i < 256; i++) {
        ide_queue[i].used = 0;
        ide_queue[i].next_node = 0xFFFF;
    }

    ide_queue[0].used = 1;
    ide_queue[0].next_node = 0xFFFF;
}

void ide_loop(){
    while(1){
        ide_process_next();
    }
}
