#include "keyboard.h"
#include "tty.h"
#include "../arch/x86_64/pic.h"
#include "../lib/io.h"
#include <stdbool.h>
#include <stdint.h>

static const char sc_normal[128]={
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '
};
static const char sc_shift[128]={
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' '
};

#define KBUF 512
static volatile char buf[KBUF];
static volatile int  head=0, tail=0;
static bool shift=false;

void handle_keyboard(void) {
    uint8_t sc = inb(0x60);
    if (sc==0x2A||sc==0x36)      shift=true;
    else if(sc==0xAA||sc==0xB6)  shift=false;
    else if(!(sc&0x80)&&sc<128) {
        char c = shift ? sc_shift[sc] : sc_normal[sc];
        if(c) {
            int next=(head+1)%KBUF;
            if(next!=tail){buf[head]=c;head=next;}
        }
    }
    pic_eoi(1);
}

bool kbd_haschar(void) { return head != tail; }

char kbd_getc(void) {
    while(tail==head) __asm__ volatile("pause");
    char c=buf[tail]; tail=(tail+1)%KBUF; return c;
}

void kbd_readline(char *b, int max) {
    int n=0;
    while(n<max-1){
        char c=kbd_getc();
        if(c=='\n'){tty_putc('\n');break;}
        if(c=='\b'){if(n>0){n--;tty_putc('\b');}}
        else{b[n++]=c;tty_putc(c);}
    }
    b[n]='\0';
}

void keyboard_init(void) { pic_unmask(1); }
