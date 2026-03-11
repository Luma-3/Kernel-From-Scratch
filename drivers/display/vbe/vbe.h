#ifndef __KFS_VBE_H
# define __KFS_VBE_H

# include <stdbool.h>
# include <stdint.h>

typedef struct multiboot_info multiboot_info_t;


enum vbe_result {
    VBE_SUCCESS = 0,
    VBE_ERROR = -1
};

typedef union {
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
} vbe_color_info_t;

typedef struct s_vbe_info {
    uintptr_t framebuffer_addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    vbe_color_info_t color_info;
} vbe_info_t;


uintptr_t vbe_get_framebuffer_addr();

uint32_t vbe_get_width();

uint32_t vbe_get_height();

uint32_t vbe_get_pitch();

uint32_t vbe_get_bpp();

vbe_color_info_t *vbe_get_color_info();

enum vbe_result vbe_get_pixel(uint32_t x, uint32_t y, uint32_t *color);

enum vbe_result vbe_putpixel(uint32_t x, uint32_t y, uint32_t color);

enum vbe_result vbe_draw_bitmap(uint32_t x, uint32_t y, const uint32_t *bitmap, uint32_t width, uint32_t height);

enum vbe_result vbe_draw_bitmap_scaled(uint32_t x, uint32_t y, const uint32_t *bitmap, uint32_t width, uint32_t height, uint32_t scale);

enum vbe_result vbe_draw_glyph(uint32_t x, uint32_t y, uint8_t *glyph, uint32_t glyph_width, uint32_t glyph_height, uint32_t fg_color, uint32_t bg_color);

enum vbe_result vbe_draw_glyph_scaled(uint32_t x, uint32_t y, uint8_t *glyph, uint32_t glyph_width, uint32_t glyph_height, uint32_t fg_color, uint32_t bg_color, uint32_t scale);

enum vbe_result vbe_draw_glyph_sized(uint32_t x, uint32_t y, uint8_t *glyph, uint32_t glyph_width, uint32_t glyph_height, uint32_t fg_color, uint32_t bg_color, uint32_t new_width, uint32_t new_height);

enum vbe_result vbe_clear(uint32_t color);

enum vbe_result vbe_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

void vbe_init(multiboot_info_t *mbi);

bool vbe_detect(multiboot_info_t *mbi);

#endif