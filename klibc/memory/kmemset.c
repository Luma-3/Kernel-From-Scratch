//
// Created by gcaptari on 2/27/26.
//

#include "mem.h"

void *kmemset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char*) s;
    c = (unsigned char)c;

    while (n--) {
        *p++ = c;
    }

    return s;
}