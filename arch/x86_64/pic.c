#include "pic.h"
#include "../../lib/io.h"

#define PIC1_CMD  0x20
#define PIC1_DAT  0x21
#define PIC2_CMD  0xA0
#define PIC2_DAT  0xA1

void pic_init(void) {
    outb(PIC1_CMD, 0x11); io_wait();
    outb(PIC2_CMD, 0x11); io_wait();
    outb(PIC1_DAT, 0x20); io_wait();
    outb(PIC2_DAT, 0x28); io_wait();
    outb(PIC1_DAT, 4);    io_wait();
    outb(PIC2_DAT, 2);    io_wait();
    outb(PIC1_DAT, 0x01); io_wait();
    outb(PIC2_DAT, 0x01); io_wait();

    outb(PIC1_DAT, 0xFF);
    outb(PIC2_DAT, 0xFF);
}

void pic_eoi(int irq) {
    if (irq >= 8) outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void pic_mask(int irq) {
    uint16_t port = (irq < 8) ? PIC1_DAT : PIC2_DAT;
    int      bit  = irq % 8;
    outb(port, (uint8_t)(inb(port) | (1 << bit)));
}

void pic_unmask(int irq) {
    uint16_t port = (irq < 8) ? PIC1_DAT : PIC2_DAT;
    int      bit  = irq % 8;
    outb(port, (uint8_t)(inb(port) & ~(1 << bit)));
}
