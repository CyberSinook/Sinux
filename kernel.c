#include <stdbool.h>
#include <stdint.h>
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000003
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
#define VIDEO_MEM 0xB8000
#define WIDTH 80
#define HEIGHT 25
#define PORT_KEYBOARD 0x60
#define PORT_STATUS 0x64
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_HEADER_MAGIC,
    MULTIBOOT_HEADER_FLAGS,
    CHECKSUM,
};
unsigned int stack[4096];
const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u',
    'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',
    'l', ';', '\'', '`', 0, '\\', 'z', 'x',
    'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, '*', 0, ' '
};
int row_offsets[25] = {0};
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}
int strcmp(const char* s1 , const char* s2){
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void disable_cursor(){
    outb(0x3d4 , 0x0a);
    outb(0x3d5 , 0x20);
}
void clear_screen(char color) {
    volatile char* video = (char*) VIDEO_MEM;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        *video++ = ' ';
        *video++ = color;    
    }
    for (int i = 0; i < HEIGHT; i++) {
        row_offsets[i] = 0;
    }
}
void vout(const char* str, int row, char color) {
    volatile char* video = (char*) VIDEO_MEM;
    int offset = row_offsets[row];
    video += row * WIDTH * 2 + offset;
    while (*str && offset < WIDTH * 2) {
        *video++ = *str++;
        *video++ = color;
        offset += 2;
    }
    row_offsets[row] = offset;
}
void vin(char* buffer, char color, int row) {
    volatile char* video = (char*) VIDEO_MEM;
    int offset = row_offsets[row];
    video += row * WIDTH * 2 + offset;
    int index = 0;
    while (1) {
        if (inb(PORT_STATUS) & 1) {
            uint8_t sc = inb(PORT_KEYBOARD);
            char c = (sc < sizeof(scancode_to_ascii)) ? scancode_to_ascii[sc] : 0;
            if (c) {
                if (c == '\n') {
                    buffer[index] = '\0';
                    break;
                }
                else if (c == '\b') {
                    if (index > 0) {
                        index--;
                        offset -= 2;
                        video -= 2;
                        *video++ = ' ';
                        *video++ = color;
                        video -= 2;
                    }
                }
                else {
                    buffer[index++] = c;
                    *video++ = c;
                    *video++ = color;
                    offset += 2;
                }
            }
        }
    }
    row_offsets[row] = offset;
}
void shutdown(){
    outw(0x0604 , 0x2000);
    while(1){
        asm volatile ("hlt");
    }
}
void reboot(){
    uint8_t temp;
    do {
        temp = inb(0x64);
    }
    while (temp & 0x02);
    outb(0x64 , 0xfe);
    while(1){
        asm volatile ("hlt");
    }
}

void system(){
    clear_screen(0x0f);
    vout("  _____   _____   _   _   _    _  __   __     " , 0 , 0x0a);
    vout(" / ____| |_   _| | \\ | | | |  | | \\ \\ / /  " , 1 , 0x0a);
    vout("| (___     | |   |  \\| | | |  | |  \\ V /    " , 2 , 0x0a);
    vout(" \\___ \\    | |   | . ` | | |  | |   > <     " , 3 , 0x0a);
    vout(" ____) |  _| |_  | |\\  | | |__| |  / . \\    " , 4 , 0x0a);
    vout("|_____/  |_____| |_| \\_|  \\____/  /_/ \\_\\ " , 5 , 0x0a);
    vout("==========================================="    , 6 , 0x0f);
    vout("OS : SINUX" , 7 , 0x0f);
    vout("Kernel : Sinux 0.08" , 8 , 0x0f);


}

void kernel_main() {
    disable_cursor();
    bool on = true;
    int numcom = 0;
    char user[15];
    vout("================================> [  " , 0 , 0x0f);
    vout("SINUX" , 0 , 0x0a);
    vout("  ] <=================================" , 0 , 0x0f);

    vout("Username : " , 1 , 0x0f);
    vin(user , 0x0f , 1);

    clear_screen(0x0f);

    while(on){
        char command[160];
        vout("[ " , numcom , 0x0a);
        vout(user , numcom , 0x0b);
        vout(" @ ", numcom , 0x0f);
        vout("SINUX " , numcom , 0x4);
        vout("] > " , numcom , 0x0a);
        vin(command , 0x0f , numcom);
        numcom++;
        if(strcmp(command , "shutdown") == 0){
            shutdown();
        }
        else if(strcmp(command , "reboot") == 0){
            reboot();
        }
        else if(strcmp(command , "clear") == 0){
            clear_screen(0x0f);
            numcom = 0;
        }
        else if(strcmp(command , "system") == 0){
            system();
        }
    }
}
__attribute__((naked)) void _start() {
    asm volatile (
        "mov $stack + 4096, %esp\n"
        "call kernel_main\n"
        "cli\n"
        "hlt\n"
    );
}