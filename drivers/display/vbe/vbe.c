# include "vbe.h"
# include "font_default.c"
# include "../display.h"
#define VBE_CHAR_WIDTH 8u
#define VBE_CHAR_HEIGHT 8u

static inline void vbe_putpixel(display_t *display, uint32_t x, uint32_t y, uint32_t color) {
    if (x >= display->data.width || y >= display->data.height) return;

    uint8_t *fb = display->data.framebuffer;
    uint32_t off = y * display->data.pitch + x * (display->data.bpp / 8);

    if (display->data.bpp == 32) {
        *(uint32_t *)(fb + off) = color;
    } else if (display->data.bpp == 24) {
        fb[off + 0] = (uint8_t)(color & 0xFF);
        fb[off + 1] = (uint8_t)((color >> 8) & 0xFF);
        fb[off + 2] = (uint8_t)((color >> 16) & 0xFF);
    }
}
static inline void vbe_putentryat(display_t *display, uint32_t fg_color, uint32_t bg_color, char c, uint32_t x, uint32_t y) {

    uint32_t px = x * VBE_CHAR_WIDTH;   // cell -> pixel
    uint32_t py = y * VBE_CHAR_HEIGHT;   // cell -> pixel

    const uint8_t *glyph = fontdata_8x8 + ((uint8_t)c * VBE_CHAR_HEIGHT);



    for (uint32_t cy = 0; cy < VBE_CHAR_HEIGHT; cy++) {
        uint8_t row = glyph[cy];
        for (uint32_t cx = 0; cx < VBE_CHAR_WIDTH; cx++) {
            uint32_t col = (row & (1u << (7 - cx))) ? fg_color : bg_color;
            vbe_putpixel(display, px + cx, py + cy, col);
        }
    }
}

static inline enum display_result vbe_execute_command(display_t *display, struct s_display_command *command) {
    switch (command->cmd) {
        case DISPLAY_CMD_CLEAR:
             // Implement VBE-specific clear logic here
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_SET_COLOR:
            // Implement VBE-specific color setting logic here
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_PUT_PIXEL:
            vbe_putpixel(display, command->put_pixel.x, command->put_pixel.y, command->put_pixel.pixel_color);
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_PUT_CHAR:
            vbe_putentryat(display, command->put_char.fg, command->put_char.bg, command->put_char.c, command->put_char.x, command->put_char.y);
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_PUT_STRING:
            // Implement VBE-specific string drawing logic here
            return DISPLAY_SUCCESS;
        default:
            return DISPLAY_ERROR; // Unknown command
    }
}

void init_vbe(display_t *display, const display_boot_config_t *boot_config) {
	display->type = DRIVER_VBE;
	display->execute_command = &vbe_execute_command;
    display->data.e_color.fg = COLOR_WHITE;
    display->data.e_color.bg = COLOR_BLACK;
    if (boot_config != NULL) {
        display->data.framebuffer = (void *)(uintptr_t)boot_config->framebuffer_addr;
        display->data.width = boot_config->framebuffer_width;
        display->data.height = boot_config->framebuffer_height;
        display->data.pitch = boot_config->framebuffer_pitch;
        display->data.bpp = boot_config->framebuffer_bpp;
        display->data.char_width = boot_config->framebuffer_width / VBE_CHAR_WIDTH;
        display->data.char_height = boot_config->framebuffer_height / VBE_CHAR_HEIGHT;
        display->data.color_info.rgb.framebuffer_blue_field_position = boot_config->color_info.rgb.framebuffer_blue_field_position;
        display->data.color_info.rgb.framebuffer_blue_mask_size = boot_config->color_info.rgb.framebuffer_blue_mask_size;
        display->data.color_info.rgb.framebuffer_green_field_position = boot_config->color_info.rgb.framebuffer_green_field_position;
        display->data.color_info.rgb.framebuffer_green_mask_size = boot_config->color_info.rgb.framebuffer_green_mask_size;
        display->data.color_info.rgb.framebuffer_red_field_position = boot_config->color_info.rgb.framebuffer_red_field_position;
        display->data.color_info.rgb.framebuffer_red_mask_size = boot_config->color_info.rgb.framebuffer_red_mask_size;
    } else {
        // Set default values or handle error
        display->data.framebuffer = NULL;
        display->data.width = 0;
        display->data.height = 0;
        display->data.pitch = 0;
        display->data.bpp = 0;
        display->data.char_width = 0;
        display->data.char_height = 0;
        display->data.color_info.rgb.framebuffer_blue_field_position = 0;
        display->data.color_info.rgb.framebuffer_blue_mask_size = 0;
        display->data.color_info.rgb.framebuffer_green_field_position = 0;
        display->data.color_info.rgb.framebuffer_green_mask_size = 0;
        display->data.color_info.rgb.framebuffer_red_field_position = 0;
        display->data.color_info.rgb.framebuffer_red_mask_size = 0;
    }
}
