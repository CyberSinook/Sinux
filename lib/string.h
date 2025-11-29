#ifndef STRING_H
#define STRING_H

#include <stddef.h>  
#include <stdbool.h>


int strcmp(const char *str1, const char *str2);


char *strcpy(char *dest, const char *src);


size_t strlen(const char *str);

void itoa(int num, char* str);

#endif 