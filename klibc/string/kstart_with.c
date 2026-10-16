//
// Created by gcaptari on 2/27/26.
//

#include "klibc/string/string.h"

bool kstarts_with(const char *str, const char *prefix) {
    while (*prefix && *str) {
        if (*str++ != *prefix++) {
            return false;
        }
    }
    return true;
}