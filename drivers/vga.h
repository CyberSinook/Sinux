#pragma once
#include <stdint.h>

enum vga_color {
    BLACK=0,BLUE,GREEN,CYAN,RED,MAGENTA,BROWN,LGRAY,
    DGRAY,LBLUE,LGREEN,LCYAN,LRED,LMAGENTA,YELLOW,WHITE
};

void vga_init(void);
void vga_setcolor(enum vga_color fg, enum vga_color bg);
void vga_putc(char c);
void vga_puts(const char *s);
void vga_clear(void);
