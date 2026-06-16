#include "string.h"
#include "terminal.h"

int putchar(char c) {
    return (term_write(active_terminal, (const uint8_t *)&c, 1));
}

int putstr(const char *str) {
    if (!str)
        return (term_write(active_terminal, (const uint8_t *)"(null)", 6));
    return (term_write(active_terminal, (const uint8_t *)str, kstrlen(str)));
}

int utoa(unsigned int value, char *str) {
    if (!str)
        return (0);
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    unsigned int tmp_value;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return 1;
    }

    while (value) {
        tmp_value = value % 10;
        value /= 10;
        *ptr++ = '0' + tmp_value;
    }
    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return (ptr - str + 1);
}

int putnbr(int value) {
    char buffer[12];
    kitoa(value, buffer);
    return putstr(buffer);
}

int putnbr_unsigned(unsigned int value) {
    char buffer[11];
    utoa(value, buffer);
    return putstr(buffer);
}

int lutohex(unsigned long value, char *str, int uppercase) {
    if (!str)
        return (0);
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    unsigned long tmp_value;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return 1;
    }

    while (value) {
        tmp_value = value % 16;
        value /= 16;
        if (tmp_value < 10)
            *ptr++ = '0' + tmp_value;
        else
            *ptr++ = (uppercase ? 'A' : 'a') + (tmp_value - 10);
    }
    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return (ptr - str + 1);
}

int puthex(unsigned long value, int uppercase) {
    char buffer[17];
    lutohex(value, buffer, uppercase);
    return putstr(buffer);
}

int putpointer(void *ptr) {
    if (!ptr)
        return putstr("(nil)");
    char buffer[19] = "0x";
    lutohex((unsigned long)ptr, buffer + 2, 0);
    return putstr(buffer);
}
