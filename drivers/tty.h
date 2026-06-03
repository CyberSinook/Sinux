#pragma once

void tty_init(void);
void tty_putc(char c);
void tty_puts(const char *s);
void tty_setcolor_info(void);
void tty_setcolor_err(void);
void tty_setcolor_ok(void);
void tty_setcolor_reset(void);
