#include "../include/stdio.h"
#include "../include/types.h"
#include "../include/unistd.h"
#include <stdarg.h>

int putchar(int c) {
    char ch = (char)c;
    write(1, &ch, 1);
    return c;
}

int puts(const char *s) {
    while (*s) putchar(*s++);
    putchar('\n');
    return 0;
}

int getchar(void) {
    char c;
    if (read(0, &c, 1) == 1) return (int)(unsigned char)c;
    return -1;
}

static void
_print_uint(uint64_t v, int base, int width, char pad)
{
    static const char H[] = "0123456789abcdef";
    char buf[64]; int i = 63;
    buf[63] = '\0';
    if (!v) buf[--i] = '0';
    else while (v) { buf[--i] = H[v % (uint64_t)base]; v /= (uint64_t)base; }
    int len = 63 - i;
    while (len++ < width) putchar(pad);
    const char *p = buf + i;
    while (*p) putchar(*p++);
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = 0;
    while (*fmt) {
        if (*fmt != '%') { putchar(*fmt++); n++; continue; }
        fmt++;
        char pad = ' '; int width = 0;
        if (*fmt == '0') { pad = '0'; fmt++; }
        while (*fmt >= '1' && *fmt <= '9') { width = width*10+(*fmt-'0'); fmt++; }
        switch (*fmt) {
        case 'd': case 'i': {
            int64_t v = va_arg(ap, int64_t);
            if (v < 0) { putchar('-'); v = -v; n++; }
            _print_uint((uint64_t)v, 10, width, pad); break;
        }
        case 'u': _print_uint(va_arg(ap,uint64_t),10,width,pad); break;
        case 'x': _print_uint(va_arg(ap,uint64_t),16,width,pad); break;
        case 'p': putchar('0'); putchar('x');
                  _print_uint(va_arg(ap,uint64_t),16,16,'0'); break;
        case 's': { const char *s = va_arg(ap,const char*);
                    if (!s) s="(null)";
                    while(*s){putchar(*s++);n++;} break; }
        case 'c': putchar(va_arg(ap,int)); break;
        case '%': putchar('%'); break;
        }
        fmt++; n++;
    }
    va_end(ap);
    return n;
}
