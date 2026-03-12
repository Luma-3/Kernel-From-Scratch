#include "terminal.h"
#include "ascii.h"
#include "font.h"
#include "keyevent.h"
#include "mem.h"
#include "string.h"
#include "vbe.h"

#include <stdalign.h>
#include <stdint.h>

static struct terminal terminals[MAX_TERMINALS];

static uint8_t terminal_count = 0;

static uint8_t active_terminal = 0;

static inline __attribute__((always_inline)) uint8_t *char_to_font(int8_t c) {
    return (uint8_t *)fontdata_8x8 + (c * 8);
}

static void scroll(struct terminal *term) {
    kmemmove(term->buffer, term->buffer + term->char_by_line,
             (term->line_by_screen - 1) * term->char_by_line *
                 sizeof(uint16_t));

    // re print screen
    for (uint32_t y = 0; y < term->line_by_screen - 1; y++) {
        for (uint32_t x = 0; x < term->char_by_line; x++) {
            uint16_t cell = term->buffer[x + y * term->char_by_line];
            uint8_t c = cell & 0xFF;
            uint32_t fg_color = ansii_color_codes[(cell >> 12) & 0xF];
            uint32_t bg_color = ansii_color_codes[(cell >> 8) & 0xF];

            uint8_t *font_buffer = char_to_font(c);
            vbe_draw_glyph_sized(x * term->font_width, y * term->font_height,
                                 font_buffer, 8, 8, fg_color, bg_color,
                                 term->font_width, term->font_height);
        }
    }

    // Clear the last line
    kmemset(term->buffer + (term->line_by_screen - 1) * term->char_by_line,
            (term->fg_color << 12) | (term->bg_color << 8) | ' ',
            term->char_by_line * sizeof(uint16_t));
    vbe_fill_rect(0, (term->line_by_screen - 1) * term->font_height,
                  term->font_width * term->char_by_line, term->font_height,
                  term->bg_color);
}

static void newline(struct terminal *term) {
    term->buffer[term->cursor.x + term->cursor.y] = '\n';
    if (term->cursor.y >= term->line_by_screen - 1) {
        scroll(term);
        term->cursor.y = term->line_by_screen - 1;
        term->cursor.x = 0;
        return;
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

    if (handle_special_char(term, c))
        return KTERM_SUCCESS;

    term->buffer[term->cursor.x + term->cursor.y * term->char_by_line] =
        (term->fg_color << 12) | (term->bg_color << 8) | c;

    uint8_t *font_buffer = char_to_font(c);

    if (vbe_draw_glyph_sized(term->cursor.x * term->font_width,
                             term->cursor.y * term->font_height, font_buffer, 8,
                             8, ansii_color_codes[term->fg_color],
                             ansii_color_codes[term->bg_color],
                             term->font_width, term->font_height)) {
        return KTERM_ERR_INVALID_CHAR;
    }

    term->cursor.x++;
    if (term->cursor.x >= term->char_by_line) {
        term->cursor.x = 0;
        if (term->cursor.y >= term->line_by_screen - 1) {
            scroll(term);
            term->cursor.y = term->line_by_screen - 1;
        } else {
            term->cursor.y++;
        }
    }

    return KTERM_SUCCESS;
}

static void change_term(uint8_t new_term_id) {
    if (new_term_id >= terminal_count) {
        return;
    }
    active_terminal = new_term_id;
    struct terminal *term = (struct terminal *)&terminals[active_terminal];

    // re print screen
    for (uint32_t y = 0; y < term->line_by_screen; y++) {
        for (uint32_t x = 0; x < term->char_by_line; x++) {
            uint16_t cell = term->buffer[x + y * term->char_by_line];
            uint8_t c = cell & 0xFF;
            uint32_t fg_color = ansii_color_codes[(cell >> 12) & 0xF];
            uint32_t bg_color = ansii_color_codes[(cell >> 8) & 0xF];

            uint8_t *font_buffer = char_to_font(c);
            vbe_draw_glyph_sized(x * term->font_width, y * term->font_height,
                                 font_buffer, 8, 8, fg_color, bg_color,
                                 term->font_width, term->font_height);
        }
    }
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
    term_write(term->id, (const uint8_t *)term->name, kstrlen(term->name));

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

void term_poll() {
    uint8_t c = term_get_keyevent();
    if (c) {
        term_write(active_terminal, &c, 1);
    }
}
