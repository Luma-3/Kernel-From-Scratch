//
// Created by gcaptari on 2/27/26.
//

#include "mem.h"

void *k_memset(void *s, int c, uint32_t n) {
    unsigned char *p = (unsigned char*) s;
    c = (unsigned char)c;

    while (n--) {
        *p++ = c;
    }

    return s;
}