#include "printk.h"
#include "../drivers/tty.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

static void
_print_uint(uint64_t v, int base, bool upper, int width, char pad)
{
    static const char lo[] = "0123456789abcdef";
    static const char hi[] = "0123456789ABCDEF";
    const char *d = upper ? hi : lo;
    char buf[64]; int i = 63;
    buf[63] = '\0';
    if (!v) buf[--i] = '0';
    else while (v) { buf[--i] = d[v % (uint64_t)base]; v /= (uint64_t)base; }
    int len = 63 - i;
    while (len++ < width) tty_putc(pad);
    tty_puts(buf + i);
}

void
printk(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt != '%') { tty_putc(*fmt++); continue; }
        fmt++;

        char pad  = ' ';
        int  width = 0;
        if (*fmt == '0') { pad = '0'; fmt++; }
        while (*fmt >= '1' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0'); fmt++;
        }

        switch (*fmt) {
        case 'd':
        case 'i': {
            int64_t v = va_arg(ap, int64_t);
            if (v < 0) { tty_putc('-'); v = -v; }
            _print_uint((uint64_t)v, 10, false, width, pad);
            break;
        }
        case 'u': _print_uint(va_arg(ap,uint64_t), 10, false, width, pad); break;
        case 'x': _print_uint(va_arg(ap,uint64_t), 16, false, width, pad); break;
        case 'X': _print_uint(va_arg(ap,uint64_t), 16, true,  width, pad); break;
        case 'p': tty_puts("0x");
                  _print_uint(va_arg(ap,uint64_t), 16, false, 16, '0');    break;
        case 's': { const char *s = va_arg(ap, const char *);
                    tty_puts(s ? s : "(null)"); break; }
        case 'c': tty_putc((char)va_arg(ap, int)); break;
        case '%': tty_putc('%'); break;
        default:  tty_putc('%'); tty_putc(*fmt); break;
        }
        fmt++;
    }
    va_end(ap);
}
