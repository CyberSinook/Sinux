#pragma once
#include "types.h"

size_t strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strcpy(char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);
void  *memset(void *p, int v, size_t n);
void  *memcpy(void *dst, const void *src, size_t n);
