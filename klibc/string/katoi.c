#include "string.h"

int katoi(const char *str) {
    if (!str) {
        return 0;
    }

    while (*str == ' ' || (*str >= '\t' && *str <= '\r')) {
        str++;
    }

    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    long result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return (int)(sign * result);
}
