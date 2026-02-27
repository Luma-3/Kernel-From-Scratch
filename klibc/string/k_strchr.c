//
// Created by gcaptari on 2/27/26.
//

#include "string.h"

const char *k_strchr(const char *str, int c) {
    const char *move = str;
    c = (unsigned char)c;

    while (*move) {
        if (*move == c) {
            return move;
        }
        ++move;
    }

    return NULL;
}