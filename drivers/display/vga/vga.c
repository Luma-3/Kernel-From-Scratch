//
// Created by gcaptari on 2/27/26.
//

# include "../display.h"
# include "vga.h"

#include <stdint.h>
#include <stdint.h>
#include <stdint.h>
#include <stdint.h>
#include <stdint.h>


static inline void vga_putentryat(display_t *display, uint8_t color, char c,  uint32_t x, uint32_t y) {
    const uint32_t index = y * display->data.width + x;
    uint16_t*terminal_buffer = (uint16_t*) display->data.framebuffer;
    terminal_buffer[index] = VGA_ENTRY(c, color);
}

static inline void vga_putstringat(display_t *display, uint8_t color, const char *str, uint32_t x, uint32_t y) {
    uint16_t*terminal_buffer = (uint16_t*) display->data.framebuffer;
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        const uint32_t index = y * display->data.width + x + i;
        terminal_buffer[index] = VGA_ENTRY(str[i], color);
    }
}

static inline enum display_result vga_clear(display_data_t *settings, enum display_clear_mode mode) {
    if(mode == DISPLAY_CLEAR_NONE) {
        return DISPLAY_SUCCESS; // Nothing to clear
    }
    uint16_t *terminal_buffer = (uint16_t*) settings->framebuffer;
    uint8_t terminal_color = VGA_MERGE_COLOR(settings->e_color.fg, settings->e_color.bg);
    for (uint32_t y = 0; y < settings->height; y++) {
        for (uint32_t x = 0; x < settings->width; x++) {
            const uint32_t index = y * settings->width + x;
                terminal_buffer[index] = VGA_ENTRY(
                    mode & DISPLAY_CLEAR_CONTENT ? '\0' : VGA_EXTRACT_CHAR(terminal_buffer[index]),
                     mode & DISPLAY_CLEAR_COLOR ? terminal_color : VGA_EXTRACT_COLOR(terminal_buffer[index]));

        }
    }

    return DISPLAY_SUCCESS;
}

static inline enum display_result vga_set_color(display_data_t *settings, enum display_color fg, enum display_color bg) {
    uint16_t *terminal_buffer = (uint16_t*) settings->framebuffer;
    uint8_t terminal_color = VGA_MERGE_COLOR(fg, bg);
    settings->e_color.fg = fg;
    settings->e_color.bg = bg;
    for (uint32_t y = 0; y < settings->height; y++) {
        for (uint32_t x = 0; x < settings->width; x++) {
            const uint32_t index = y * settings->width + x;
            terminal_buffer[index] = VGA_ENTRY(VGA_EXTRACT_CHAR(terminal_buffer[index]), terminal_color);
        }
    }
    return DISPLAY_SUCCESS;
}


enum display_result vga_execute_command(display_t *display, struct s_display_command *command) {
    switch (command->cmd) {
        case DISPLAY_CMD_CLEAR:
            vga_clear(&display->data, command->clear.flag);
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_SET_COLOR:
            vga_set_color(&display->data, command->set_color.fg, command->set_color.bg);
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_PUT_CHAR: {
            uint16_t color;
                color = VGA_MERGE_COLOR(
                    (
                        command->put_char.fg == COLOR_NONE ?
                        display->data.e_color.fg :
                        command->put_char.fg
                    ),
                    (
                        command->put_char.bg == COLOR_NONE ?
                        display->data.e_color.bg :
                        command->put_char.bg
                    ));
            vga_putentryat(display, color, command->put_char.c, command->put_char.x, command->put_char.y);
            return DISPLAY_SUCCESS;
        }
        case DISPLAY_CMD_PUT_STRING: {
            uint16_t color;
            const char *str = command->put_string.str;
            if(str == NULL) {
                return DISPLAY_ERROR;
            }
            color = VGA_MERGE_COLOR(
                (
                    command->put_char.fg == COLOR_NONE ?
                    display->data.e_color.fg :
                    command->put_char.fg
                ),
                (
                    command->put_char.bg == COLOR_NONE ?
                    display->data.e_color.bg :
                    command->put_char.bg
                ));
            vga_putstringat(display, color, str, command->put_string.x, command->put_string.y);
            return DISPLAY_SUCCESS;
        }
        default:
            return DISPLAY_ERROR; // Unknown command
    }
}

void init_vga(display_t *display) {
    display->data.framebuffer = (void *)VGA_MEMORY;
    display->data.width = VGA_WIDTH;
    display->data.height = VGA_HEIGHT;
    display->data.e_color = (struct s_diplay_color) {
        .fg = COLOR_LIGHT_GREY,
        .bg = COLOR_BLACK
    };
    display->execute_command = &vga_execute_command;
}