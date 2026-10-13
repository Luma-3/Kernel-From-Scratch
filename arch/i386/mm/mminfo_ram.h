#ifndef KFS_MMINFO_RAM_H
#define KFS_MMINFO_RAM_H

#include <stdint.h>

typedef struct multiboot_info multiboot_info_t;

typedef struct memory_detection_info {
    uint64_t total_usable_ram;
    uint64_t max_physical_addr;
} memory_detection_info_t;

memory_detection_info_t detect_memory(multiboot_info_t *mbi);
#endif /* KFS_MMINFO_RAM_H */