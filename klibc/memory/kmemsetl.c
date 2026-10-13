//
// Created by gcaptari on 2/27/26.
//

#include "mem.h"

void *kmemsetl(void *s, uint32_t c, size_t n) {
    uint32_t *p = (uint32_t*) s;

    while (n--) {
        *p++ = c;
    }

    return s;
}