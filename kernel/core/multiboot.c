#include "../../lib/printk.h"
#include "../../drivers/tty.h"
#include <stdint.h>
#include <stdbool.h>

#define MB2_MAGIC_VAL 0x36D76289u
#define TAG_END  0u
#define TAG_MMAP 6u

typedef struct __attribute__((packed)) { uint32_t total, reserved; } mb2_hdr_t;
typedef struct __attribute__((packed)) { uint32_t type, size;      } mb2_tag_t;
typedef struct __attribute__((packed)) {
    uint32_t type, size, entry_size, entry_ver;
} mb2_mmap_tag_t;
typedef struct __attribute__((packed)) {
    uint64_t base, len; uint32_t type, _res;
} mb2_mmap_entry_t;

static const char *mmap_type(uint32_t t) {
    switch(t){case 1:return "Usable  ";case 2:return "Reserved";
              case 3:return "ACPI    ";case 4:return "ACPI NVS";
              case 5:return "Bad RAM ";default:return "Unknown ";}
}

void cmd_mem(uint32_t magic, uint64_t info_addr) {
    if (magic != MB2_MAGIC_VAL) { printk("  No Multiboot2 info.\n"); return; }
    mb2_hdr_t *hdr = (mb2_hdr_t*)(uintptr_t)info_addr;
    uint8_t *ptr = (uint8_t*)hdr + 8;
    uint8_t *end = (uint8_t*)hdr + hdr->total;
    uint64_t usable = 0; bool found = false;

    while (ptr + sizeof(mb2_tag_t) <= end) {
        mb2_tag_t *tag = (mb2_tag_t*)ptr;
        if (tag->type == TAG_END) break;
        if (tag->type == TAG_MMAP) {
            found = true;
            mb2_mmap_tag_t *mt = (mb2_mmap_tag_t*)ptr;
            uint8_t *ep = ptr + sizeof(mb2_mmap_tag_t);
            uint8_t *ee = ptr + tag->size;
            tty_setcolor_info();
            printk("  Base                Len                 Type\n");
            printk("  -----------------------------------------------\n");
            tty_setcolor_reset();
            while (ep + mt->entry_size <= ee) {
                mb2_mmap_entry_t *e = (mb2_mmap_entry_t*)ep;
                printk("  0x%016x  0x%016x  %s\n",
                    e->base, e->len, mmap_type(e->type));
                if (e->type == 1) usable += e->len;
                ep += mt->entry_size;
            }
        }
        ptr += (tag->size + 7) & ~7u;
    }
    if (!found) { printk("  No memory map tag.\n"); return; }
    printk("\n  Usable RAM: %u MiB\n", usable >> 20);
}
