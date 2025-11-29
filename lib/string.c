#include <stddef.h>  
#include <stdbool.h>


int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}


char *strcpy(char *dest, const char *src) {
    char *dst = dest;
    while ((*dst++ = *src++)) {
        
    }
    return dest;
}


size_t strlen(const char *str) {
    size_t len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

void itoa(int num, char* str) {
    int i = 0;
    bool isNegative = false;

    
    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    
    do {
        str[i++] = (num % 10) + '0'; 
        num = num / 10;  
    } while (num != 0);

    
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';  

    
    int start = 0;
    int end = i - 1;
    while (start < end) {
        
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}