//
// Created by gcaptari on 2/27/26.
//

# include "display.h"
# include "vga/vga.h"
# include "vbe/vbe.h"

struct s_display g_display = {
    .type = DRIVER_NONE,
    .execute_command = NULL
};


void init_display(const enum driver_type type, const struct s_display_boot_config*boot_config) {
    g_display.type = type;

    switch (type) {
        case DRIVER_VGA:
            init_vga(&g_display);
            break;
        case DRIVER_VBE:
            init_vbe(&g_display, boot_config);
            break;
        default:
            // Handle unsupported driver types if necessary
            break;
    }
}

void deinit_display() {
        switch (g_display.type) {
            case DRIVER_VGA:
            case DRIVER_VBE:
                g_display.type = DRIVER_NONE;
                g_display.execute_command = NULL;
                break;
            default:
                // Handle unsupported driver types if necessary
                break;
        }
}




enum display_result display_clear(enum display_clear_mode mode) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_CLEAR,
        .clear = { .flag = mode }
    };
    return g_display.execute_command(&g_display, &cmd);
}

enum display_result  display_set_color(const enum display_color fg, const enum display_color bg) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_SET_COLOR,
        .set_color = { .fg = fg, .bg = bg }
    };

    return g_display.execute_command(&g_display, &cmd);
}


enum display_result  display_put_char(const char c, const uint32_t x, const uint32_t y, const enum display_color fg, const enum display_color bg) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_PUT_CHAR,
        .put_char = {
             .c = c, 
             .x = x,
             .y = y,
             .fg = fg,
            .bg = bg
            }
    };
    return g_display.execute_command(&g_display, &cmd);
}

enum display_result  display_put_string(const char *str, const uint32_t x, const uint32_t y, const enum display_color fg, const enum display_color bg) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_PUT_STRING,
        .put_string = {
            .str = str,
             .x = x,
             .y = y,
            .fg = fg,
            .bg = bg
        }
    };
    return g_display.execute_command(&g_display, &cmd);
}

enum display_result  display_put_pixel(const uint32_t x, const uint32_t y, const uint32_t pixel_color) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_PUT_PIXEL,
        .put_pixel = {
             .x = x,
             .y = y,
             .pixel_color = pixel_color
            }
    };
    return g_display.execute_command(&g_display, &cmd);
}

const display_data_t *get_display_data(void) {
    return &g_display.data;
}
