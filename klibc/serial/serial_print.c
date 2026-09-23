#include "serial/serial.h"

void print_serial(const char *str) {
    while (*str) {
        write_serial(*str++);
    }
}

void panic_serial(const char *str) {
    print_serial("PANIC: ");
    print_serial(str);
    print_serial("\n");
}
