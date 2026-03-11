#ifndef __KFS_TERMINAL_H
#define __KFS_TERMINAL_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_TERMINALS 3
#define CHAR_BY_LINE 160
#define LINE_BY_SCREEN 45
#define FONT_WIDTH 8u
#define FONT_HEIGHT 16u

enum kterm_err {
    KTERM_SUCCESS = 0,
    KTERM_ERR_TOO_MANY_TERM = -1,
    KTERM_ERR_INVALID_TERM = -2,
    KTERM_ERR_INVALID_CHAR = -3,
};

struct cursor_pos {
    uint32_t x;
    uint32_t y;
};

struct terminal {
    uint32_t id;
    const char *name;
    struct cursor_pos cursor;

    uint32_t char_by_line;
    uint32_t line_by_screen;
    uint32_t font_height;
    uint32_t font_width;
    uint32_t fg_color;
    uint32_t bg_color;
    uint8_t buffer[CHAR_BY_LINE * LINE_BY_SCREEN];
};

int32_t init_terminal(const char *name);

int32_t term_write(const uint8_t term_id, const uint8_t *data,
                   const uint32_t size);

void term_poll();

#endif
