#include <kernel/arch/x86/interrupts/idt.h>
#include <kernel/arch/x86/mm/heap.h>
#include <lib/inout.h>
#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x11
#define ICW4_8086    0x01

void pic_remap() {
    uint8_t a1, a2;
    int offset1 = 0x20;
    int offset2 = 0x28;
    
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    
    outb(PIC1_COMMAND, ICW1_INIT);
    outb(PIC2_COMMAND, ICW1_INIT);

    
    outb(PIC1_DATA, offset1); 
    outb(PIC2_DATA, offset2); 

    
    outb(PIC1_DATA, 4); 
    outb(PIC2_DATA, 2);

    
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

IDT_entry_t *IDT;

void set_IDT_entry(int num, uint32_t base, uint16_t selector, uint8_t type_attr) {
    IDT[num].offset_low  = base & 0xFFFF;          
    IDT[num].selector    = selector;               
    IDT[num].zero        = 0;                      
    IDT[num].type_attr   = type_attr;              
    IDT[num].offset_high = (base >> 16) & 0xFFFF;  
}

void pic_mask_all() {
    outb(0x21, 0xFF); // Master PIC
    outb(0xA1, 0xFF); // Slave PIC
}

void pic_unmask_irq(uint8_t irq) {
    if(irq < 8) { // Master
        uint8_t mask = inb(0x21);
        mask &= ~(1 << irq);
        outb(0x21, mask);
    } else {      // Slave
        irq -= 8;
        uint8_t mask = inb(0xA1);
        mask &= ~(1 << irq);
        outb(0xA1, mask);
    }
}

extern void isr_default_handler();
extern void irq0_handler();
void idt_init(){
    IDT = (IDT_entry_t*)kmalloc(sizeof(IDT_entry_t)*256);

    if((uintptr_t)IDT == 0xFFFFFFFF){
        while (1)
        {
            asm volatile("hlt");
        }
    }

    pic_remap();

    IDT_ptr_t idtp;
    idtp.limit = (sizeof(IDT_entry_t) * 256) - 1;
    idtp.base = (uint32_t)IDT;

    pic_mask_all();

    asm volatile("lidt %0" : : "m"(idtp));
}