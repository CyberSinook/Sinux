#include "tty.h"
#include "vga.h"
#include "serial.h"

void tty_init(void) {
    serial_init();
    vga_init();
}

void tty_putc(char c) { vga_putc(c); serial_putc(c); }
void tty_puts(const char *s) { while(*s) tty_putc(*s++); }

void tty_setcolor_info(void)  { vga_setcolor(LCYAN,  BLACK); }
void tty_setcolor_err(void)   { vga_setcolor(LRED,   BLACK); }
void tty_setcolor_ok(void)    { vga_setcolor(LGREEN, BLACK); }
void tty_setcolor_reset(void) { vga_setcolor(WHITE,  BLACK); }
