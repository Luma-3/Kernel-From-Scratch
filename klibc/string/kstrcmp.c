//
// Created by gcaptari on 2/27/26.
//

#include "string.h"

int kstrcmp(const char *s1, const char *s2) {
    const unsigned char *p1 = (const unsigned char*) s1;
    const unsigned char *p2 = (const unsigned char*) s2;
    unsigned char	u1;
    unsigned char	u2;

    do {
        u1 = (unsigned char)*p1++;
        u2 = (unsigned char)*p2++;
        if (u1 == 0) {
            return u1 - u2;
        }
    } while (u1 == u2);

    return u1 - u2;
}