#include <stdint.h>
#include <kernel/arch/x86/mm/heap.h>
#include <kernel/arch/x86/mm/pmm.h>

#define KERNEL_HEAP_PAGES 8182
#define HEAP_FREE            1
#define HEAP_RESERVED        2

heap_unit *heap_units;
uint32_t heap_location;
uint32_t heap_first = 1;
uint32_t last_empty_search = 1;

uint32_t heap_search_empty_he(){
    uint32_t start_index = last_empty_search;
    do
    {
        if(heap_units[last_empty_search].type == 0)
            return last_empty_search;

        if(last_empty_search >= 511){
            last_empty_search = 0;
            continue;
        }
        last_empty_search++;
    } while (start_index != last_empty_search);
    
    return 0xFFFFFFFF;
}

uint32_t kmalloc(uint32_t size){
    uint32_t unit_index = heap_first;

    do
    {
        if(heap_units[unit_index].type == 1 && heap_units[unit_index].size >= size){
            if(heap_units[unit_index].size == size){
                heap_units[unit_index].type = HEAP_RESERVED;

                return heap_units[unit_index].start;
            }

            uint32_t new_unit_index = heap_search_empty_he();

            if(new_unit_index == 0xFFFFFFFF){
                return 0xFFFFFFFF;
            }

            heap_units[new_unit_index].type = HEAP_RESERVED;
            heap_units[new_unit_index].size = size;
            heap_units[new_unit_index].start = heap_units[unit_index].start;
            heap_units[new_unit_index].next_unit = unit_index;
            heap_units[new_unit_index].last_unit = heap_units[unit_index].last_unit;
            heap_units[heap_units[unit_index].last_unit].next_unit = new_unit_index;
            heap_units[unit_index].last_unit = new_unit_index;
            heap_units[unit_index].size -= size;
            heap_units[unit_index].start += size;

            return heap_units[new_unit_index].start;
        }
        unit_index = heap_units[unit_index].next_unit;
    }while (unit_index != 0xFFFFFFFF);

    return 0xFFFFFFFF;
}

void kfree(uint32_t address){
    uint32_t unit_index = heap_first;
    do
    {
        if(heap_units[unit_index].start == address){
            if(heap_units[heap_units[unit_index].last_unit].type == HEAP_FREE){
                heap_units[unit_index].size += heap_units[heap_units[unit_index].last_unit].size;
                heap_units[unit_index].start -= heap_units[heap_units[unit_index].last_unit].size;
                heap_units[heap_units[unit_index].last_unit].type = 0;
                heap_units[unit_index].last_unit = heap_units[heap_units[unit_index].last_unit].last_unit;
                heap_units[heap_units[heap_units[unit_index].last_unit].last_unit].next_unit = unit_index;
            }

            if(heap_units[unit_index].next_unit != 0xFFFFFFFF && heap_units[heap_units[unit_index].next_unit].type == HEAP_FREE){
                heap_units[unit_index].size += heap_units[heap_units[unit_index].next_unit].size;
                heap_units[heap_units[unit_index].next_unit].type = 0;
                if(heap_units[heap_units[unit_index].next_unit].next_unit != 0xFFFFFFFF){
                    heap_units[heap_units[heap_units[unit_index].next_unit].next_unit].last_unit = unit_index;
                }
                heap_units[unit_index].next_unit = heap_units[heap_units[unit_index].next_unit].next_unit;
            }

            heap_units[unit_index].type = HEAP_FREE;
        }
        unit_index = heap_units[unit_index].next_unit;
    } while (unit_index != 0xFFFFFFFF);
}

void heap_init(){
    uint32_t heap_units_address = pmm_reserve_kernel_page(2);

    heap_units = (heap_unit*)heap_units_address;
    heap_location = pmm_reserve_kernel_page(KERNEL_HEAP_PAGES);

    if(heap_location == 0xFFFFFFFF || heap_units_address == 0xFFFFFFFF){
        while (1){
            asm volatile("hlt");
        }
    }

    heap_units[1].size = 0;
    heap_units[1].type = 2;
    heap_units[1].start = 0;
    heap_units[1].last_unit = 0xFFFFFFFF;
    heap_units[1].next_unit = 1;

    heap_units[1].size = KERNEL_HEAP_PAGES * 4096;
    heap_units[1].type = 1;
    heap_units[1].start = heap_location;
    heap_units[1].last_unit = 0xFFFFFFFF;
    heap_units[1].next_unit = 0xFFFFFFFF;
}