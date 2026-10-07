//
// Created by gcaptari on 2/27/26.
//

#include "mem.h"

void *kmemsetw(void *s, uint16_t c, size_t n) {
    uint16_t *p = (uint16_t*) s;

    while (n--) {
        *p++ = c;
    }

    return s;
}