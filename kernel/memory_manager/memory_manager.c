#include "../../lib/multiboot_info.h"
#include <stdint.h>
#include <stddef.h>


#define PAGE_SIZE 4096
#define KERNEL_PAGES 512  
#define PAGE_DIR_ENTRIES 1024  
#define PAGE_TABLE_ENTRIES 1024  
#define MAX_PAGE_DIRECTORIES 32  


#define PAGE_FREE 0x0
#define PAGE_USED 0x2


typedef struct {
    uint32_t id;  
    uint32_t pd_address;  
    uint32_t last_pt_address;  
    uint32_t last_pt_index;  
    uint32_t last_entry_index;  
    uint32_t used_tables;  
} page_directory_t;


uint32_t find_kernel_location(uint32_t size);


multiboot_memory_map_t *memory_map;
size_t memory_map_entries;

uint32_t memory_size;
uint32_t kernel_address;  
uint8_t *page_map;  
uint32_t page_map_address;  
uint32_t total_pages;  
uint32_t kernel_page_start;  

uint32_t total_free_pages = 0;


page_directory_t page_directories[MAX_PAGE_DIRECTORIES];
uint32_t pd_count = 0;  

void count_free_pages() {
    total_free_pages = 0;
    for (uint32_t i = 0; i < total_pages; i++) {
        if (page_map[i] == PAGE_FREE) {
            total_free_pages++;
        }
    }
}

extern void create_kernel_page_directory();
void memory_manager_init(multiboot_info_t* mbi){
    memory_map = (multiboot_memory_map_t*)(uintptr_t)(mbi->mmap_addr);
    memory_map_entries = mbi->mmap_length;

    
    memory_size = 0;
    uint32_t max_addr = 0;
    multiboot_memory_map_t *entry = memory_map;
    uint32_t entries_count = mbi->mmap_length / sizeof(multiboot_memory_map_t);
    
    for(uint32_t i = 0; i < entries_count; i++){
        uint32_t end_addr = (uint32_t)(entry->addr + entry->len);
        if(end_addr > max_addr){
            max_addr = end_addr;
        }
        if(entry->type == 1){ 
            memory_size += entry->len;
        }
        entry++;
    }

    
    uint32_t kernel_size = KERNEL_PAGES * PAGE_SIZE;
    kernel_address = find_kernel_location(kernel_size);
    
    if(kernel_address == 0){
        return;  
    }

    
    kernel_page_start = kernel_address / PAGE_SIZE;
    page_map_address = kernel_address + kernel_size;
    
    
    total_pages = (max_addr + PAGE_SIZE - 1) / PAGE_SIZE;
    page_map = (uint8_t*)page_map_address;
    
    
    for(uint32_t i = 0; i < total_pages; i++){
        page_map[i] = PAGE_USED;
    }
    
    
    entry = memory_map;
    for(uint32_t i = 0; i < entries_count; i++){
        if(entry->type == 1){ 
            uint32_t start_page = (uint32_t)(entry->addr / PAGE_SIZE);
            uint32_t end_page = start_page + (entry->len / PAGE_SIZE);
            
            for(uint32_t page = start_page; page < end_page; page++){
                page_map[page] = PAGE_FREE;
            }
        }
        entry++;
    }
    
    
    for(uint32_t i = 0; i < KERNEL_PAGES; i++){
        page_map[kernel_page_start + i] = PAGE_USED;
    }

    create_kernel_page_directory();
    count_free_pages();
}

uint32_t find_kernel_location(uint32_t size){
    multiboot_memory_map_t *entry = memory_map;
    uint32_t entries_count = memory_map_entries / sizeof(multiboot_memory_map_t);
    
    for(uint32_t i = 0; i < entries_count; i++){
        if(entry->type == 1){ 
            
            if(entry->len >= size){
                return (uint32_t)entry->addr;
            }
        }
        entry++;
    }
    
    return 0;  
}

uint8_t get_page_status(uint32_t page_num){
    if(page_num >= total_pages){
        return PAGE_USED;
    }
    return page_map[page_num];
}

void set_page_status(uint32_t page_num, uint8_t status){
    if(page_num < total_pages){
        page_map[page_num] = status;
    }
}

page_directory_t* create_page_directory(uint32_t id, uint32_t pd_address){
    if(pd_count >= MAX_PAGE_DIRECTORIES){
        return NULL;  
    }
    
    page_directory_t *pd = &page_directories[pd_count];
    pd->id = id;
    pd->pd_address = pd_address;
    pd->last_pt_address = 0;
    pd->last_pt_index = 0;
    pd->last_entry_index = 0;
    pd->used_tables = 0;
    
    pd_count++;
    return pd;
}

page_directory_t* get_page_directory(uint32_t id){
    for(uint32_t i = 0; i < pd_count; i++){
        if(page_directories[i].id == id){
            return &page_directories[i];
        }
    }
    return NULL;
}

void update_page_directory_state(page_directory_t *pd, uint32_t pt_address, 
                                 uint32_t pt_index, uint32_t entry_index){
    if(pd == NULL){
        return;
    }
    
    pd->last_pt_address = pt_address;
    pd->last_pt_index = pt_index;
    pd->last_entry_index = entry_index;
    
    
    if(pt_index >= pd->used_tables){
        pd->used_tables = pt_index + 1;
    }
}

uint32_t get_next_page_table_position(page_directory_t *pd, 
                                       uint32_t *out_pt_index, 
                                       uint32_t *out_entry_index){
    if(pd == NULL){
        return 0;
    }
    
    
    if(pd->last_entry_index < PAGE_TABLE_ENTRIES - 1){
        *out_pt_index = pd->last_pt_index;
        *out_entry_index = pd->last_entry_index + 1;
        return 1;
    }
    
    
    if(pd->last_pt_index < PAGE_DIR_ENTRIES - 1){
        *out_pt_index = pd->last_pt_index + 1;
        *out_entry_index = 0;
        return 1;
    }
    
    
    return 0;
}

uint32_t allocate_free_page(void){
    for(uint32_t i = 0; i < total_pages; i++){
        if(page_map[i] == PAGE_FREE){
            return i;
        }
    }
    
    return 0xFFFFFFFF;  
}

uint32_t allocate_page_kernel(void){
    uint32_t page_num = allocate_free_page();
    
    if(page_num == 0xFFFFFFFF){
        return 0;  
    }
    
    
    page_map[page_num] = PAGE_USED;
    total_free_pages--;
    
    
    return page_num * PAGE_SIZE;
}


uint32_t allocate_page(void){
    uint32_t page_num = allocate_free_page();
    
    if(page_num == 0xFFFFFFFF){
        return 0;  
    }
    
    
    page_map[page_num] = 0x1;
    total_free_pages--;
    
    
    return page_num * PAGE_SIZE;
}

uint32_t create_new_page_directory() {
    
    uint32_t pd_address = allocate_page_kernel(); 
    if (pd_address == 0) {
        return 0;  
    }

    
    page_directory_t *pd = create_page_directory(pd_count, pd_address); 
    if (pd == NULL) {
        return 0;  
    }

    
    page_directories[pd_count - 1] = *pd; 

    
    return pd->id;
}

uint32_t reserve_page_in_directory(page_directory_t *pd) {
    if (pd == NULL) {
        return 0xFFFFFFFF;  
    }

    uint32_t pt_index = 0, entry_index = 0;

    
    if (!get_next_page_table_position(pd, &pt_index, &entry_index)) {
        
        uint32_t new_pt_address = allocate_page_kernel();  
        if (new_pt_address == 0) {
            return 0xFFFFFFFF;  
        }

        
        update_page_directory_state(pd, new_pt_address, pt_index + 1, 0);
        pt_index++;
    }

    
    uint32_t page_num = allocate_free_page();  
    if (page_num == 0xFFFFFFFF) {
        return 0xFFFFFFFF;  
    }

    
    set_page_status(page_num, PAGE_USED);  
    
    
    uint32_t pt_address = pd->last_pt_address + pt_index * PAGE_SIZE; 
    update_page_directory_state(pd, pt_address, pt_index, entry_index);

    
    return page_num * PAGE_SIZE;
}


void load_page_directory(uint32_t *pd_address) {
    
    __asm__ volatile(
        "mov %%eax, %%cr3;"      
        "mov %%cr0, %%eax;"      
        "or $0x80000000, %%eax;" 
        "mov %%eax, %%cr0;"      
        :
        : "a"(pd_address)        
        : "memory"
    );
}

void create_kernel_page_directory() {
    uint32_t pd_address = allocate_page_kernel();  
    if (pd_address == 0) {
        return;  
    }

    page_directory_t *first_pd = create_page_directory(0, pd_address);  
    if (first_pd == NULL) {
        return;  
    }

    
    for (uint32_t i = 0; i < total_pages; i++) {
        uint32_t pt_index = 0, entry_index_in_pt = 0;
        uint32_t pt_address = 0;

        
        if (!get_next_page_table_position(first_pd, &pt_index, &entry_index_in_pt)) {
            pt_address = allocate_page_kernel();
            if (pt_address == 0) {
                return;  
            }
            update_page_directory_state(first_pd, pt_address, pt_index + 1, 0);
        }

        set_page_status(i, PAGE_USED);  
        uint32_t pt_entry = i * PAGE_SIZE;
        *((uint32_t*)pt_address + entry_index_in_pt) = pt_entry | 0x3; 
    }

    
    load_page_directory((uint32_t *)pd_address);  

    
    page_directories[0] = *first_pd;
}

void create_user_page_directory(){
    uint32_t pd_address = allocate_page_kernel();  
    if (pd_address == 0) {
        return;  
    }

    page_directory_t *user_pd = create_page_directory(0, pd_address);  
    if (user_pd == NULL) {
        return;  
    }

    page_directories[1] = *user_pd;
}