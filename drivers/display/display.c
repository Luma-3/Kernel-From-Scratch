//
// Created by gcaptari on 2/27/26.
//

# include "display.h"
# include "vga/vga.h"

display_t g_display = {
    .type = DRIVER_NONE,
    .execute_command = NULL
};


void init_display(const enum driver_type type) {
    g_display.type = type;

    switch (type) {
        case DRIVER_VGA:
            init_vga(&g_display);
            break;
        default:
            // Handle unsupported driver types if necessary
            break;
    }
}

void deinit_display() {
        switch (g_display.type) {
            case DRIVER_VGA:
                g_display.type = DRIVER_NONE;
                g_display.execute_command = NULL;
                break;
            default:
                // Handle unsupported driver types if necessary
                break;
        }
}




enum display_result display_clear(void) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_CLEAR
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


enum display_result  display_put_char(const char c, const uint32_t x, const uint32_t y) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_PUT_CHAR,
        .put_char = { .c = c, .x = x, .y = y }
    };
    return g_display.execute_command(&g_display, &cmd);
}

enum display_result  display_put_string(const char *str, const uint32_t x, const uint32_t y) {
    if (g_display.execute_command == NULL) {
        return DISPLAY_ERROR;
    }
    struct s_display_command cmd = {
        .cmd = DISPLAY_CMD_PUT_STRING,
        .put_string = { .str = str, .x = x, .y = y }
    };
    return g_display.execute_command(&g_display, &cmd);
}

const display_data_t *get_display_data(void) {
    return &g_display.data;
}
