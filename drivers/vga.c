#include "vga.h"
#include "../lib/io.h"

#define VGA_W   80
#define VGA_H   25
#define VGA_MEM ((volatile uint16_t*)0xB8000)

static int     row=0, col=0;
static uint8_t attr=0;

void vga_setcolor(enum vga_color fg, enum vga_color bg) {
    attr = (uint8_t)(((uint8_t)bg<<4)|(uint8_t)fg);
}

static void update_cursor(void) {
    uint16_t pos = (uint16_t)(row*VGA_W+col);
    outb(0x3D4,14); outb(0x3D5,(uint8_t)(pos>>8));
    outb(0x3D4,15); outb(0x3D5,(uint8_t)(pos&0xFF));
}

static void scroll(void) {
    for (int r=0;r<VGA_H-1;r++)
        for (int c=0;c<VGA_W;c++)
            VGA_MEM[r*VGA_W+c]=VGA_MEM[(r+1)*VGA_W+c];
    for (int c=0;c<VGA_W;c++)
        VGA_MEM[(VGA_H-1)*VGA_W+c]=(uint16_t)(' ')|((uint16_t)attr<<8);
    row=VGA_H-1;
}

void vga_putc(char c) {
    if (c=='\n')      { col=0; row++; }
    else if (c=='\r') { col=0; }
    else if (c=='\b') {
        if(col>0) col--;
        VGA_MEM[row*VGA_W+col]=(uint16_t)(' ')|((uint16_t)attr<<8);
    } else {
        VGA_MEM[row*VGA_W+col]=(uint16_t)(uint8_t)c|((uint16_t)attr<<8);
        if(++col>=VGA_W){col=0;row++;}
    }
    if(row>=VGA_H) scroll();
    update_cursor();
}

void vga_puts(const char *s) { while(*s) vga_putc(*s++); }

void vga_clear(void) {
    for(int i=0;i<VGA_W*VGA_H;i++)
        VGA_MEM[i]=(uint16_t)(' ')|((uint16_t)attr<<8);
    row=0; col=0; update_cursor();
}

void vga_init(void) {
    vga_setcolor(WHITE, BLACK);
    vga_clear();
}
