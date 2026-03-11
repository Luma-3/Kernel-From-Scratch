# include "vbe.h"
# include "multiboot.h"
# include "mem.h"
static struct s_vbe_info vbe_info = {0};

uintptr_t vbe_get_framebuffer_addr() {
    return (uintptr_t)vbe_info.framebuffer_addr;
}

inline uint32_t vbe_get_width() {
    return vbe_info.width;
}

inline uint32_t vbe_get_height() {
    return vbe_info.height;
}

inline uint32_t vbe_get_pitch() {
    return vbe_info.pitch;
}

inline uint32_t vbe_get_bpp() {
    return vbe_info.bpp;
}

inline vbe_color_info_t *vbe_get_color_info() {
    return &vbe_info.color_info;
}

inline enum vbe_result vbe_get_pixel(uint32_t x, uint32_t y, uint32_t *color) {
    if(vbe_info.bpp != 24 && vbe_info.bpp != 32) return VBE_ERROR;

    if (x >= vbe_info.width || y >= vbe_info.height) return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off = y * vbe_info.pitch + x * (vbe_info.bpp / 8);

    switch (vbe_info.bpp) {
        case 32:
            *color = *(uint32_t *)(fb + off);
            return VBE_SUCCESS;
        case 24:
            *color = fb[off] | (fb[off + 1] << 8) | (fb[off + 2] << 16);
            return VBE_SUCCESS;
        default:
            return VBE_ERROR;
    }
}

static inline __attribute__((always_inline)) void vbe_write(uint8_t *addr, uint32_t bpp, uint32_t value) {
    switch (bpp) {
        case 32:
            *(uint32_t *)addr = value;
            break;
        case 24:
            addr[0] = (uint8_t)(value & 0xFF);
            addr[1] = (uint8_t)((value >> 8) & 0xFF);
            addr[2] = (uint8_t)((value >> 16) & 0xFF);
            break;
    }
}

inline enum vbe_result vbe_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if(vbe_info.bpp != 24 && vbe_info.bpp != 32) return VBE_ERROR;

    if (x >= vbe_info.width || y >= vbe_info.height) return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off = y * vbe_info.pitch + x * (vbe_info.bpp / 8);

    vbe_write(fb + off, vbe_info.bpp, color);
    return VBE_SUCCESS;
}

enum vbe_result vbe_draw_bitmap(uint32_t x, uint32_t y, const uint32_t *bitmap, uint32_t width, uint32_t height) {
    if(vbe_info.bpp != 24 && vbe_info.bpp != 32) return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    uint32_t pixel_x;
    uint32_t pixel_y;
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t color = bitmap[j * width + i];
            pixel_x = x + i;
            pixel_y = y + j;
            if (pixel_x >= vbe_info.width || pixel_y >= vbe_info.height) return VBE_ERROR;
            off = pixel_y * vbe_info.pitch + pixel_x * (vbe_info.bpp / 8);
            vbe_write(fb + off, vbe_info.bpp, color);
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_draw_bitmap_scaled(uint32_t x, uint32_t y, const uint32_t *bitmap, uint32_t width, uint32_t height, uint32_t scale) {
    if(vbe_info.bpp != 24 && vbe_info.bpp != 32) return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    uint32_t pixel_x;
    uint32_t pixel_y;

    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t color = bitmap[j * width + i];
            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    pixel_x = x + i * scale + sx;
                    pixel_y = y + j * scale + sy;
                    if (pixel_x >= vbe_info.width || pixel_y >= vbe_info.height) return VBE_ERROR;
                    off = pixel_y * vbe_info.pitch + pixel_x * (vbe_info.bpp / 8);
                    vbe_write(fb + off, vbe_info.bpp, color);
                }
            }
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_draw_glyph(uint32_t x, uint32_t y, uint8_t *glyph, uint32_t glyph_width, uint32_t glyph_height,  uint32_t fg_color, uint32_t bg_color) {
    if(vbe_info.bpp != 24 && vbe_info.bpp != 32) return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    uint32_t pixel_x;
    uint32_t pixel_y;

    for (uint32_t j = 0; j < glyph_height; j++) {
        uint8_t row = glyph[j];
        for (uint32_t i = 0; i < glyph_width; i++) {
            pixel_x = x + i;
            pixel_y = y + j;
            if (pixel_x >= vbe_info.width || pixel_y >= vbe_info.height) return VBE_ERROR;
            off = pixel_y * vbe_info.pitch + pixel_x * (vbe_info.bpp / 8);
            if (row & (1 << (7 - i))) {
                vbe_write(fb + off, vbe_info.bpp, fg_color);
            } else {
                vbe_write(fb + off, vbe_info.bpp, bg_color);
            }
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_draw_glyph_scaled(uint32_t x, uint32_t y, uint8_t *glyph, uint32_t glyph_width, uint32_t glyph_height,  uint32_t fg_color, uint32_t bg_color, uint32_t scale) {
    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    uint32_t pixel_x;
    uint32_t pixel_y;

    for (uint32_t j = 0; j < glyph_height; j++) {
        uint8_t row = glyph[j];
        for (uint32_t i = 0; i < glyph_width; i++) {
            if (row & (1 << (7 - i))) {
                for (uint32_t sy = 0; sy < scale; sy++) {
                    for (uint32_t sx = 0; sx < scale; sx++) {
                        pixel_x = x + i * scale + sx;
                        pixel_y = y + j * scale + sy;
                        if (pixel_x >= vbe_info.width || pixel_y >= vbe_info.height) return VBE_ERROR;
                        off = pixel_y * vbe_info.pitch + pixel_x * (vbe_info.bpp / 8);
                        vbe_write(fb + off, vbe_info.bpp, fg_color);
                    }
                }
            } else {
                for (uint32_t sy = 0; sy < scale; sy++) {
                    for (uint32_t sx = 0; sx < scale; sx++) {
                        pixel_x = x + i * scale + sx;
                        pixel_y = y + j * scale + sy;
                        if (pixel_x >= vbe_info.width || pixel_y >= vbe_info.height) return VBE_ERROR;
                        off = pixel_y * vbe_info.pitch + pixel_x * (vbe_info.bpp / 8);
                        vbe_write(fb + off, vbe_info.bpp, bg_color);
                    }
                }
            }
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_draw_glyph_sized(uint32_t x, uint32_t y, uint8_t *glyph, uint32_t glyph_width, uint32_t glyph_height, uint32_t fg_color, uint32_t bg_color, uint32_t new_width, uint32_t new_height) {
    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    uint32_t pixel_x;
    uint32_t pixel_y;

    for (uint32_t j = 0; j < new_height; j++) {
        uint32_t src_j = j * glyph_height / new_height;
        uint8_t row = glyph[src_j];
        for (uint32_t i = 0; i < new_width; i++) {
            uint32_t src_i = i * glyph_width / new_width;
            if (row & (1 << (7 - src_i))) {
                pixel_x = x + i;
                pixel_y = y + j;
                if (pixel_x >= vbe_info.width || pixel_y >= vbe_info.height) return VBE_ERROR;
                off = pixel_y * vbe_info.pitch + pixel_x * (vbe_info.bpp / 8);
                vbe_write(fb + off, vbe_info.bpp, fg_color);
            } else {
                pixel_x = x + i;
                pixel_y = y + j;
                if (pixel_x >= vbe_info.width || pixel_y >= vbe_info.height) return VBE_ERROR;
                off = pixel_y * vbe_info.pitch + pixel_x * (vbe_info.bpp / 8);
                vbe_write(fb + off, vbe_info.bpp, bg_color);
            }
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    if (x >= vbe_info.width || y >= vbe_info.height ||
         x + width > vbe_info.width || y + height > vbe_info.height) return VBE_ERROR;

    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            off = (y + j) * vbe_info.pitch + (x + i) * (vbe_info.bpp / 8);
            vbe_write(fb + off, vbe_info.bpp, color);
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_draw_circle(uint32_t center_x, uint32_t center_y, uint32_t radius, uint32_t color) {
    if(vbe_info.bpp != 24 && vbe_info.bpp != 32) return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y) {
        off = (center_y + y) * vbe_info.pitch + (center_x + x) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);
        off = (center_y + y) * vbe_info.pitch + (center_x - x) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);
        off = (center_y - y) * vbe_info.pitch + (center_x + x) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);
        off = (center_y - y) * vbe_info.pitch + (center_x - x) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);

        off = (center_y + x) * vbe_info.pitch + (center_x + y) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);
        off = (center_y + x) * vbe_info.pitch + (center_x - y) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);
        off = (center_y - x) * vbe_info.pitch + (center_x + y) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);
        off = (center_y - x) * vbe_info.pitch + (center_x - y) * (vbe_info.bpp / 8);
        vbe_write(fb + off, vbe_info.bpp, color);

        y++;
        err += 1 + 2*y;
        if (2*(err-x) + 1 > 0) {
            x--;
            err += 1 - 2*x;
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_draw_circle_filled(uint32_t center_x, uint32_t center_y, uint32_t radius, uint32_t color) {
    if(vbe_info.bpp != 24 && vbe_info.bpp != 32) return VBE_ERROR;

    uint8_t *fb = (uint8_t *)vbe_info.framebuffer_addr;
    uint32_t off;
    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y) {
        for (int32_t i = center_x - x; i <= center_x + x; i++) {
            off = (center_y + y) * vbe_info.pitch + i * (vbe_info.bpp / 8);
            vbe_write(fb + off, vbe_info.bpp, color);
            off = (center_y - y) * vbe_info.pitch + i * (vbe_info.bpp / 8);
            vbe_write(fb + off, vbe_info.bpp, color);
        }
        for (int32_t i = center_x - y; i <= center_x + y; i++) {
            off = (center_y + x) * vbe_info.pitch + i * (vbe_info.bpp / 8);
            vbe_write(fb + off, vbe_info.bpp, color);
            off = (center_y - x) * vbe_info.pitch + i * (vbe_info.bpp / 8);
            vbe_write(fb + off, vbe_info.bpp, color);
        }

        y++;
        err += 1 + 2*y;
        if (2*(err-x) + 1 > 0) {
            x--;
            err += 1 - 2*x;
        }
    }
    return VBE_SUCCESS;
}

enum vbe_result vbe_clear(uint32_t color) {
    return vbe_fill_rect(0, 0, vbe_info.width, vbe_info.height, color);
}

bool vbe_detect(multiboot_info_t *mbi) {
    return (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) != 0u &&
      mbi->framebuffer_addr != 0u &&
      mbi->framebuffer_type == 1u &&
      (mbi->framebuffer_bpp == 24u || mbi->framebuffer_bpp == 32u);
}

void vbe_init(multiboot_info_t *mbi) {
    if(vbe_info.framebuffer_addr != 0u) {
        return;
    }
    
    vbe_info.framebuffer_addr = (uintptr_t)mbi->framebuffer_addr;    
    vbe_info.width = mbi->framebuffer_width;
    vbe_info.height = mbi->framebuffer_height;
    vbe_info.pitch = mbi->framebuffer_pitch;
    vbe_info.bpp = mbi->framebuffer_bpp;

    if (mbi->framebuffer_type == 1u) {
        vbe_info.color_info.rgb.framebuffer_red_field_position = mbi->color_info.rgb.framebuffer_red_field_position;
        vbe_info.color_info.rgb.framebuffer_red_mask_size = mbi->color_info.rgb.framebuffer_red_mask_size;
        vbe_info.color_info.rgb.framebuffer_green_field_position = mbi->color_info.rgb.framebuffer_green_field_position;
        vbe_info.color_info.rgb.framebuffer_green_mask_size = mbi->color_info.rgb.framebuffer_green_mask_size;
        vbe_info.color_info.rgb.framebuffer_blue_field_position = mbi->color_info.rgb.framebuffer_blue_field_position;
        vbe_info.color_info.rgb.framebuffer_blue_mask_size = mbi->color_info.rgb.framebuffer_blue_mask_size;
    } else if (mbi->framebuffer_type == 0u) {
        vbe_info.color_info.palette.framebuffer_palette_addr = mbi->color_info.palette.framebuffer_palette_addr;
        vbe_info.color_info.palette.framebuffer_palette_num_colors = mbi->color_info.palette.framebuffer_palette_num_colors;
    }
}
