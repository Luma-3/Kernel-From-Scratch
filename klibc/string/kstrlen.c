//
// Created by gcaptari on 2/27/26.
//

#include "string.h"

size_t kstrlen(const char *str) {
    const char *s = str;

    while (*s) {
        ++s;
    }

    return s - str;
}