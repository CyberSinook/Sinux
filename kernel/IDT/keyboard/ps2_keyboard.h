


#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H
#include <stdint.h>

typedef struct {
    uint8_t type;
    uint8_t key_code;
    char character;
} KeyMapEntry;

extern void keyboard_handler_irq1(void);

#endif