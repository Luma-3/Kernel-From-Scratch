#ifndef KFS_VGA_H
#define KFS_VGA_H

#include <stdint.h>
#include <stdbool.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

enum vga_color {
    VGA_COLOR_BLACK         = 0x0,
    VGA_COLOR_BLUE          = 0x1,
    VGA_COLOR_GREEN         = 0x2,
    VGA_COLOR_CYAN          = 0x3,
    VGA_COLOR_RED           = 0x4,
    VGA_COLOR_MAGENTA       = 0x5,
    VGA_COLOR_BROWN         = 0x6,
    VGA_COLOR_LIGHT_GREY    = 0x7,
    VGA_COLOR_DARK_GREY     = 0x8,
    VGA_COLOR_LIGHT_BLUE    = 0x9,
    VGA_COLOR_LIGHT_GREEN   = 0xA,
    VGA_COLOR_LIGHT_CYAN    = 0xB,
    VGA_COLOR_LIGHT_RED     = 0xC,
    VGA_COLOR_LIGHT_MAGENTA = 0xD,
    VGA_COLOR_LIGHT_BROWN   = 0xE,
    VGA_COLOR_WHITE         = 0xF,
};

enum vga_step_status {
    VGA_STEP_OK,
    VGA_STEP_FAIL,
    VGA_STEP_INFO,
    VGA_STEP_WARN
};

void vga_init(void);
void vga_clear(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putchar(char c);
void vga_puts(const char *str);
void vga_puthex(uint32_t val);
void vga_putdec(uint32_t val);
void vga_log_step(const char *step_name, enum vga_step_status status);
void vga_set_cursor(uint8_t x, uint8_t y);
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void vga_disable_cursor(void);

#endif // KFS_VGA_H
