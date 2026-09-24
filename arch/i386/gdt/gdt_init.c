#include "gdt.h"
#include <stdint.h>

struct gdt_entry *gdt = (struct gdt_entry *)0x00000800;

static void encode_gdt_entry(uint8_t entry, uint32_t base, uint32_t limit,
                             uint8_t access, uint8_t flags) {
    // Encode Base
    gdt[entry].base_low = (base & 0xFF);
    gdt[entry].base_middle = (base >> 16) & 0xFF;
    gdt[entry].base_high = (base >> 24) & 0xFF;

    // Encoce Limit
    gdt[entry].limit_low = (limit & 0xFF);
    gdt[entry].flags_limit = (limit >> 16) & 0x0F;

    // flag
    gdt[entry].flags_limit |= (flags & 0xF0);

    // access
    gdt[entry].access_byte = access;
}

void init_gdt() {
    encode_gdt_entry(0, GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, 0x00, 0);
    encode_gdt_entry(1, GDT_ENTRY_BASE, GDT_ENTRY_LIMIT,
                     GDT_ENTRY_ACCESS_KERNEL_CODE, GDT_ENTRY_FLAGS);
    encode_gdt_entry(2, GDT_ENTRY_BASE, GDT_ENTRY_LIMIT,
                     GDT_ENTRY_ACCESS_KERNEL_DATA, GDT_ENTRY_FLAGS);
    encode_gdt_entry(3, GDT_ENTRY_BASE, GDT_ENTRY_LIMIT,
                     GDT_ENTRY_ACCESS_KERNEL_STACK, GDT_ENTRY_FLAGS);
    encode_gdt_entry(4, GDT_ENTRY_BASE, GDT_ENTRY_LIMIT,
                     GDT_ENTRY_ACCESS_USER_CODE, GDT_ENTRY_FLAGS);
    encode_gdt_entry(5, GDT_ENTRY_BASE, GDT_ENTRY_LIMIT,
                     GDT_ENTRY_ACCESS_USER_DATA, GDT_ENTRY_FLAGS);
    encode_gdt_entry(6, GDT_ENTRY_BASE, GDT_ENTRY_LIMIT,
                     GDT_ENTRY_ACCESS_USER_STACK, GDT_ENTRY_FLAGS);

    struct gdt_ptr gdt_descriptor;

    // (Size of GDT entry * nb of entry) - 1
    gdt_descriptor.limit = (sizeof(struct gdt_entry) * 7) - 1;
    gdt_descriptor.base = (uint32_t)gdt;

    gdt_flush((uint32_t)&gdt_descriptor);
}
