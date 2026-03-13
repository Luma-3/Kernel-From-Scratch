#include "string.h"
#include "terminal.h"
#include <stddef.h>
#include <stdint.h>

enum ansi_color_mode { ANSI_COLOR_STANDARD, ANSI_COLOR_RGB, ANSI_COLOR_256 };

uint8_t decode_ansii_color(const char *color_str, uint32_t default_color) {
    int color_index = color_str[0] - '0';
    if (color_index >= 0 && color_index < 8) {
        return color_index;
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
        term->fg_color = 7;
        term->bg_color = 0;
        return;
    }

    bool is_foreground = true;

    uint8_t fg_color = term->fg_color;
    uint8_t bg_color = term->bg_color;

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
            uint8_t color = decode_ansii_color(p, default_color);
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
