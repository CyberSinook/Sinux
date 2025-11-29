#include <stdbool.h>
#include <stdint.h>
#include "lib/string.c"
#include "kernel/video/video.c"
#include "ramfs/ramfs.c"
#include "kernel/memory_manager/memory_manager.h"
#include "lib/inout.h"
#include "kernel/IDT/IDT.h"
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000003
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_HEADER_MAGIC, 
    MULTIBOOT_HEADER_FLAGS,
    CHECKSUM,
};

unsigned int stack[4096];


#define SYSCALL(syscal_number, arg1, arg2) ({ \
    long ret; \
    asm volatile ( \
        "int $0x80" \
        : "=a"(ret) \
        : "a"(syscal_number), "b"(arg1), "c"(arg2) \
        : "memory"); \
    ret; \
})


void disable_cursor(){
    outb(0x3d4 , 0x0a);
    outb(0x3d5 , 0x20);
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

volatile uint32_t timer_count = 0;
volatile uint32_t sleep_start;
volatile uint32_t sleep_interval;
void handle_timer(){
    timer_count++;
    if(sleep_interval != 0 && timer_count-sleep_start >= sleep_interval){
        sleep_interval = 0;
    }
    outb(0xF8, '"');
}

void sleep(uint32_t ticks){
    sleep_start = timer_count;
    sleep_interval = ticks;
    while(sleep_interval != 0){
        asm volatile("hlt");
    }
}

void system(){
    clear_screen();
    default_color.foreground = VGA_FG_GREEN;
    print_string("  _____   _____   _   _   _    _  __   __     \n");
    print_string(" / ____| |_   _| | \\ | | | |  | | \\ \\ / /  \n");
    print_string("| (___     | |   |  \\| | | |  | |  \\ V /    \n");
    print_string(" \\___ \\    | |   | . ` | | |  | |   > <     \n");
    print_string(" ____) |  _| |_  | |\\  | | |__| |  / . \\    \n");
    print_string("|_____/  |_____| |_| \\_|  \\____/  /_/ \\_\\ \n");
    default_color.foreground = VGA_FG_WHITE;
    print_string("===========================================   \n");
    print_string("OS : SINUX\n");
    print_string("Kernel : Sinux 0.01\n");
    print_string("Memory Size : ");
    char size[20];
    itoa(memory_size / (1024 * 1024), size);
    print_string(size);
    print_string(" MB\n");
    print_string("Free Memory : ");
    char free_size[20];
    itoa(total_free_pages * 4096 / (1024 * 1024), free_size);
    print_string(free_size);
    print_string(" MB\n");
    print_string("===========================================\n");
}

void help() {
    default_color.foreground = VGA_FG_WHITE;
    print_string("Available Commands:\n");
    print_string("============================================================\n");
    print_string("  shutdown                - Power off the system\n");
    print_string("  reboot                  - Reboot the system\n");
    print_string("  clear                   - Clear the screen\n");
    print_string("  system                  - Display system information\n");
    print_string("  ls                      - List files in the RAMFS\n");
    print_string("  help                    - Show this help message\n");
    print_string("  modules                 - Show modules list\n");
    print_string("  syscal                  - Send test clear screen syscal\n");
    print_string("============================================================\n");
}

void red_screen_error(){  
    clear_screen();
    default_color.background = VGA_BG_RED;
    default_color.foreground = VGA_FG_WHITE;
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("   #######                                                                      ");
    print_string("   #    # #                                                                     ");
    print_string("   #    #  #                                                                    ");
    print_string("   #    #   #                                                                   ");
    print_string("   #    #####                                                                   ");
    print_string("   #        #                                                                   ");
    print_string("   #        #                                                                   ");
    print_string("   # #    # #                                                                   ");
    print_string("   #  ####  #     Sinux Is Dead.                                                ");
    print_string("   ##########                                                                   ");
    print_string("                                                                                ");
    print_string("   CRITICAL ERROR: The kernel asked for help. I refused. Now we're here.        ");
    print_string("   RED SCREEN: If you're reading this, congratulations. You found a new         ");
    print_string("   bug species.                                                                 ");
    print_string("                                                                                ");
    print_string("   Last Instruction: Whatever it was, it didn't work.                           ");
    print_string("   CPU State: Confused.                                                         ");
    print_string("   Memory State: Screaming.                                                     ");
    print_string("                                                                                ");
    print_string("   Advice: Restart the system. Then fix your kernel, genius.                    ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");


    asm volatile("hlt");
}

char user[15];
void login(){
    default_color.background = VGA_BG_BLUE;
    default_color.foreground = VGA_FG_WHITE;
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                    ########################################                    ");
    print_string("                    # Username # : #                       #                    ");
    print_string("                    ########################################                    ");
    print_string("                    ########################################                    ");
    print_string("                    # Password # : #                       #                    ");
    print_string("                    ########################################                    ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");
    print_string("                                                                                ");

    cursor_pos.row = 15;
    cursor_pos.column = 37;
    vin(user);
    char password[15];
    cursor_pos.row = 18;
    cursor_pos.column = 37;
    vin(password);
}

int process_command(char command[160], char parameters_buffer[5][100]){
    uint8_t index = 0;
    uint8_t parameter_number = 0;
    uint8_t parameter_index = 0;

    while(index < 160){
        parameter_index = 0;
        while (index < 160 && command[index] == ' '){
            index++;
        }

        uint8_t is_p = 0;

        while(index < 160 && command[index] != ' '){
            parameters_buffer[parameter_number][parameter_index] = command[index];
            index++;
            parameter_index++;
            is_p = 1;
        }
        parameter_number+=is_p;
    }

    return parameter_number;
}

multiboot_info_t* mbi_global;

void kernel_main(multiboot_info_t* mbi) {
    mbi_global = mbi;

    disable_cursor();
    init_ramfs(mbi);
    memory_manager_init(mbi);
    init_IDT();

    asm volatile("sti"); 

    login();

    bool on = true;
    default_color.foreground = VGA_FG_WHITE;
    default_color.background = VGA_BG_BLACK;

    clear_screen();

    while(on){
        char command[160];
        default_color.foreground = VGA_FG_LIGHT_GREEN;
        print_string("[ ");
        default_color.foreground = VGA_FG_LIGHT_CYAN;
        print_string(user);
        default_color.foreground = VGA_FG_WHITE;
        print_string(" @ ");
        default_color.foreground = VGA_FG_RED;
        print_string("SINUX ");
        default_color.foreground = VGA_FG_LIGHT_GREEN;
        print_string("] > ");
        default_color.foreground = VGA_FG_WHITE;
        vin(command);

        char parameters[5][100];
        uint8_t parameter_count = process_command(command, parameters);

        if(strcmp(command , "shutdown") == 0){
            shutdown();
        }
        else if(strcmp(command , "reboot") == 0){
            reboot();
        }
        else if(strcmp(command , "clear") == 0){
            clear_screen();
        }
        else if(strcmp(command , "system") == 0){
            system();
        }else if(strcmp(command, "ls") == 0){
            print_char('\n');
            int file_index = 0;
            while (file_index < 256 && ramfs_header[file_index].used) {
                char size[12];
                itoa(ramfs_header[file_index].end - ramfs_header[file_index].start, size);
                print_string(ramfs_header[file_index].name);
                print_string("  Size: ");
                print_string(size);
                print_string(" Start: ");
                char start[15];
                itoa(ramfs_header[file_index].start, start);
                print_string(start);
                print_char('\n');
                file_index++;
            }
        }else if(strcmp(command , "help") == 0){
            help();
        }else if(strcmp(command , "modules") == 0){
            print_string("Modules Count: ");
            char modcount[10];
            itoa(mbi->mods_count , modcount);
            print_string(modcount);
            print_char('\n');
            for(uint32_t i = 0; i < mbi->mods_count; i++){
                multiboot_module_t* mod = (multiboot_module_t*)(uintptr_t)(mbi->mods_addr + i * sizeof(multiboot_module_t));
                print_string("Module ");
                char modnum[10];
                itoa(i , modnum);
                print_string(modnum);
                print_string(" Start: ");
                char modstart[20];
                itoa(mod->mod_start , modstart);
                print_string(modstart);
                print_string(" End: ");
                char modend[20];
                itoa(mod->mod_end , modend);
                print_string(modend);
                print_char('\n');
            }
        }else if(strcmp(command, "syscal") == 0){
            SYSCALL(0, 0, 0);
        }else if(strcmp(command, "redscreen") == 0){
            __asm__ __volatile__(
                "mov $0xdead, %ax\n"
                "mov %ax, %ds\n"
                "movb $1, 0x0\n"
            );
        }else{
            print_string("Unknown command: \n");
            print_string(command);
        }
    }
}
__attribute__((naked)) void _start() {
    asm volatile (
        "mov $stack + 4096, %esp\n"
        "push %ebx\n"
        "call kernel_main\n"
        "cli\n"
        "hlt\n"
    );
}