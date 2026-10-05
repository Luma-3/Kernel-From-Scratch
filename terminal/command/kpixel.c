#include "kpixel.h"
#include "kpixel_art.h"
#include "vbe.h"
#include "pit.h"
#include "ps2.h"
#include "keyevent.h"
#include "printf.h"
#include "printk.h"
#include "terminal.h"
#include "command.h"
#include "klibc/kunistd.h"
#include "klibc/string/string.h"
#include "klibc/memory/mem.h"
#include <stdbool.h>
#include <stddef.h>

static void extract_str(void *data, const uint8_t *src, size_t n) {
    if (src == NULL || data == NULL) {
        return;
    }
    char *dest = (char *)data;
    size_t max_len = 31;
    if (n < max_len) {
        max_len = n;
    }
    kmemcpy(dest, src, max_len);
    dest[max_len] = '\0';
}

static void extract_param(void *data, const uint8_t *src, size_t n) {
    if (src == NULL || data == NULL) {
        return;
    }
    (void)n;
    *((int32_t *)data) = katoi((const char *)src);
}

static void str_tolower(char *str) {
    if (str == NULL) {
        return;
    }
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = (char)(str[i] + ('a' - 'A'));
        }
    }
}

static void wait_for_key(void) {
    pit_sleep_ms(300);
    while (ps2_has_data()) {
        ps2_keyboard_poll();
        (void)kbd_pop_event();
    }
    while (1) {
        if (ps2_has_data()) {
            ps2_keyboard_poll();
            struct key_event event = kbd_pop_event();
            if (event.keycode != 0 && event.state.pressed) {
                break;
            }
        }
        pit_sleep_ms(20);
    }
    while (ps2_has_data()) {
        ps2_keyboard_poll();
        (void)kbd_pop_event();
    }
}

int32_t kpixel_command(int32_t argc, const uint8_t *argv) {
    char art_name[32] = {0};
    int32_t loops = 5;
    int32_t scale = 6;

    if (argc >= 1 && argv != NULL) {
        if (extract_arguments(&argv, (void *)art_name, &extract_str) != SUCCESS) {
            art_name[0] = '\0';
        }
    }

    str_tolower(art_name);

    // If first argument is a number (e.g. `kpixel 5 6`), treat it as loops for parrot
    if (art_name[0] >= '0' && art_name[0] <= '9') {
        loops = katoi(art_name);
        if (loops <= 0) {
            loops = 5;
        }
        kmemcpy(art_name, "parrot", 7);
        if (argc >= 2 && argv != NULL) {
            int32_t parsed_scale = 0;
            if (extract_arguments(&argv, (void *)&parsed_scale, &extract_param) == SUCCESS && parsed_scale > 0) {
                scale = parsed_scale;
            }
        }
    } else if (kstrcmp(art_name, "parrot") == 0 || kstrcmp(art_name, "party") == 0 ||
               kstrcmp(art_name, "party_parrot") == 0) {
        if (argc >= 2 && argv != NULL) {
            int32_t parsed_loops = 0;
            if (extract_arguments(&argv, (void *)&parsed_loops, &extract_param) == SUCCESS && parsed_loops > 0) {
                loops = parsed_loops;
            }
        }
        if (argc >= 3 && argv != NULL) {
            int32_t parsed_scale = 0;
            if (extract_arguments(&argv, (void *)&parsed_scale, &extract_param) == SUCCESS && parsed_scale > 0) {
                scale = parsed_scale;
            }
        }
    }

    if (art_name[0] == '\0') {
        kmemcpy(art_name, "parrot", 7);
    }

    if (kstrcmp(art_name, "help") == 0 || kstrcmp(art_name, "list") == 0) {
        printf("Available drawings: parrot, apple, potato, saturn, axolote\n");
        printf("Usage:\n");
        printf("  kpixel <drawing> [loops] [scale]\n");
        printf("  kpixel parrot [loops] [scale]\n");
        printf("  kpixel apple\n");
        printf("  kpixel potato\n");
        printf("  kpixel saturn\n");
        printf("  kpixel axolote\n");
        return 0;
    }

    bool is_parrot = (kstrcmp(art_name, "parrot") == 0 || kstrcmp(art_name, "party") == 0 ||
                      kstrcmp(art_name, "party_parrot") == 0);
    bool is_apple = (kstrcmp(art_name, "apple") == 0 || kstrcmp(art_name, "pomme") == 0);
    bool is_potato = (kstrcmp(art_name, "potato") == 0 || kstrcmp(art_name, "patate") == 0);
    bool is_saturn = (kstrcmp(art_name, "saturn") == 0 || kstrcmp(art_name, "saturne") == 0);
    bool is_axolote = (kstrcmp(art_name, "axolote") == 0 || kstrcmp(art_name, "axolotl") == 0 ||
                       kstrcmp(art_name, "bucket") == 0);

    if (!is_parrot && !is_apple && !is_potato && !is_saturn && !is_axolote) {
        printf("Unknown drawing '%s'.\n", art_name);
        printf("Available drawings: parrot, apple, potato, saturn, axolote\n");
        printf("Usage: kpixel <drawing> [loops] [scale]\n");
        return -1;
    }

    uint32_t screen_w = vbe_get_width();
    uint32_t screen_h = vbe_get_height();

    if (is_parrot) {
        uint32_t sprite_w = 35 * (uint32_t)scale;
        uint32_t sprite_h = 25 * (uint32_t)scale;

        uint32_t start_x = 0;
        uint32_t start_y = 0;
        if (screen_w > sprite_w) {
            start_x = (screen_w - sprite_w) / 2;
        }
        if (screen_h > sprite_h) {
            start_y = (screen_h - sprite_h) / 2;
        }

        printf("Starting Party Parrot demo (%d loops, scale %d)...\n", loops, scale);
        printf("Press any key to stop.\n");
        pit_sleep_ms(300);

        demo_party_parrot_vbe(start_x, start_y, (uint32_t)scale, (uint32_t)loops, 50);
    } else {
        uint32_t start_x = (screen_w > 64) ? (screen_w - 64) / 2 : 0;
        uint32_t start_y = (screen_h > 270) ? (screen_h - 270) / 2 : 0;

        printf("Drawing %s...\n", art_name);
        printf("Press any key to return.\n");
        pit_sleep_ms(300);

        if (is_apple) {
            draw_apple_vbe(start_x, start_y);
        } else if (is_potato) {
            draw_potato_vbe(start_x, start_y);
        } else if (is_saturn) {
            draw_saturn_vbe(start_x, start_y);
        } else if (is_axolote) {
            draw_axelote_on_bucket_vbe(start_x, start_y);
        }

        wait_for_key();
    }

    term_refresh(active_terminal);
    (void)term_read_line("trucOs>");
    return 0;
}
