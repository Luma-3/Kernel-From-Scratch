#ifndef __KFS_TERMINAL_H
#define __KFS_TERMINAL_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_TERMINALS 3
#define CHAR_BY_LINE 160
#define LINE_BY_SCREEN 45
#define FONT_WIDTH 16u
#define FONT_HEIGHT 16u

enum kterm_err {
    KTERM_SUCCESS = 0,
    KTERM_ERR_TOO_MANY_TERM = -1,
    KTERM_ERR_INVALID_TERM = -2,
    KTERM_ERR_INVALID_CHAR = -3,
    KTERM_ERR_VBE_FAILURE = -4,
};

// 16 color palette
enum kterm_color {
    COLOR_BLACK = 0x000000,
    COLOR_GREY = 0x808080,
    COLOR_RED = 0xFF0000,
    COLOR_GREEN = 0x00FF00,
    COLOR_BLUE = 0x0000FF,
    COLOR_YELLOW = 0xFFFF00,
    COLOR_CYAN = 0x00FFFF,
    COLOR_MAGENTA = 0xFF00FF,
};

static const uint32_t ansii_color_codes[] = {
    COLOR_BLACK, COLOR_RED,     COLOR_GREEN, COLOR_YELLOW,
    COLOR_BLUE,  COLOR_MAGENTA, COLOR_CYAN,  COLOR_GREY,
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
    uint8_t bg_color;
    uint8_t fg_color;
    // 4 bit for fg, 4 bit for bg, 8 bit for char
    uint16_t buffer[CHAR_BY_LINE * LINE_BY_SCREEN];
};

extern uint8_t active_terminal;

int32_t init_terminal(const char *name);

int32_t term_write(const uint8_t term_id, const uint8_t *data,
                   const uint32_t size);

void term_poll();

void handle_ansii_esc_seq(struct terminal *term, const char **seq,
                          uint32_t default_color);

int32_t term_refresh(const uint8_t term_id);

int32_t change_term(uint8_t new_term_id);

#endif
