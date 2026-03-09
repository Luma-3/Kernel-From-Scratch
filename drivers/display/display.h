//
// Created by gcaptari on 2/27/26.
//

#ifndef __KFS_DISPLAY_H
#define __KFS_DISPLAY_H

# include <stddef.h>
#include <stdint.h>
#include <stdint.h>
# include <stdbool.h>

typedef struct s_diplay_data display_data_t;
typedef struct s_display display_t;
typedef struct s_display_boot_config display_boot_config_t;

enum driver_type {
    DRIVER_NONE = 0,
    DRIVER_VGA,
    DRIVER_VBE
};

enum display_color {
    COLOR_BLACK = 0x000000,
    COLOR_BLUE = 0x0000FF,
    COLOR_GREEN = 0x00FF00,
    COLOR_CYAN = 0x00FFFF,
    COLOR_RED = 0xFF0000,
    COLOR_MAGENTA = 0xFF00FF,
    COLOR_BROWN = 0x808000,
    COLOR_LIGHT_GREY = 0xC0C0C0,
    COLOR_DARK_GREY = 0x808080,
    COLOR_LIGHT_BLUE = 0xADD8E6,
    COLOR_LIGHT_GREEN = 0x90EE90,
    COLOR_LIGHT_CYAN = 0xE0FFFF,
    COLOR_LIGHT_RED = 0xFFB6C1,
    COLOR_LIGHT_MAGENTA = 0xFFC0CB,
    COLOR_YELLOW = 0xFFFF00,
    COLOR_WHITE = 0xFFFFFF
};

enum display_command {
    DISPLAY_CMD_CLEAR,
    DISPLAY_CMD_SET_COLOR,
    DISPLAY_CMD_PUT_CHAR,
    DISPLAY_CMD_PUT_STRING,
    DISPLAY_CMD_PUT_PIXEL
};

enum display_clear_mode {
    DISPLAY_CLEAR_NONE = 0,
    DISPLAY_CLEAR_COLOR = 1 << 0,
    DISPLAY_CLEAR_CONTENT = 1 << 1,
    DISPLAY_CLEAR_ALL = DISPLAY_CLEAR_COLOR | DISPLAY_CLEAR_CONTENT
};

enum display_result {
    DISPLAY_SUCCESS,
    DISPLAY_ERROR,
};

struct s_display_command {
    enum display_command cmd;
    union {
        struct {
            enum display_clear_mode flag;
        } clear;
        struct {
            uint32_t fg;
            uint32_t bg;
        } set_color;
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t pixel_color;
        } put_pixel;
        struct {
            char c;
            uint32_t x;
            uint32_t y;
            bool use_default_color;
            uint32_t fg;
            uint32_t bg;
        } put_char;
        struct {
            const char *str;
            uint32_t x;
            uint32_t y;
            uint32_t fg;
            uint32_t bg;
        } put_string;
    };
};

struct s_diplay_color {
    enum display_color fg;
    enum display_color bg;
};

struct s_diplay_data {
    void *framebuffer;
    struct  s_diplay_color e_color;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t char_width;
    uint32_t char_height;
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        } palette;
        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        } rgb;
    } color_info;
};

struct s_display_boot_config {
    uint32_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        } palette;
        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        } rgb;
    } color_info;
};

struct s_display {
    enum driver_type type;
    display_data_t data;
    enum display_result (*execute_command)(display_t* , struct s_display_command *cmd);
};

void init_display(enum driver_type type, const display_boot_config_t *boot_config);

void deinit_display(void);

enum display_result  display_clear(enum display_clear_mode mode);
enum display_result  display_set_color(enum display_color fg, enum display_color bg);

enum display_result display_put_char(char c, uint32_t x, uint32_t y, enum display_color fg, enum display_color bg);

enum display_result display_put_pixel(uint32_t x, uint32_t y, uint32_t pixel_color);

const display_data_t *get_display_data(void);



#endif //__KFS_DISPLAY_H