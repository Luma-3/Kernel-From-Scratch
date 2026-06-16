#include "kernel.h"
#include "terminal.h"
#include <stdarg.h>
#include "print_utility.h"

static int parse_print(const char *format, va_list *args) {
    switch (*format) {
        case 'c':
            return putchar(va_arg(*args, int));
        case 's':
            return putstr(va_arg(*args, const char *));
        case 'd':
        case 'i': {
            return putnbr(va_arg(*args, int));
        }
        case 'u': {
            return putnbr_unsigned(va_arg(*args, unsigned int));
        }
        case 'x': {
            return puthex(va_arg(*args, unsigned long), 0);
        }
        case 'X': {
            return puthex(va_arg(*args, unsigned long), 1);
        }
        case 'p': {
            void *ptr = va_arg(*args, void *);
            return putpointer(ptr);
        }
        case '%':
            return putchar('%');
        default:
            return (0);
    }
}

int printf(const char *format, ...) {
    va_list args;
    int len;
    int tmp;

    if (!format)
        return (-1);
    va_start(args, format);
    while (*format) {
        if (*format == '%') {
            tmp = parse_print(++format, &args);
            if (tmp == -1)
                return (-1);
            len += tmp;
        } else
            len += putchar(*format);
        format++;
    }
    va_end(args);
    return len;
}