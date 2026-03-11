#ifndef __KFS_TERMINAL_H
#define __KFS_TERMINAL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct s_terminal terminal_t;
typedef struct s_cursor_position cursor_position_t;

struct s_cursor_position {
    uint32_t x;
    uint32_t y;
};

struct s_terminal {
    const char *name;
    cursor_position_t cursor;
    cursor_position_t default_cursor;
    // enum display_color fg_color;
    // enum display_color bg_color;
};

void init_terminal(const char *name);

// void terminal_put_char_with_color(const char c, const enum display_color fg,
//                                   const enum display_color bg);
void terminal_put_char(const char c);

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

#endif
