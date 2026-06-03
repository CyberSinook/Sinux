#pragma once
void serial_init(void);
void serial_putc(char c);
void serial_puts(const char *s);
char serial_getc(void);
