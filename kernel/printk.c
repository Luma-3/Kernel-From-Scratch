#include "printk.h"
#include "print_utility.h"
#include <stdarg.h>

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
int printk(enum kernel_log_level log_level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int written = 0;
    switch (log_level) {
    case KERNEL_LOG_LEVEL_DEBUG:
        written += putstr("[\033[34mDEBUG\033[0m] ");
        break;
    case KERNEL_LOG_LEVEL_INFO:
        written += putstr("[\033[32mINFO\033[0m] ");
        break;
    case KERNEL_LOG_LEVEL_WARN:
        written += putstr("[\033[33mWARN\033[0m] ");
        break;
    case KERNEL_LOG_LEVEL_ERROR:
        written += putstr("[\033[31mERROR\033[0m] ");
        break;
    }
    while (*format) {
        if (*format == '%') {
            format++;
            written += parse_print(format, &args);
        } else {
            written += putchar(*format);
        }
        format++;
    }
    va_end(args);
    return written;
}
