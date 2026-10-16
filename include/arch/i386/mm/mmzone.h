#ifndef KFS_MMZONE_H
#define KFS_MMZONE_H

extern char __kernel_start[];
extern char __kernel_end[];

enum zone_type {
    ZONE_DMA = 0,
    ZONE_NORMAL,
    ZONE_HIGHMEM,
    ZONE_MAX,
};

#endif // KFS_MMZONE_H
