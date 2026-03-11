#include "vbe.h"
#include "multiboot.h"

struct s_vbe_info vbe_info = {0};

inline enum vbe_result vbe_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= vbe_info.width || y >= vbe_info.height)
        return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off = y * vbe_info.pitch + x * (vbe_info.bpp / 8);

    if (vbe_info.bpp == 32) {
        *(uint32_t *)(fb + off) = color;
    } else if (vbe_info.bpp == 24) {
        fb[off + 0] = (uint8_t)(color & 0xFF);
        fb[off + 1] = (uint8_t)((color >> 8) & 0xFF);
        fb[off + 2] = (uint8_t)((color >> 16) & 0xFF);
    } else {
        return VBE_ERROR;
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_putbitmap(uint32_t x, uint32_t y, const uint32_t *bitmap,
                              uint32_t width, uint32_t height) {
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t color = bitmap[j * width + i];
            if (vbe_putpixel(x + i, y + j, color) != VBE_SUCCESS) {
                return VBE_ERROR;
            }
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_putbitmap_scaled(uint32_t x, uint32_t y,
                                     const uint32_t *bitmap, uint32_t width,
                                     uint32_t height, uint32_t scale) {
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t color = bitmap[j * width + i];
            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    if (vbe_putpixel(x + i * scale + sx, y + j * scale + sy,
                                     color) != VBE_SUCCESS) {
                        return VBE_ERROR;
                    }
                }
            }
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_clear(uint32_t color) {
    for (uint32_t y = 0; y < vbe_info.height; y++) {
        for (uint32_t x = 0; x < vbe_info.width; x++) {
            if (vbe_putpixel(x, y, color) != VBE_SUCCESS) {
                return VBE_ERROR;
            }
        }
    }
    return VBE_SUCCESS;
}

bool vbe_detect(multiboot_info_t *mbi) {
    return (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) != 0u &&
           mbi->framebuffer_addr != 0u && mbi->framebuffer_type == 1u &&
           (mbi->framebuffer_bpp == 24u || mbi->framebuffer_bpp == 32u);
}

void vbe_init(multiboot_info_t *mbi) {

    vbe_info.framebuffer_addr = (uint32_t)mbi->framebuffer_addr;
    vbe_info.width = mbi->framebuffer_width;
    vbe_info.height = mbi->framebuffer_height;
    vbe_info.pitch = mbi->framebuffer_pitch;
    vbe_info.bpp = mbi->framebuffer_bpp;

    if (mbi->framebuffer_type == 1u) {
        vbe_info.color_info.rgb.framebuffer_red_field_position =
            mbi->color_info.rgb.framebuffer_red_field_position;
        vbe_info.color_info.rgb.framebuffer_red_mask_size =
            mbi->color_info.rgb.framebuffer_red_mask_size;
        vbe_info.color_info.rgb.framebuffer_green_field_position =
            mbi->color_info.rgb.framebuffer_green_field_position;
        vbe_info.color_info.rgb.framebuffer_green_mask_size =
            mbi->color_info.rgb.framebuffer_green_mask_size;
        vbe_info.color_info.rgb.framebuffer_blue_field_position =
            mbi->color_info.rgb.framebuffer_blue_field_position;
        vbe_info.color_info.rgb.framebuffer_blue_mask_size =
            mbi->color_info.rgb.framebuffer_blue_mask_size;
    } else if (mbi->framebuffer_type == 0u) {
        vbe_info.color_info.palette.framebuffer_palette_addr =
            mbi->color_info.palette.framebuffer_palette_addr;
        vbe_info.color_info.palette.framebuffer_palette_num_colors =
            mbi->color_info.palette.framebuffer_palette_num_colors;
    }
}
