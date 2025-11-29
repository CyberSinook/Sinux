#include "ps2_keyboard.h"
#include "../../../lib/inout.h"
#include "../../../lib/key_code.h"

KeyMapEntry key_codes[256] = {
    [0x1E] = {.type=0, .key_code=KEYCODE_A, .character='a'},
    [0x1E + 0x80] = {.type=1, .key_code=KEYCODE_A, .character='a'},

    [0x30] = {.type=0, .key_code=KEYCODE_B, .character='b'},
    [0x30 + 0x80] = {.type=1, .key_code=KEYCODE_B, .character='b'},

    [0x2E] = {.type=0, .key_code=KEYCODE_C, .character='c'},
    [0x2E + 0x80] = {.type=1, .key_code=KEYCODE_C, .character='c'},

    [0x20] = {.type=0, .key_code=KEYCODE_D, .character='d'},
    [0x20 + 0x80] = {.type=1, .key_code=KEYCODE_D, .character='d'},

    [0x12] = {.type=0, .key_code=KEYCODE_E, .character='e'},
    [0x12 + 0x80] = {.type=1, .key_code=KEYCODE_E, .character='e'},

    [0x21] = {.type=0, .key_code=KEYCODE_F, .character='f'},
    [0x21 + 0x80] = {.type=1, .key_code=KEYCODE_F, .character='f'},

    [0x22] = {.type=0, .key_code=KEYCODE_G, .character='g'},
    [0x22 + 0x80] = {.type=1, .key_code=KEYCODE_G, .character='g'},

    [0x23] = {.type=0, .key_code=KEYCODE_H, .character='h'},
    [0x23 + 0x80] = {.type=1, .key_code=KEYCODE_H, .character='h'},

    [0x17] = {.type=0, .key_code=KEYCODE_I, .character='i'},
    [0x17 + 0x80] = {.type=1, .key_code=KEYCODE_I, .character='i'},

    [0x24] = {.type=0, .key_code=KEYCODE_J, .character='j'},
    [0x24 + 0x80] = {.type=1, .key_code=KEYCODE_J, .character='j'},

    [0x25] = {.type=0, .key_code=KEYCODE_K, .character='k'},
    [0x25 + 0x80] = {.type=1, .key_code=KEYCODE_K, .character='k'},

    [0x26] = {.type=0, .key_code=KEYCODE_L, .character='l'},
    [0x26 + 0x80] = {.type=1, .key_code=KEYCODE_L, .character='l'},

    [0x32] = {.type=0, .key_code=KEYCODE_M, .character='m'},
    [0x32 + 0x80] = {.type=1, .key_code=KEYCODE_M, .character='m'},

    [0x31] = {.type=0, .key_code=KEYCODE_N, .character='n'},
    [0x31 + 0x80] = {.type=1, .key_code=KEYCODE_N, .character='n'},

    [0x18] = {.type=0, .key_code=KEYCODE_O, .character='o'},
    [0x18 + 0x80] = {.type=1, .key_code=KEYCODE_O, .character='o'},

    [0x19] = {.type=0, .key_code=KEYCODE_P, .character='p'},
    [0x19 + 0x80] = {.type=1, .key_code=KEYCODE_P, .character='p'},

    [0x10] = {.type=0, .key_code=KEYCODE_Q, .character='q'},
    [0x10 + 0x80] = {.type=1, .key_code=KEYCODE_Q, .character='q'},

    [0x13] = {.type=0, .key_code=KEYCODE_R, .character='r'},
    [0x13 + 0x80] = {.type=1, .key_code=KEYCODE_R, .character='r'},

    [0x1F] = {.type=0, .key_code=KEYCODE_S, .character='s'},
    [0x1F + 0x80] = {.type=1, .key_code=KEYCODE_S, .character='s'},

    [0x14] = {.type=0, .key_code=KEYCODE_T, .character='t'},
    [0x14 + 0x80] = {.type=1, .key_code=KEYCODE_T, .character='t'},

    [0x16] = {.type=0, .key_code=KEYCODE_U, .character='u'},
    [0x16 + 0x80] = {.type=1, .key_code=KEYCODE_U, .character='u'},

    [0x2F] = {.type=0, .key_code=KEYCODE_V, .character='v'},
    [0x2F + 0x80] = {.type=1, .key_code=KEYCODE_V, .character='v'},

    [0x11] = {.type=0, .key_code=KEYCODE_W, .character='w'},
    [0x11 + 0x80] = {.type=1, .key_code=KEYCODE_W, .character='w'},

    [0x2D] = {.type=0, .key_code=KEYCODE_X, .character='x'},
    [0x2D + 0x80] = {.type=1, .key_code=KEYCODE_X, .character='x'},

    [0x15] = {.type=0, .key_code=KEYCODE_Y, .character='y'},
    [0x15 + 0x80] = {.type=1, .key_code=KEYCODE_Y, .character='y'},

    [0x2C] = {.type=0, .key_code=KEYCODE_Z, .character='z'},
    [0x2C + 0x80] = {.type=1, .key_code=KEYCODE_Z, .character='z'},

    [0x1C] = {.type=0, .key_code=KEYCODE_ENTER, .character='\n'},
    [0x1C + 0x80] = {.type=1, .key_code=KEYCODE_ENTER, .character='\n'},

    [0x39] = {.type=0, .key_code=KEYCODE_SPACE, .character=' '},
    [0x39 + 0x80] = {.type=1, .key_code=KEYCODE_SPACE, .character=' '},

    [0x0E] = {.type=0, .key_code=KEYCODE_BACKSPACE, .character='\b'},
    [0x0E + 0x80] = {.type=1, .key_code=KEYCODE_BACKSPACE, .character='\b'},

    [0x34] = {.type=0, .key_code=KEYCODE_DOT_MAIN, .character='.'},
    [0x34 + 0x80] = {.type=1, .key_code=KEYCODE_DOT_MAIN, .character='.'},

    [0x4B] = {.type=0, .key_code=KEYCODE_ARROW_LEFT},
    [0x4B + 0x80] = {.type=1, .key_code=KEYCODE_ARROW_LEFT},
};

extern void on_key_pressed(KeyMapEntry key_code);
extern void on_key_released(KeyMapEntry key_code);
void handle_keyboard(void) {
    uint8_t scancode = inb(0x60);
    KeyMapEntry key_code = key_codes[scancode];

    if(key_code.key_code == 0) {
        return;
    }

    if(key_code.type) {
        on_key_released(key_code);
    } else {
        on_key_pressed(key_code);
    }
}