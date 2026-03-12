#include "terminal.h"
#include "ascii.h"
#include "font.h"
#include "keyevent.h"
#include "mem.h"
#include "vbe.h"

#include <stdalign.h>
#include <stdint.h>

static struct terminal terminals[MAX_TERMINALS];

static uint8_t terminal_count = 0;

static inline __attribute__((always_inline)) uint8_t *char_to_font(int8_t c) {
    return (uint8_t *)fontdata_8x8 + (c * 8);
}

static void newline(struct terminal *term) {
    term->buffer[term->cursor.x + term->cursor.y] = '\n';
    if (term->cursor.y >= term->line_by_screen - 1) {
        // TODO: Scroll
    }
    term->cursor.x = 0;
    term->cursor.y++;
}

static void tab(struct terminal *term) {
    term->cursor.x = (term->cursor.x + 8) & ~(7);
    if (term->cursor.x >= term->char_by_line) {
        newline(term);
    }
}

static void backspace(struct terminal *term) {
    if (term->cursor.x > 0) {
        term->cursor.x--;
    } else if (term->cursor.y > 0) {
        term->cursor.y--;
        term->cursor.x = term->char_by_line - 1;
    }

    vbe_fill_rect(term->cursor.x * term->font_width,
                  term->cursor.y * term->font_height, term->font_width,
                  term->font_height, term->bg_color);
}

static void scroll(struct terminal *term) {}

static int32_t handle_special_char(struct terminal *term, const int8_t c) {

    if (c == '\n') {
        newline(term);
        return 1;
    }
    if (c == '\r') {
        term->cursor.x = 0;
        return 1;
    }
    if (c == '\t') {
        tab(term);
        return 1;
    }
    if (c == '\b') {
        backspace(term);
        return 1;
    }
    return 0;
}

static int32_t putchar(struct terminal *term, const uint8_t c) {

    if (handle_special_char(term, c))
        return KTERM_SUCCESS;

    term->buffer[term->cursor.x + term->cursor.y] =
        (term->fg_color << 12) | (term->bg_color << 8) | c;

    uint8_t *font_buffer = char_to_font(c);

    if (vbe_draw_glyph_sized(term->cursor.x * term->font_width,
                             term->cursor.y * term->font_height, font_buffer, 8,
                             8, term->fg_color, term->bg_color,
                             term->font_width, term->font_height)) {
        return KTERM_ERR_INVALID_CHAR;
    }

    term->cursor.x++;
    if (term->cursor.x >= term->char_by_line) {
        term->cursor.x = 0;
        term->cursor.y++;
    }

    return KTERM_SUCCESS;
}

int32_t init_terminal(const char *name) {

    if (terminal_count >= MAX_TERMINALS) {
        return KTERM_ERR_TOO_MANY_TERM;
    }

    struct terminal *term = (struct terminal *)&terminals[terminal_count];
    term->id = terminal_count;

    terminal_count++;

    term->name = name;
    term->cursor.x = 0;
    term->cursor.y = 0;
    term->char_by_line = CHAR_BY_LINE;
    term->line_by_screen = LINE_BY_SCREEN;
    term->font_height = FONT_HEIGHT;
    term->font_width = FONT_WIDTH;
    term->bg_color = COLOR_BLACK;
    term->fg_color = COLOR_GREY;

    k_memset(term->buffer, ' ', term->char_by_line * term->line_by_screen);
    // TODO : draw Rect bg_color;

    term_write(term->id, (const uint8_t *)"\033[34;45mTerminal\n Test\b", 28);

    return KTERM_SUCCESS;
}

int32_t term_write(const uint8_t term_id, const uint8_t *data,
                   const uint32_t size) {
    if (term_id >= terminal_count) {
        return KTERM_ERR_INVALID_TERM;
    }

    struct terminal *term = (struct terminal *)&terminals[term_id];

    uint8_t *it = (uint8_t *)data;
    uint8_t *end = (uint8_t *)data + size;

    uint32_t err;

    while (it < end && *it != '\0') {
        handle_ansii_esc_seq(term, (const char **)&it, COLOR_GREY);
        if ((err = putchar(term, *it)) != KTERM_SUCCESS)
            return err;
        ++it;
    }

    return it - data;
}

uint8_t term_get_keyevent() {
    struct key_event event = kbd_pop_event();

    if (event.keycode == 0 || !event.state.pressed)
        return 0;

    uint8_t to_print = 0;

    if (event.state.shift == 1) {
        to_print = keycode_ascii_shift[event.keycode];
    } else {
        to_print = keycode_ascii[event.keycode];
    }
    return to_print;
}

void term_poll() {
    uint8_t c = term_get_keyevent();
    if (c) {
        term_write(0, &c, 1);
    }
}
