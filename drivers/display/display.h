//
// Created by gcaptari on 2/27/26.
//

#ifndef __KFS_DISPLAY_H
#define __KFS_DISPLAY_H

# include <stddef.h>
#include <stdint.h>
#include <stdint.h>

typedef struct s_diplay_data display_data_t;
typedef struct s_display display_t;

enum driver_type {
    DRIVER_NONE = 0,
    DRIVER_VGA,
};

enum display_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15
};

enum display_command {
    DISPLAY_CMD_CLEAR,
    DISPLAY_CMD_SET_COLOR,
    DISPLAY_CMD_PUT_CHAR,
    DISPLAY_CMD_PUT_STRING,
};

enum display_result {
    DISPLAY_SUCCESS,
    DISPLAY_ERROR,
};

struct s_display_command {
    enum display_command cmd;
    union {
        struct {
            enum display_color fg;
            enum display_color bg;
        } set_color;
        struct {
            char c;
            size_t x;
            size_t y;
        } put_char;
        struct {
            const char *str;
            size_t x;
            size_t y;
        } put_string;
    };
};

struct s_diplay_color {
    enum display_color fg;
    enum display_color bg;
};

struct s_diplay_position {
    size_t x;
    size_t y;
};

struct s_diplay_data {
    void *framebuffer;
    struct  s_diplay_color e_color;
    size_t width;
    size_t height;
    size_t bpp;
    uint8_t color;
};

struct s_display {
    enum driver_type type;
    display_data_t data;
    struct s_diplay_position cursor;
    enum display_result (*execute_command)(display_t* , struct s_display_command *cmd);
};

void init_display(enum driver_type type);

void deinit_display(void);

enum display_result  display_clear(void);

enum display_result  display_set_color(enum display_color fg, enum display_color bg);

enum display_result display_put_char_with_position(char c, size_t x, size_t y);

enum display_result display_put_char(char c);



#endif //__KFS_DISPLAY_H