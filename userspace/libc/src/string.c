#include "../include/string.h"

size_t strlen(const char *s) { size_t n=0; while(s[n])n++; return n; }
int strcmp(const char *a, const char *b) {
    while(*a && *a==*b){a++;b++;} return (unsigned char)*a-(unsigned char)*b;
}
int strncmp(const char *a, const char *b, size_t n) {
    while(n--&&*a&&*a==*b){a++;b++;}
    return n==(size_t)-1?0:(unsigned char)*a-(unsigned char)*b;
}
char *strcpy(char *d, const char *s) { char *r=d; while((*d++=*s++)); return r; }
char *strncpy(char *d, const char *s, size_t n) {
    char *r=d; while(n--&&(*d++=*s++)); while(n--)*d++=0; return r;
}
void *memset(void *p, int v, size_t n) {
    unsigned char *b=p; while(n--)*b++=(unsigned char)v; return p;
}
void *memcpy(void *d, const void *s, size_t n) {
    unsigned char *dd=d; const unsigned char *ss=s; while(n--)*dd++=*ss++; return d;
}
