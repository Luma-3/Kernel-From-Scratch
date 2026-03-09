#include "terminal.h"
#include "../klibc/string/string.h"

terminal_t g_terminal = {0};

void init_terminal(const char* name) {
    display_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    display_clear();
    display_put_char('A', 0, 0);
    g_terminal.name = name;
    g_terminal.cursor.x = 0;
    g_terminal.cursor.y = 0;
    g_terminal.default_cursor.x = 0;
    g_terminal.default_cursor.y = 0;
    terminal_writestring(name);
    terminal_put_char('\n');
    terminal_writestring("______________________________\n");
    g_terminal.default_cursor.y = g_terminal.cursor.y;
}


void terminal_put_char(const char c) {
    const display_data_t *display_data = get_display_data();
    if (c == '\n') {
        g_terminal.cursor.x = 0;
        if (++g_terminal.cursor.y >= display_data->height) {
            g_terminal.cursor.y = g_terminal.default_cursor.y;
        }
        return;
    }
    if (c == '\r') {
        g_terminal.cursor.x = g_terminal.default_cursor.x;
        return;
    }
    if (c == '\t') {
        g_terminal.cursor.x = (g_terminal.cursor.x + 8) & ~(8 - 1);
        if (g_terminal.cursor.x >= display_data->width) {
            g_terminal.cursor.x = 0;
            if (++g_terminal.cursor.y >= display_data->height) {
                g_terminal.cursor.y = g_terminal.default_cursor.y;
            }
        }
    }
    if (display_put_char(c, g_terminal.cursor.x, g_terminal.cursor.y) == DISPLAY_ERROR) {
        return;
    }
    if (++g_terminal.cursor.x >= display_data->width) {
        g_terminal.cursor.x = 0;
        if (++g_terminal.cursor.y >= display_data->height) {
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

void terminal_writestring(const char *data) {
     terminal_write(data, k_strlen(data));
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

bool terminal_clear(void) {
    if (display_clear() == DISPLAY_ERROR) {
        return false;
    }
    g_terminal.cursor.x = 0;
    g_terminal.cursor.y = 0;
    return true;
}