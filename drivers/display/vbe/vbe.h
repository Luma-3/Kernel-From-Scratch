#ifndef __KFS_VBE_H
# define __KFS_VBE_H

# include <stdbool.h>
# include <stdint.h>

typedef struct multiboot_info multiboot_info_t;


enum vbe_result {
    VBE_SUCCESS = 0,
    VBE_ERROR = -1
};

typedef struct s_vbe_info {
    uint32_t framebuffer_addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
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
} vbe_info_t;


void vbe_init(multiboot_info_t *mbi);
bool vbe_detect(multiboot_info_t *mbi);

enum vbe_result vbe_putpixel(uint32_t x, uint32_t y, uint32_t color);

enum vbe_result vbe_putbitmap(uint32_t x, uint32_t y, const uint32_t *bitmap, uint32_t width, uint32_t height);

enum vbe_result vbe_putbitmap_scaled(uint32_t x, uint32_t y, const uint32_t *bitmap, uint32_t width, uint32_t height, uint32_t scale);

enum vbe_result vbe_clear(uint32_t color);



#endif