//
// Created by gcaptari on 2/27/26.
//

#include "string.h"

uint32_t k_strlen(const char *str) {
    const char *s = str;

    while (*s) {
        ++s;
    }

    return s - str;
}