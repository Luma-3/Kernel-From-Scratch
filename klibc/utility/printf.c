#include "terminal.h"
#include <stdarg.h>
#include "print_utility.h"

static int parse_print(const char *format, va_list *args, int *jump) {
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
        case 'l': {
            format++;
            if (*format == 'd' || *format == 'i') {
                *jump = 2;
                return putnbr_long(va_arg(*args, long));
            } else if (*format == 'u') {
                *jump = 2;
                    format++;
                return putnbr_unsigned_long(va_arg(*args, unsigned long));
            } else if (*format == 'x') {
                *jump = 2;
                    format++;
                return puthex(va_arg(*args, unsigned long), 0);
            } else if (*format == 'X') {
                *jump = 2;
                    format++;
                return puthex(va_arg(*args, unsigned long), 1);
            } else if (*format == 'l') {
                format++;
                if (*format == 'd' || *format == 'i') {
                    *jump = 3;
                    format++;
                    return putnbr_long_long(va_arg(*args, long long));
                } else if (*format == 'u') {
                    *jump = 3;
                    format++;
                    return putnbr_unsigned_long_long(va_arg(*args, unsigned long long));
                } else if (*format == 'x') {
                    *jump = 3;
                    format++;
                    return puthex_long_long(va_arg(*args, unsigned long long), 0);
                } else if (*format == 'X') {
                    *jump = 3;
                    format++;
                    return puthex_long_long(va_arg(*args, unsigned long long), 1);
                }else {
                    return (0);
                }
            }else {
                return (0);
            }
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
            int jump_aditional = 0;
            tmp = parse_print(++format, &args, &jump_aditional);
            if (tmp == -1)
                return (-1);
            len += tmp;
            if (jump_aditional > 0) {
                format += jump_aditional - 1;
            }
        } else
            len += putchar(*format);
        format++;
    }
    va_end(args);
    return len;
}