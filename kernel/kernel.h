#ifndef KFS_KERNEL_H
#define KFS_KERNEL_H

#include "math.h"
#include "mem.h"
#include "string.h"
#define SUCCESS 0
#define FAILLURE -1

struct s_kchecker {
    uint8_t serial : 1;
    uint8_t magic : 1;
    uint8_t elf : 1;
    uint8_t info_mem : 1;
    uint8_t vbe : 1;
};

#endif // KFS_KERNEL_H
