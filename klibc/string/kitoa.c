#include "limits.h"
#include "mem.h"
#define INT_MIN_STR "-2147483648"
#define INT_MAX_STR "2147483647"

int kitoa(int value, char *str) {
    if(!str) {
        return 0;
    }
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

    if(value == INT_MIN) {
        kmemcpy(str, INT_MIN_STR, sizeof(INT_MIN_STR));
        return 11;
    }

    if(value == INT_MAX) {
        kmemcpy(str, INT_MAX_STR, sizeof(INT_MAX_STR));
        return 10;
    }

    if (value < 0) {
        value = -value;
        *ptr++ = '-';
        ptr1++;
    }

    do {
        tmp_value = value;
        value /= 10;
        *ptr++ = '0' + (tmp_value - value * 10);
    } while (value);

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return ptr - str + 1;
}