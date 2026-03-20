#include "terminal.h"
#include "ascii.h"
#include "font.h"
#include "keyevent.h"
#include "mem.h"
#include "printf.h"
#include "vbe.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

uint8_t active_terminal = 0;

static struct terminal terminals[MAX_TERMINALS];

static uint8_t terminal_count = 0;

static inline __attribute__((always_inline)) uint8_t *char_to_font(int8_t c) {
    return (uint8_t *)fontdata_8x8 + (c * 8);
}

static int32_t refresh_cell(struct terminal *term, uint32_t x, uint32_t y) {
    uint16_t cell = term->buffer[x + y * term->char_by_line];

    uint8_t c = cell & 0xFF;
    uint32_t fg_color = ansii_color_codes[(cell >> 12) & 0xF];
    uint32_t bg_color = ansii_color_codes[(cell >> 8) & 0xF];

    uint8_t *font_buffer = char_to_font(c);

    return vbe_draw_glyph_sized(x * term->font_width, y * term->font_height,
                                font_buffer, 8, 8, fg_color, bg_color,
                                term->font_width, term->font_height);
}

static int32_t draw_term(struct terminal *term, uint8_t x, uint8_t y,
                         uint32_t width, uint32_t height) {
    uint32_t err = 0;

    for (uint32_t j = y; j < y + height; j++) {
        for (uint32_t i = x; i < x + width; i++) {
            if ((err = refresh_cell(term, i, j)) != VBE_SUCCESS) {
                return err;
            }
        }
    }

    return KTERM_SUCCESS;
}

int32_t scroll(struct terminal *term) {

    size_t line_size = term->char_by_line * sizeof(uint16_t);
    size_t all_but_last = (term->line_by_screen - 1) * line_size;

    kmemmove(term->buffer, term->buffer + term->char_by_line,
             (term->line_by_screen - 1) * term->char_by_line *
                 sizeof(uint16_t));

    uint16_t empty_cell = (term->fg_color << 12) | (term->bg_color << 8) | ' ';
    for (uint32_t i = 0; i < term->char_by_line; i++) {
        term->buffer[(term->line_by_screen - 1) * term->char_by_line + i] =
            empty_cell;
    }

    return draw_term(term, 0, 0, term->char_by_line, term->line_by_screen);
}

static void backspace(struct terminal *term) {
    if (term->cursor.x > 0) {
        term->cursor.x--;
    } else if (term->cursor.y > 0) {
        term->cursor.y--;
        term->cursor.x = term->char_by_line - 1;
    }

    term->buffer[term->cursor.x + term->cursor.y * term->char_by_line] =
        (term->fg_color << 12) | (term->bg_color << 8) | ' ';

    vbe_fill_rect(term->cursor.x * term->font_width,
                  term->cursor.y * term->font_height, term->font_width,
                  term->font_height, ansii_color_codes[term->bg_color]);
}

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

    // Dellete the cursor before drawing the character
    refresh_cell(term, term->cursor.x, term->cursor.y);

    if (handle_special_char(term, c))
        return KTERM_SUCCESS;

    // Store the character in the buffer with color attributes
    term->buffer[term->cursor.x + term->cursor.y * term->char_by_line] =
        (term->fg_color << 12) | (term->bg_color << 8) | c;

    if (refresh_cell(term, term->cursor.x, term->cursor.y) != VBE_SUCCESS) {
        return KTERM_ERR_VBE_FAILURE;
    }

    advance_cursor(term);

    return KTERM_SUCCESS;
}

int32_t change_term(uint8_t new_term_id) {
    if (new_term_id >= terminal_count) {
        return KTERM_ERR_INVALID_TERM;
    }
    active_terminal = new_term_id;
    struct terminal *term = (struct terminal *)&terminals[active_terminal];

    return draw_term(term, 0, 0, term->char_by_line, term->line_by_screen);
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
    term->bg_color = 0;
    term->fg_color = 7;

    kmemset(term->buffer, (term->fg_color << 12) | (term->bg_color << 8) | ' ',
            term->char_by_line * term->line_by_screen);

    term_write(term->id, (const uint8_t *)"\033[32mWelcome to KFS\n", 20);
    printf("Terminal %s initialized\n", name);

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

int32_t term_refresh(const uint8_t term_id) {
    struct terminal *term = &terminals[term_id];

    return draw_term(term, 0, 0, term->char_by_line, term->line_by_screen);
}

uint8_t term_get_keyevent() {
    struct key_event event = kbd_pop_event();

    if (event.keycode == 0 || !event.state.pressed)
        return 0;

    if (event.state.alt && event.keycode >= KEY_F1 && event.keycode <= KEY_F3) {
        // Alt + F1/F2/F3 to switch terminal
        active_terminal = event.keycode - KEY_F1;
        change_term(active_terminal);
        return 0;
    }

    uint8_t to_print = 0;

    if (event.state.shift == 1) {
        to_print = keycode_ascii_shift[event.keycode];
    } else {
        to_print = keycode_ascii[event.keycode];
    }
    return to_print;
}

struct terminal *get_active_terminal() {
    if (active_terminal >= terminal_count) {
        return NULL;
    }
    return &terminals[active_terminal];
}

void term_poll() {
    uint8_t c = term_get_keyevent();
    if (c) {
        struct terminal *term = get_active_terminal();
        term_write(active_terminal, &c, 1);
        print_cursor(term, true);
    }
}
