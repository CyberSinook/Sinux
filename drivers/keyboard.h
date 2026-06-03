#pragma once
#include <stdbool.h>
void keyboard_init(void);
void handle_keyboard(void);
char kbd_getc(void);
void kbd_readline(char *buf, int max);
bool kbd_haschar(void);
