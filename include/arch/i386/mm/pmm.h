#ifndef KFS_PMM_H
#define KFS_PMM_H

#include "boot/multiboot.h"
#include "arch/i386/mm/buddy.h"
#include "arch/i386/mm/mminfo_ram.h"

void init_pmm(multiboot_info_t *mbi);

#endif // KFS_PMM_H
