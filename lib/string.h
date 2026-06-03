#pragma once
#include <stdint.h>
#include <stddef.h>

void   kmemset(void *p, uint8_t v, size_t n);
void   kmemcpy(void *dst, const void *src, size_t n);
size_t kstrlen(const char *s);
int    kstrcmp(const char *a, const char *b);
char  *kstrcpy(char *dst, const char *src);
char  *kstrncpy(char *dst, const char *src, size_t n);
int    kstrncmp(const char *a, const char *b, size_t n);
char  *kstrncat(char *dst, const char *src, size_t n);
