#include "string.h"
#include "terminal.h"
#include <stddef.h>
#include <stdint.h>

enum ansi_color_mode { ANSI_COLOR_STANDARD, ANSI_COLOR_RGB, ANSI_COLOR_256 };

uint32_t decode_ansii_color(const char *color_str, uint32_t default_color) {
    static const uint32_t standard_colors[] = {
        COLOR_BLACK, COLOR_RED,     COLOR_GREEN, COLOR_YELLOW,
        COLOR_BLUE,  COLOR_MAGENTA, COLOR_CYAN,  COLOR_GREY,
    };
    int color_index = color_str[0] - '0';
    if (color_index >= 0 && color_index < 8) {
        return standard_colors[color_index];
    } else {
        return default_color;
    }

    return default_color;
}

void handle_ansii_esc_seq(struct terminal *term, const char **seq,
                          uint32_t default_color) {
    if (kstarts_with(*seq, "\033[") == false) {
        return;
    }

    // Skip the ESC and [
    const char *p = *seq + 2;

    if (*p == '0' && *(p + 1) == 'm') {
        *seq = p + 2;
        term->fg_color = COLOR_GREY;
        term->bg_color = COLOR_BLACK;
        return;
    }

    bool is_foreground = true;

    uint32_t fg_color = term->fg_color;
    uint32_t bg_color = term->bg_color;

    while (*p != '\0' && *p != 'm') {
        if (*p == ';') {
            ++p;
            continue;
        }
        if (*p == '3' || *p == '4') {
            is_foreground = (*p == '3');
            ++p;
        } else {
            ++p;
            break;
        }

        // Parse parameters
        if (*p >= '0' && *p <= '9') {
            uint32_t color = decode_ansii_color(p, default_color);
            if (is_foreground) {
                fg_color = color;
            } else {
                bg_color = color;
            }
            ++p; // Skip the color code and 'm'
        }
    }
    if (*p == 'm') {
        term->fg_color = fg_color;
        term->bg_color = bg_color;
        ++p;
    }
    *seq = p;
}
