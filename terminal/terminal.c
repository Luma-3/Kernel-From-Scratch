#include "terminal.h"
#include "font.h"
#include "vbe.h"

#include <stdint.h>

static struct terminal terminals[MAX_TERMINALS] = {0};

static uint8_t terminal_count = 0;

int32_t init_terminal(const char *name) {

    if (terminal_count >= MAX_TERMINALS) {
        return KTERM_ERR_TOO_MANY_TERM;
    }
    struct terminal *term = (struct terminal *)&terminals[terminal_count];

    terminal_count++;

    term->name = name;
    term->cursor.x = 0;
    term->cursor.y = 0;

    term_write(term->id, (const int8_t *)"Terminal ", 9);

    // display_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    // display_clear(DISPLAY_CLEAR_ALL);

    // g_terminal.fg_color = COLOR_LIGHT_GREY;
    // g_terminal.bg_color = COLOR_BLACK;
    // terminal_writestring("Welcome to ");

    // terminal_writestring_with_color(name, COLOR_CYAN, COLOR_BLACK);
    // terminal_put_char('\n');
    // g_terminal.default_cursor.y = g_terminal.cursor.y;
    // terminal_writestring(
    // "__Kernel From Scratch__ is a simple kernel written in C, designed to "
    //     "run on x86 architecture. It serves as a learning project for "
    //     "understanding the basics of operating system development, including
    //     " "memory management, process scheduling, and hardware
    //     interaction.\n");
    // terminal_put_char('\n');
    // terminal_write_int(get_display_data()->char_width);
    // terminal_put_char('x');
    // terminal_write_int(get_display_data()->char_height);
    // terminal_put_char('\n');

    // terminal_write_int(get_display_data()->color_info.rgb.framebuffer_red_field_position);
    // terminal_put_char('\n');

    // terminal_write_int(get_display_data()->color_info.rgb.framebuffer_green_field_position);
    // terminal_put_char('\n');

    // terminal_write_int(
    // get_display_data()->color_info.rgb.framebuffer_blue_field_position);
}

void handle_special_char(const uint8_t term_id, const int8_t c) {
    struct terminal *term = &terminals[term_id];

    if (c == '\n') {
        term->cursor.x = 0;
        if (++term->cursor.y >= term->line_by_screen) {
            term->cursor.y = 0;
        }
        return;
    }
    if (c == '\r') {
        term->cursor.x = 0;
        return;
    }
    if (c == '\t') {
        term->cursor.x = (term->cursor.x + 8) & ~(8 - 1);
        if (term->cursor.x >= term->char_by_line) {
            term->cursor.x = 0;
            if (++term->cursor.y >= term->line_by_screen) {
                term->cursor.y = 0;
            }
        }
    }
    if (c == '\b') {
        if (term->cursor.x > 0) {
            term->cursor.x--;
        } else if (term->cursor.y > 0) {
            term->cursor.y--;
            term->cursor.x = term->char_by_line - 1;
        }
        // TODO: clear the char on the screen
        // TODO: handle blank char
    }
}

void char_to_font(int8_t c, uint8_t *buffer) {
    for (int i = 0; i < 8; i++) {
        buffer[i] = fontdata_8x8[c * 8 + i];
    }
}

static int32_t draw_font(uint32_t x, uint32_t y, const uint8_t *bitmap,
                         uint32_t fg_color, uint32_t bg_color) {
    for (uint32_t i = 0; i < 8; ++i) {
        uint8_t raw_data = bitmap[i];

        for (uint32_t j = 0; j < 8; ++j) {
        }
    }
}

int32_t term_put_char(const uint8_t term_id, const int8_t c) {
    if (term_id >= terminal_count) {
        return KTERM_ERR_INVALID_TERM;
    }

    // handle_special_char(term_id, c);

    uint8_t font_buffer[8];
    char_to_font(c, font_buffer);

    if (vbe_putbitmap(terminals[term_id].cursor.x * 8,
                      terminals[term_id].cursor.y * 8,
                      (const uint32_t *)font_buffer, 8, 8) != VBE_SUCCESS) {
        return KTERM_ERR_INVALID_CHAR;
    }

    return KTERM_SUCCESS;
}

int32_t term_write(const uint8_t term_id, const int8_t *data,
                   const uint32_t size) {
    int32_t r = 0;
    if (term_id >= terminal_count) {
        return KTERM_ERR_INVALID_TERM;
    }
    for (uint32_t i = 0; i < size; i++) {

        if (term_put_char(term_id, data[i]) != KTERM_SUCCESS) {
            r = KTERM_ERR_INVALID_TERM;
            break;
        }
        r++;
    }
    return r;
}

// void terminal_put_char_with_color(const char c, const enum display_color fg,
//                                   const enum display_color bg) {
//     // const display_data_t *display_data = get_display_data();
//     if (c == '\n') {
//         g_terminal.cursor.x = 0;
//         if (++g_terminal.cursor.y >= display_data->char_height) {
//             g_terminal.cursor.y = g_terminal.default_cursor.y;
//         }
//         return;
//     }
//     if (c == '\r') {
//         g_terminal.cursor.x = 0;
//         return;
//     }
//     if (c == '\t') {
//         g_terminal.cursor.x = (g_terminal.cursor.x + 8) & ~(8 - 1);
//         if (g_terminal.cursor.x >= display_data->char_width) {
//             g_terminal.cursor.x = 0;
//             if (++g_terminal.cursor.y >= display_data->char_height) {
//                 g_terminal.cursor.y = g_terminal.default_cursor.y;
//             }
//         }
//     }
//     if (display_put_char(c, g_terminal.cursor.x, g_terminal.cursor.y, fg, bg)
//     ==
//         DISPLAY_ERROR) {
//         return;
//     }
//     if (++g_terminal.cursor.x >= display_data->char_width) {
//         g_terminal.cursor.x = 0;
//         if (++g_terminal.cursor.y >= display_data->char_height) {
//             g_terminal.cursor.y = g_terminal.default_cursor.y;
//         }
//     }
//     return;
// }

// void terminal_put_char(const char c) {
//     const display_data_t *display_data = get_display_data();
//     if (c == '\n') {
//         g_terminal.cursor.x = 0;
//         if (++g_terminal.cursor.y >= display_data->char_height) {
//             g_terminal.cursor.y = g_terminal.default_cursor.y;
//         }
//         return;
//     }
//     if (c == '\r') {
//         g_terminal.cursor.x = 0;
//         return;
//     }
//     if (c == '\t') {
//         g_terminal.cursor.x = (g_terminal.cursor.x + 8) & ~(8 - 1);
//         if (g_terminal.cursor.x >= display_data->char_width) {
//             g_terminal.cursor.x = 0;
//             if (++g_terminal.cursor.y >= display_data->char_height) {
//                 g_terminal.cursor.y = g_terminal.default_cursor.y;
//             }
//         }
//     }
//     if (display_put_char(c, g_terminal.cursor.x, g_terminal.cursor.y,
//                          g_terminal.fg_color,
//                          g_terminal.bg_color) == DISPLAY_ERROR) {
//         return;
//     }
//     if (++g_terminal.cursor.x >= display_data->char_width) {
//         g_terminal.cursor.x = 0;
//         if (++g_terminal.cursor.y >= display_data->char_height) {
//             g_terminal.cursor.y = g_terminal.default_cursor.y;
//         }
//     }
//     return;
// }
//
// void terminal_write(const char *data, uint32_t size) {
//     for (uint32_t i = 0; i < size; i++) {
//         terminal_put_char(data[i]);
//     }
// }
//
// void terminal_write_with_color(const char *data, uint32_t size,
//                                const enum display_color fg,
//                                const enum display_color bg) {
//     for (uint32_t i = 0; i < size; i++) {
//         terminal_put_char_with_color(data[i], fg, bg);
//     }
// }
//
// void terminal_writestring(const char *data) {
//     terminal_write(data, k_strlen(data));
// }
//
// void terminal_writestring_with_color(const char *data,
//                                      const enum display_color fg,
//                                      const enum display_color bg) {
//     terminal_write_with_color(data, k_strlen(data), fg, bg);
// }

void terminal_write_int(int value) {
    char buffer[12];
    int index = 0;

    if (value < 0) {
        buffer[index++] = '-';
        value = -value;
    }

    if (value == 0) {
        buffer[index++] = '0';
    } else {
        while (value > 0) {
            buffer[index++] = '0' + (value % 10);
            value /= 10;
        }
    }
    // Reverse the buffer
    for (int i = index - 1; i >= 0; i--) {
        // terminal_put_char(buffer[i]);
    }
}

// void terminal_cursor_move(uint32_t x, uint32_t y) {
//     const display_data_t *display_data = get_display_data();
//     if (x >= display_data->char_width || y >= display_data->char_height) {
//         return;
//     }
//     g_terminal.cursor.x = x;
//     g_terminal.cursor.y = y;
// }
//
// void terminal_put_pixel(uint32_t x, uint32_t y, uint32_t pixel_color) {
//     display_put_pixel(x, y, pixel_color);
// }
//
// bool terminal_clear(void) {
//     if (display_clear(DISPLAY_CLEAR_ALL) == DISPLAY_ERROR) {
//         return false;
//     }
//     g_terminal.cursor.x = 0;
//     g_terminal.cursor.y = 0;
//     return true;
// }
