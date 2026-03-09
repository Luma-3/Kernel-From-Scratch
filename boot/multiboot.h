#ifndef __KFS_MULTIBOOT_H
#define __KFS_MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT1_BOOTLOADER_MAGIC 0x2BADB002

#define MULTIBOOT_INFO_MEMORY (1u << 0)
#define MULTIBOOT_INFO_BOOTDEV (1u << 1)
#define MULTIBOOT_INFO_CMDLINE (1u << 2)
#define MULTIBOOT_INFO_MODS (1u << 3)
#define MULTIBOOT_INFO_AOUT_SYMS (1u << 4)
#define MULTIBOOT_INFO_ELF_SHDR (1u << 5)
#define MULTIBOOT_INFO_MMAP (1u << 6)
#define MULTIBOOT_INFO_DRIVE_INFO (1u << 7)
#define MULTIBOOT_INFO_CONFIG_TABLE (1u << 8)
#define MULTIBOOT_INFO_BOOT_LOADER_NAME (1u << 9)
#define MULTIBOOT_INFO_APM_TABLE (1u << 10)
#define MULTIBOOT_INFO_VBE_INFO (1u << 11)
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO (1u << 12)

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;

    union {
        struct {
            uint32_t tabsize;
            uint32_t strsize;
            uint32_t addr;
            uint32_t reserved;
        } aout_sym;
        struct {
            uint32_t num;
            uint32_t size;
            uint32_t addr;
            uint32_t shndx;
        } elf_sec;
    } u;

    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;

    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        } palette;
        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        } rgb;
    } color_info;
} __attribute__((packed)) multiboot_info_t;

typedef struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

#endif
