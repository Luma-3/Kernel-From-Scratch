#include "terminal.h"
#include "vbe.h"
#include <stdint.h>

int32_t print_cursor(const struct terminal *term, bool visible) {

    uint32_t height = term->font_height >> 2; // divide by 4
    uint32_t x_pos = term->cursor.x * term->font_width;
    uint32_t y_pos =
        (term->cursor.y * term->font_height) + term->font_height - height;

    uint32_t color = visible ? ansii_color_codes[term->fg_color]
                             : ansii_color_codes[term->bg_color];

    return vbe_fill_rect(x_pos, y_pos, term->font_width, height, color);
}

void advance_cursor(struct terminal *term) {
    term->cursor.x++;

    if (term->cursor.x >= term->char_by_line) {
        newline(term);
    }
}

void newline(struct terminal *term) {

    term->cursor.x = 0;
    term->cursor.y++;

    if (term->cursor.y >= term->line_by_screen) {
        scroll(term);
        term->cursor.y = term->line_by_screen - 1;
        return;
    }
}

void tab(struct terminal *term) {
    term->cursor.x = (term->cursor.x + 8) & ~(7);

    if (term->cursor.x >= term->char_by_line) {
        newline(term);
    }
}

int32_t move_cursor(struct terminal *term, uint32_t x, uint32_t y) {
    if (x >= term->char_by_line || y >= term->line_by_screen) {
        return KTERM_ERR_INVALID_CURSOR_POS;
    }
    term->cursor.x = x;
    term->cursor.y = y;
    return KTERM_SUCCESS;
}
