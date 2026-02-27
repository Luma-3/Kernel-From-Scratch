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


static inline void vga_putentryat(display_t *display, uint8_t color, char c,  size_t x, size_t y) {
    const size_t index = y * display->data.width + x;
    uint16_t*terminal_buffer = (uint16_t*) display->data.framebuffer;
    terminal_buffer[index] = VGA_ENTRY(c, color);
}

static inline enum display_result vga_clear(display_data_t *settings) {
    uint16_t *terminal_buffer = (uint16_t*) settings->framebuffer;
    uint8_t terminal_color = settings->color;
    uint16_t reset_value = VGA_ENTRY(' ', terminal_color);
    for (size_t y = 0; y < settings->height; y++) {
        for (size_t x = 0; x < settings->width; x++) {
            const size_t index = y * settings->width + x;
            terminal_buffer[index] = reset_value;
        }
    }

    return DISPLAY_SUCCESS;
}

static inline enum display_result vga_set_color(display_data_t *settings, enum display_color fg, enum display_color bg) {
    uint16_t *terminal_buffer = (uint16_t*) settings->framebuffer;
    uint8_t terminal_color = VGA_MERGE_COLOR(fg, bg);
    settings->e_color.fg = fg;
    settings->e_color.bg = bg;
    settings->color = terminal_color;
    for (size_t y = 0; y < settings->height; y++) {
        for (size_t x = 0; x < settings->width; x++) {
            const size_t index = y * settings->width + x;
            terminal_buffer[index] = VGA_ENTRY(VGA_EXTRACT_CHAR(terminal_buffer[index]), terminal_color);
        }
    }
    return DISPLAY_SUCCESS;
}


enum display_result vga_execute_command(display_t *display, struct s_display_command *command) {
    switch (command->cmd) {
        case DISPLAY_CMD_CLEAR:
            vga_clear(&display->data);
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_SET_COLOR:
            vga_set_color(&display->data, command->set_color.fg, command->set_color.bg);
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_PUT_CHAR:
            vga_putentryat(display, display->data.color, command->put_char.c, command->put_char.x, command->put_char.y);
            return DISPLAY_SUCCESS;
        case DISPLAY_CMD_PUT_STRING:
            // TODO: Implement string output if needed
            return DISPLAY_ERROR; // Not implemented
        default:
            return DISPLAY_ERROR; // Unknown command
    }
}

void init_vga(display_t *display) {
    display->cursor.x = 0;
    display->cursor.y = 0;
    display->data.framebuffer = (void *)VGA_MEMORY;
    display->data.width = VGA_WIDTH;
    display->data.height = VGA_HEIGHT;
    display->data.e_color = (struct s_diplay_color) {
        .fg = COLOR_LIGHT_GREY,
        .bg = COLOR_BLACK
    };
    display->data.color = VGA_MERGE_COLOR(display->data.e_color.fg, display->data.e_color.bg);
    display->execute_command = &vga_execute_command;
}