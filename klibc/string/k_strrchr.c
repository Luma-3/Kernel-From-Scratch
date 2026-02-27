//
// Created by gcaptari on 2/27/26.
//

#include "string.h"

const char *k_strrchr(const char *str, int c) {
    const char *move = str;
    const char *last_occurrence = NULL;
    c = (unsigned char)c;

    while (*move) {
        if (*move == c) {
            last_occurrence = move;
        }
        ++move;
    }

    return last_occurrence;
}