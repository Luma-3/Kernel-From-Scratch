#include "terminal.h"
#include "display.h"
#include <stdint.h>

int32_t print_cursor(struct terminal *term, bool visible) {
    if (term == nullptr) {
        return KTERM_ERR_INVALID_TERM;
    }
    uint8_t value_cursor = term->cursor_visible;
    term->cursor_visible = visible;
    if(term->color_universal){
        if (visible) {
            if(!value_cursor) {
                display_reveal_cursor_universal_color(term->cursor.x, term->cursor.y, term->cursor_color);
            }
            display_set_cursor_universal_color(term->cursor.x, term->cursor.y, term->cursor_color);
        }else {
            display_hide_cursor_universal_color(term->cursor.x, term->cursor.y, term->bg_color);
        }
    }else{
        if (visible) {
            if(!value_cursor) {
                display_reveal_cursor(term->cursor.x, term->cursor.y, term->cursor_color);
            }
            display_set_cursor(term->cursor.x, term->cursor.y, term->cursor_color);
        }else {
            display_hide_cursor(term->cursor.x, term->cursor.y, term->bg_color);
        }
    }
    return KTERM_SUCCESS;
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
