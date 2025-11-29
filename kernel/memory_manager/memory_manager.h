#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <stdint.h>
#include "../../lib/multiboot_info.h"


#define PAGE_FREE 0x0
#define PAGE_USED 0x2


#define PAGE_DIR_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define MAX_PAGE_DIRECTORIES 10


typedef struct {
    uint32_t id;  
    uint32_t pd_address;  
    uint32_t last_pt_address;  
    uint32_t last_pt_index;  
    uint32_t last_entry_index;  
    uint32_t used_tables;  
} page_directory_t;

extern uint32_t memory_size;
extern uint32_t kernel_address;
extern uint8_t *page_map;
extern uint32_t page_map_address;
extern uint32_t total_pages;
extern uint32_t kernel_page_start;
extern page_directory_t page_directories[MAX_PAGE_DIRECTORIES];
extern uint32_t pd_count;

extern uint32_t total_free_pages;

void memory_manager_init(multiboot_info_t* mbi);
uint32_t find_kernel_location(uint32_t size);
uint8_t get_page_status(uint32_t page_num);
void set_page_status(uint32_t page_num, uint8_t status);


page_directory_t* create_page_directory(uint32_t id, uint32_t pd_address);
page_directory_t* get_page_directory(uint32_t id);
void update_page_directory_state(page_directory_t *pd, uint32_t pt_address, 
                                 uint32_t pt_index, uint32_t entry_index);
uint32_t get_next_page_table_position(page_directory_t *pd, 
                                       uint32_t *out_pt_index, 
                                       uint32_t *out_entry_index);

#endif