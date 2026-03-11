#ifndef __KFS_TERMINAL_H
#define __KFS_TERMINAL_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_TERMINALS 3
#define CHAR_BY_LINE 160
#define LINE_BY_SCREEN 45

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
    uint8_t buffer[]
    // enum display_color fg_color;
    // enum display_color bg_color;
    // TODO: probably should be removed and manage this otherwise
};

int32_t init_terminal(const char *name);

// void terminal_put_char_with_color(const char c, const enum display_color fg,
//                                   const enum display_color bg);
// void terminal_put_char(const char c);

void terminal_write(const char *data, uint32_t size);

// void terminal_write_with_color(const char *data, uint32_t size,
//                                const enum display_color fg,
//                                const enum display_color bg);

void terminal_writestring(const char *data);

// void terminal_writestring_with_color(const char *data,
//                                      const enum display_color fg,
//                                      const enum display_color bg);

void terminal_write_int(int value);

void terminal_cursor_move(uint32_t x, uint32_t y);

void terminal_put_pixel(uint32_t x, uint32_t y, uint32_t pixel_color);

bool terminal_clear(void);

// NEW API

int32_t term_write(const uint8_t term_id, const int8_t *data,
                   const uint32_t size);

#endif
