#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t flags_limit;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit; // GDT Size in byte - 1
    uint32_t base;  // GDT ptr (0x800)
} __attribute__((packed));

/**
 * @brief Load the GDT into the GDTR register.
 *
 * @param gdt_ptr The pointer to the GDT descriptor.
 */
extern void gdt_flush(uint32_t gdt_ptr);

void init_gdt();

#define GDT_ENTRY_BASE 0
#define GDT_ENTRY_LIMIT 0xFFFFF

#define GDT_ENTRY_FLAGS 0xC0

/**
 * @brief GDT Entry Access Byte
 * @param P Present (1 = present, 0 = not present)
 * @param DPL Descriptor Privilege Level (0-3)
 * @param S Descriptor Type (0 = system, 1 = code/data)
 * @param E Executable (1 = code, 0 = data)
 * @param DC Direction/Conforming (1 = conforming, 0 = non-conforming)
 * @param RW Readable/Writable (1 = readable/writable, 0 = not
 * readable/writable)
 * @param A Accessed (1 = accessed, 0 = not accessed)
 */
#define GDT_ENTRY_ACCESS(P, DPL, S, E, DC, RW, A)                              \
    (((P) << 7) | ((DPL) << 5) | ((S) << 4) | ((E) << 3) | ((DC) << 2) |       \
     ((RW) << 1) | (A))

#define GDT_ENTRY_ACCESS_KERNEL_CODE GDT_ENTRY_ACCESS(1, 0, 1, 1, 0, 1, 0)

#define GDT_ENTRY_ACCESS_KERNEL_DATA GDT_ENTRY_ACCESS(1, 0, 1, 0, 0, 1, 0)

#define GDT_ENTRY_ACCESS_KERNEL_STACK GDT_ENTRY_ACCESS(1, 0, 1, 0, 0, 1, 0)

#define GDT_ENTRY_ACCESS_USER_CODE GDT_ENTRY_ACCESS(1, 3, 1, 1, 0, 1, 0)

#define GDT_ENTRY_ACCESS_USER_DATA GDT_ENTRY_ACCESS(1, 3, 1, 0, 0, 1, 0)

#define GDT_ENTRY_ACCESS_USER_STACK GDT_ENTRY_ACCESS(1, 3, 1, 0, 0, 1, 0)

// Note: not use for now, but we can use it for TSS in the future
#define GDT_ENTRY_ACCESS_TSS GDT_ENTRY_ACCESS(1, 0, 0, 1, 0, 0, 0)

#endif // !GDT_H
