#include "arch/i386/mm/mminfo_ram.h"
#include "boot/multiboot.h"
memory_detection_info_t detect_memory(multiboot_info_t *mbi) {
    memory_detection_info_t info = {0, 0};

    if (!(mbi->flags & MULTIBOOT_INFO_MMAP)) {
        return info; // Panic : Carte mémoire absente
    }

    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *)(uintptr_t)mbi->mmap_addr;
    uintptr_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    while ((uintptr_t)mmap < mmap_end) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            info.total_usable_ram += mmap->len;
            if (mmap->addr + mmap->len > info.max_physical_addr) {
                info.max_physical_addr = mmap->addr + mmap->len;
            }
        }
        mmap = (multiboot_mmap_entry_t *)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }

    return info;
}