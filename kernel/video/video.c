#include <stdint.h>
#include "../../lib/inout.h"

typedef struct __attribute__((packed)) {
    uint8_t foreground : 4;  
    uint8_t background : 3;  
    uint8_t blink      : 1;  
} VGAColor;

typedef struct __attribute__((packed)) {
    char character;  
    VGAColor color;  
} VGAEntry;

typedef struct {
    uint8_t row;
    uint8_t column;
} cursor_position;

typedef struct {
    uint8_t type;
    uint8_t key_code;
    char character;
} KeyMapEntry;


typedef enum {
    VGA_FG_BLACK        = 0x0,
    VGA_FG_BLUE         = 0x1,
    VGA_FG_GREEN        = 0x2,
    VGA_FG_CYAN         = 0x3,
    VGA_FG_RED          = 0x4,
    VGA_FG_MAGENTA      = 0x5,
    VGA_FG_BROWN        = 0x6,
    VGA_FG_LIGHT_GRAY   = 0x7,
    VGA_FG_DARK_GRAY    = 0x8,
    VGA_FG_LIGHT_BLUE   = 0x9,
    VGA_FG_LIGHT_GREEN  = 0xA,
    VGA_FG_LIGHT_CYAN   = 0xB,
    VGA_FG_LIGHT_RED    = 0xC,
    VGA_FG_LIGHT_MAGENTA= 0xD,
    VGA_FG_YELLOW       = 0xE,
    VGA_FG_WHITE        = 0xF
} VGA_ForegroundColor;


typedef enum {
    VGA_BG_BLACK        = 0x0,
    VGA_BG_BLUE         = 0x1,
    VGA_BG_GREEN        = 0x2,
    VGA_BG_CYAN         = 0x3,
    VGA_BG_RED          = 0x4,
    VGA_BG_MAGENTA      = 0x5,
    VGA_BG_BROWN        = 0x6,
    VGA_BG_LIGHT_GRAY   = 0x7
} VGA_BackgroundColor;

VGAEntry (*vga_buffer)[80] = (VGAEntry(*)[80])0xB8000;

cursor_position cursor_pos = {0, 0};

VGAColor default_color = { .foreground = 15, .background = 0, .blink = 0 };

void scroll_up() {
    for (int y = 1; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            vga_buffer[y - 1][x] = vga_buffer[y][x];
        }
    }

    for (int x = 0; x < 80; x++) {
        vga_buffer[25 - 1][x].character = ' ';
        vga_buffer[25 - 1][x].color = default_color;
    }
}

void next_cursor_position() {
    cursor_pos.column++;
    vga_buffer[cursor_pos.row][cursor_pos.column].color.blink = 0;

    if (cursor_pos.column >= 80) {
        cursor_pos.column = 0;
        cursor_pos.row++;
        if (cursor_pos.row >= 25) {
            scroll_up();
            cursor_pos.row = 24;
        }
    }
}

uint8_t timer = 0;
extern uint8_t inputting;
void handle_cursor(){
    timer++;
    if(timer >= 9){
        vga_buffer[cursor_pos.row][cursor_pos.column].color.blink = !vga_buffer[cursor_pos.row][cursor_pos.column].color.blink;
        timer = 0;
    }
}

void last_cursor_position() {
    if (cursor_pos.column == 0) {
        if (cursor_pos.row > 0) {
            cursor_pos.row--;
            cursor_pos.column = 79;
        }
    } else {
        cursor_pos.column--;
    }
}

void print_char(char c) {
    if (c == '\n') {
        cursor_pos.column = 0;
        cursor_pos.row++;
        if (cursor_pos.row >= 25) {
            scroll_up();
            cursor_pos.row = 24;
        }
    } else if (c == '\r') {
        cursor_pos.column = 0;
    } else {
        vga_buffer[cursor_pos.row][cursor_pos.column].character = c;
        vga_buffer[cursor_pos.row][cursor_pos.column].color = default_color;
        next_cursor_position();
    }
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void clear_screen() {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            vga_buffer[y][x].character = ' ';
            vga_buffer[y][x].color = default_color;
        }
    }
    cursor_pos.row = 0;
    cursor_pos.column = 0;
}

uint8_t vin_input = 0x0;
char vin_input_char = 0;
uint8_t inputting = 0;
void vin(char* buffer) {
    inputting = 1;
    int index = 0;
    vin_input = 0x0;
    while (1) {
        while (vin_input == 0x0) {
            asm volatile ("hlt");
        }
        if(vin_input_char == '\n'){
            inputting = 0;
            vga_buffer[cursor_pos.row][cursor_pos.column].color.blink = 0;
            buffer[index] = '\0';
            print_char('\n');
            vin_input = 0x0;
            break;
        }
        else if(vin_input_char == '\b'){
            if(index > 0){
                index--;
                buffer[index] = ' ';
                last_cursor_position();
                print_char(' ');
                last_cursor_position();
            }
        }
        else{        
            buffer[index] = vin_input_char;
            print_char(vin_input_char);

            index++;
        }
        vin_input = 0x0;
    }
}

void on_key_pressed(KeyMapEntry key_code){
    if(key_code.character == 0)
        return;
    vin_input_char = key_code.character;
    vin_input = 0x1;
}

void on_key_released(KeyMapEntry key_code){
}

void init_video() {
    clear_screen();
}