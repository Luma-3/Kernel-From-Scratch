#include "terminal.h"
#include "../klibc/string/string.h"

terminal_t g_terminal = {0};

void init_terminal(const char* name) {
    display_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    display_clear(DISPLAY_CLEAR_ALL);
    g_terminal.name = name;
    g_terminal.cursor.x = 0;
    g_terminal.cursor.y = 0;
    g_terminal.default_cursor.x = 0;
    g_terminal.default_cursor.y = 0;
    g_terminal.fg_color = COLOR_LIGHT_GREY;
    g_terminal.bg_color = COLOR_BLACK;
    terminal_writestring("Welcome to ");
    terminal_writestring_with_color(name, COLOR_CYAN, COLOR_BLACK);
    terminal_put_char('\n');
    g_terminal.default_cursor.y = g_terminal.cursor.y;
    terminal_writestring("__Kernel From Scratch__ is a simple kernel written in C, designed to run on x86 architecture. It serves as a learning project for understanding the basics of operating system development, including memory management, process scheduling, and hardware interaction.\n");
    terminal_put_char('\n');
    terminal_write_int(get_display_data()->char_width);
    terminal_put_char('x');
    terminal_write_int(get_display_data()->char_height);
        terminal_put_char('\n');

    terminal_write_int(get_display_data()->color_info.rgb.framebuffer_red_field_position);    
    terminal_put_char('\n');

    terminal_write_int(get_display_data()->color_info.rgb.framebuffer_green_field_position);
        terminal_put_char('\n');

    terminal_write_int(get_display_data()->color_info.rgb.framebuffer_blue_field_position);
}


void terminal_put_char_with_color(const char c, const enum display_color fg, const enum display_color bg) {
    const display_data_t *display_data = get_display_data();
    if (c == '\n') {
        g_terminal.cursor.x = 0;
        if (++g_terminal.cursor.y >= display_data->char_height) {
            g_terminal.cursor.y = g_terminal.default_cursor.y;
        }
        return;
    }
    if (c == '\r') {
        g_terminal.cursor.x = 0;
        return;
    }
    if (c == '\t') {
        g_terminal.cursor.x = (g_terminal.cursor.x + 8) & ~(8 - 1);
        if (g_terminal.cursor.x >= display_data->char_width) {
            g_terminal.cursor.x = 0;
            if (++g_terminal.cursor.y >= display_data->char_height) {
                g_terminal.cursor.y = g_terminal.default_cursor.y;
            }
        }
    }
    if (display_put_char(c, g_terminal.cursor.x, g_terminal.cursor.y, fg, bg) == DISPLAY_ERROR) {
        return;
    }
    if (++g_terminal.cursor.x >= display_data->char_width) {
        g_terminal.cursor.x = 0;
        if (++g_terminal.cursor.y >= display_data->char_height) {
            g_terminal.cursor.y = g_terminal.default_cursor.y;
        }
    }
    return;
}


void terminal_put_char(const char c) {
    const display_data_t *display_data = get_display_data();
    if (c == '\n') {
        g_terminal.cursor.x = 0;
        if (++g_terminal.cursor.y >= display_data->char_height) {
            g_terminal.cursor.y = g_terminal.default_cursor.y;
        }
        return;
    }
    if (c == '\r') {
        g_terminal.cursor.x = 0;
        return;
    }
    if (c == '\t') {
        g_terminal.cursor.x = (g_terminal.cursor.x + 8) & ~(8 - 1);
        if (g_terminal.cursor.x >= display_data->char_width) {
            g_terminal.cursor.x = 0;
            if (++g_terminal.cursor.y >= display_data->char_height) {
                g_terminal.cursor.y = g_terminal.default_cursor.y;
            }
        }
    }
    if (display_put_char(c, g_terminal.cursor.x, g_terminal.cursor.y, g_terminal.fg_color, g_terminal.bg_color) == DISPLAY_ERROR) {
        return;
    }
    if (++g_terminal.cursor.x >= display_data->char_width) {
        g_terminal.cursor.x = 0;
        if (++g_terminal.cursor.y >= display_data->char_height) {
            g_terminal.cursor.y = g_terminal.default_cursor.y;
        }
    }
    return;
}


void terminal_write(const char *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        terminal_put_char(data[i]);
    }
}

void terminal_write_with_color(const char *data, uint32_t size, const enum display_color fg, const enum display_color bg) {
    for (uint32_t i = 0; i < size; i++) {
        terminal_put_char_with_color(data[i], fg, bg);
    }
}

void terminal_writestring(const char *data) {
     terminal_write(data, k_strlen(data));
}

void terminal_writestring_with_color(const char *data, const enum display_color fg, const enum display_color bg) {
     terminal_write_with_color(data, k_strlen(data), fg, bg);
}


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
        terminal_put_char(buffer[i]);
    }
}

void terminal_cursor_move(uint32_t x, uint32_t y) {
    const display_data_t *display_data = get_display_data();
    if (x >= display_data->char_width || y >= display_data->char_height) {
        return;
    }
    g_terminal.cursor.x = x;
    g_terminal.cursor.y = y;
}

void terminal_put_pixel(uint32_t x, uint32_t y, uint32_t pixel_color) {
    display_put_pixel(x, y, pixel_color);
}

bool terminal_clear(void) {
    if (display_clear(DISPLAY_CLEAR_ALL) == DISPLAY_ERROR) {
        return false;
    }
    g_terminal.cursor.x = 0;
    g_terminal.cursor.y = 0;
    return true;
}