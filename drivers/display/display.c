#include "display.h"
#include "vbe/vbe.h"
#include "vga/vga.h"
#include "font/font.h"
#include "klibc/debug/kdebug.h"

static enum display_mode active_mode = DISPLAY_MODE_NONE;

static const uint32_t palette_rgb[16] = {
    0x000000, // 0: Black
    0xFF0000, // 1: Red
    0x00FF00, // 2: Green
    0xFFFF00, // 3: Yellow
    0x0000FF, // 4: Blue
    0xFF00FF, // 5: Magenta
    0x00FFFF, // 6: Cyan
    0x808080, // 7: Grey
    0x404040, // 8: Dark Grey
    0xFF5555, // 9: Bright Red
    0x55FF55, // 10: Bright Green
    0xFFFF55, // 11: Bright Yellow
    0x5555FF, // 12: Bright Blue
    0xFF55FF, // 13: Bright Magenta
    0x55FFFF, // 14: Bright Cyan
    0xFFFFFF  // 15: White
};

static const uint8_t ansi_to_vga_palette[8] = {
    0, // 0: Black
    4, // 1: Red
    2, // 2: Green
    6, // 3: Yellow (Brown)
    1, // 4: Blue
    5, // 5: Magenta
    3, // 6: Cyan
    7  // 7: Grey
};

void display_init(multiboot_info_t *mbi) {
    // 1. Toujours initialiser le VGA en premier pour garantir un affichage immediat
    vga_init();
    active_mode = DISPLAY_MODE_VGA;

    // 2. Tenter de detecter et initialiser le VBE
    if (mbi && vbe_detect(mbi)) {
        vbe_init(mbi);
        if (vbe_get_framebuffer_addr() != 0) {
            active_mode = DISPLAY_MODE_VBE;
        }
    }
}

enum display_mode display_get_mode(void) {
    return active_mode;
}

uint32_t display_get_cols(void) {
    return 80;
}

uint32_t display_get_rows(void) {
    if (active_mode == DISPLAY_MODE_VBE) {
        return 45; // 720 / 16
    }
    return 25; // VGA standard
}

uint32_t display_get_width_px(void) {
    if (active_mode == DISPLAY_MODE_VBE) {
        return vbe_get_width();
    }
    return 0;
}

uint32_t display_get_height_px(void) {
    if (active_mode == DISPLAY_MODE_VBE) {
        return vbe_get_height();
    }
    return 0;
}


void display_clear(uint32_t bg_rgb) {
    if (active_mode == DISPLAY_MODE_VBE) {
        vbe_clear(bg_rgb);
    } else {
        vga_clear();
    }
}

void display_putchar(char c) {
    if (active_mode == DISPLAY_MODE_VBE) {
        print_serial("[DISPLAY] VBE: ");
        print_serial("Drawing character: ");
        print_serial(&c);
        print_serial("\n");
    } else {
        vga_putchar(c);
    }
}

void display_puts(const char *str) {
    if (active_mode == DISPLAY_MODE_VGA) {
        vga_puts(str);
    }
}

void display_draw_cell_universal_color(uint32_t col, uint32_t row, char c, uint8_t fg, uint8_t bg) {
    if (active_mode == DISPLAY_MODE_VBE) {
        uint32_t fg_rgb = palette_rgb[fg & 0x0F];
        uint32_t bg_rgb = palette_rgb[bg & 0x07];
        uint8_t *glyph = (uint8_t *)fontdata_8x8 + ((uint8_t)c * 8);

        vbe_draw_glyph_sized(col * 16u, row * 16u, glyph, 8, 8, fg_rgb, bg_rgb, 16u, 16u);
    } else {
        if (col < 80 && row < 25) {
            volatile uint16_t *vga = (volatile uint16_t *)VGA_MEMORY;
            uint8_t vga_fg = (fg < 8) ? ansi_to_vga_palette[fg] : (fg & 0x0F);
            uint8_t vga_bg = (bg < 8) ? ansi_to_vga_palette[bg] : (bg & 0x07);
            uint8_t attr = (vga_bg << 4) | vga_fg;
            vga[row * 80 + col] = ((uint16_t)attr << 8) | (uint8_t)c;
        }
    }
}

void display_draw_cell(uint32_t col, uint32_t row, char c, uint32_t fg, uint32_t bg) {
    if (active_mode == DISPLAY_MODE_VBE) {
        uint32_t fg_rgb = (fg < 16) ? palette_rgb[fg & 0x0F] : fg;
        uint32_t bg_rgb = (bg < 16) ? palette_rgb[bg & 0x07] : bg;
        uint8_t *glyph = (uint8_t *)fontdata_8x8 + ((uint8_t)c * 8);

        vbe_draw_glyph_sized(col * 16u, row * 16u, glyph, 8, 8, fg_rgb, bg_rgb, 16u, 16u);
    } else {
        if (col < 80 && row < 25) {
            volatile uint16_t *vga = (volatile uint16_t *)VGA_MEMORY;
            uint8_t vga_fg = (fg < 8) ? ansi_to_vga_palette[fg] : (fg & 0x0F);
            uint8_t vga_bg = (bg < 8) ? ansi_to_vga_palette[bg] : (bg & 0x07);
            uint8_t attr = (vga_bg << 4) | vga_fg;
            vga[row * 80 + col] = ((uint16_t)attr << 8) | (uint8_t)c;
        }
    }
}

void display_set_cursor(uint32_t col, uint32_t row, uint32_t color) {
    if (active_mode == DISPLAY_MODE_VBE) {
        vbe_fill_rect(col * 16u, (row * 16u) + 12u, 16u, 4u, color);
    } else {
        vga_set_cursor((uint8_t)col, (uint8_t)row);
    }
}

void display_set_cursor_universal_color(uint32_t col, uint32_t row, uint8_t color) {
    if (active_mode == DISPLAY_MODE_VBE) {
        uint32_t rgb_color = palette_rgb[color & 0x0F];
        vbe_fill_rect(col * 16u, (row * 16u) + 12u, 16u, 4u, rgb_color);
    } else {
        vga_set_cursor((uint8_t)col, (uint8_t)row);
    }
}


void display_reveal_cursor(uint32_t col, uint32_t row, uint32_t color) {
    if (active_mode == DISPLAY_MODE_VBE) {
        vbe_fill_rect(col * 16u, (row * 16u) + 12u, 16u, 4u, color);
    } else {
        vga_enable_cursor(14, 15);
    }
}

void display_hide_cursor(uint32_t col, uint32_t row, uint32_t color) {
    if (active_mode == DISPLAY_MODE_VBE) {
        vbe_fill_rect(col * 16u, (row * 16u) + 12u, 16u, 4u, color);
    } else {
        vga_disable_cursor();
    }
}

void display_reveal_cursor_universal_color(uint32_t col, uint32_t row, uint8_t color) {
    if (active_mode == DISPLAY_MODE_VBE) {
        uint32_t rgb_color = palette_rgb[color & 0x0F];
        vbe_fill_rect(col * 16u, (row * 16u) + 12u, 16u, 4u, rgb_color);
    } else {
        vga_enable_cursor(14, 15);
    }
}

void display_hide_cursor_universal_color(uint32_t col, uint32_t row, uint8_t color) {
    if (active_mode == DISPLAY_MODE_VBE) {
        uint32_t rgb_color = palette_rgb[color & 0x0F];
        vbe_fill_rect(col * 16u, (row * 16u) + 12u, 16u, 4u, rgb_color);
    } else {
        vga_disable_cursor();
    }
}

void display_log_step(const char *step_name, enum display_step_status status) {
    enum vga_step_status vga_st = VGA_STEP_OK;
    switch (status) {
    case DISPLAY_STEP_OK:   vga_st = VGA_STEP_OK;   break;
    case DISPLAY_STEP_FAIL: vga_st = VGA_STEP_FAIL; break;
    case DISPLAY_STEP_INFO: vga_st = VGA_STEP_INFO; break;
    case DISPLAY_STEP_WARN: vga_st = VGA_STEP_WARN; break;
    }
    vga_log_step(step_name, vga_st);
}
