#ifndef KFS_DISPLAY_H
#define KFS_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "multiboot.h"

enum display_mode {
    DISPLAY_MODE_NONE = 0,
    DISPLAY_MODE_VGA,
    DISPLAY_MODE_VBE
};

enum display_step_status {
    DISPLAY_STEP_OK,
    DISPLAY_STEP_FAIL,
    DISPLAY_STEP_INFO,
    DISPLAY_STEP_WARN
};

#define DISPLAY_UNIVERSAL_COLOR_BLACK   0
#define DISPLAY_UNIVERSAL_COLOR_RED     1
#define DISPLAY_UNIVERSAL_COLOR_GREEN   2
#define DISPLAY_UNIVERSAL_COLOR_YELLOW  3
#define DISPLAY_UNIVERSAL_COLOR_BLUE    4
#define DISPLAY_UNIVERSAL_COLOR_MAGENTA 5
#define DISPLAY_UNIVERSAL_COLOR_CYAN    6
#define DISPLAY_UNIVERSAL_COLOR_GREY    7 
#define DISPLAY_UNIVERSAL_COLOR_DARK_GREY 8
#define DISPLAY_UNIVERSAL_COLOR_BRIGHT_RED     9
#define DISPLAY_UNIVERSAL_COLOR_BRIGHT_GREEN   10
#define DISPLAY_UNIVERSAL_COLOR_BRIGHT_YELLOW  11
#define DISPLAY_UNIVERSAL_COLOR_BRIGHT_BLUE    12
#define DISPLAY_UNIVERSAL_COLOR_BRIGHT_MAGENTA 13
#define DISPLAY_UNIVERSAL_COLOR_BRIGHT_CYAN    14
#define DISPLAY_UNIVERSAL_COLOR_WHITE          15
#

/**
 * @brief Initialise le pilote d'affichage universel.
 * Detecte automatiquement la presence du VBE via Multiboot.
 * Si VBE est disponible, active DISPLAY_MODE_VBE.
 * Sinon, active DISPLAY_MODE_VGA (mode texte 80x25 a 0xB8000).
 */
void display_init(multiboot_info_t *mbi);

/**
 * @brief Renvoie le mode d'affichage actuellement actif.
 */
enum display_mode display_get_mode(void);

/**
 * @brief Nombre de colonnes de texte (80 en VGA, 80 en VBE).
 */
uint32_t display_get_cols(void);

/**
 * @brief Nombre de lignes de texte (25 en VGA, 45 en VBE).
 */
uint32_t display_get_rows(void);

/**
 * @brief Largeur en pixels de l'ecran (0 en mode texte pur, 1280 en VBE).
 */
uint32_t display_get_width_px(void);

/**
 * @brief Hauteur en pixels de l'ecran (0 en mode texte pur, 720 en VBE).
 */
uint32_t display_get_height_px(void);

/**
 * @brief Efface l'ecran avec la couleur de fond specifiee.
 */
void display_clear(uint32_t bg_rgb);

/**
 * @brief Ecrit un caractere avec defilement (terminal/early boot).
 */
void display_putchar(char c);

/**
 * @brief Ecrit une chaine de caracteres.
 */
void display_puts(const char *str);

/**
 * @brief Affiche une cellule de caractere a une position (col, row) donnee.
 * Fait le pont transparent : ecrit dans 0xB8000 en VGA, ou dessine le glyphe en VBE.
 */
void display_draw_cell(uint32_t col, uint32_t row, char c, uint32_t fg, uint32_t bg);

/**
 * @brief Met a jour la position du curseur sur l'ecran.
 * @param color Couleur rgb du curseur (0xRRGGBB) en mode VBE, ou couleur VGA (0-7) en mode texte.
 */
void display_set_cursor(uint32_t col, uint32_t row, uint32_t color);
/**
 * @brief Affiche le curseur a une position (col, row) donnee.
 * @param color Couleur rgb du curseur (0xRRGGBB) en mode VBE, ou couleur VGA (0-7) en mode texte.
 */
void display_reveal_cursor(uint32_t col, uint32_t row, uint32_t color);

/**
 * @brief Cache le curseur a une position (col, row) donnee.
 */
void display_hide_cursor(uint32_t col, uint32_t row, uint32_t color);

/**
 * @brief Affiche une cellule de caractere a une position (col, row) donnee.
 * Fait le pont transparent : ecrit dans 0xB8000 en VGA, ou dessine le glyphe en VBE.
 */
void display_draw_cell_universal_color(uint32_t col, uint32_t row, char c, uint8_t fg, uint8_t bg);

/**
 * @brief Met a jour la position du curseur sur l'ecran.
 * Fait le pont transparent : ecrit dans 0xB8000 en VGA, ou dessine un rectangle en VBE.
 * @param color Couleur du curseur (0-15) en mode universel.
 */
void display_set_cursor_universal_color(uint32_t col, uint32_t row, uint8_t color);

/**
 * @brief Affiche le curseur a une position (col, row) donnee.
 * @param color Couleur du curseur (0-15) en mode universel.
 */
void display_reveal_cursor_universal_color(uint32_t col, uint32_t row, uint8_t color);
/**
 * @brief Cache le curseur a une position (col, row) donnee.
 * @param color Couleur du curseur (0-15) en mode universel.
 */
void display_hide_cursor_universal_color(uint32_t col, uint32_t row, uint8_t color);

/**
 * @brief Logue une etape de boot avec statut colore ([ OK ], [ FAIL ], [ INFO ], [ WARN ]).
 */
void display_log_step(const char *step_name, enum display_step_status status);

#endif // KFS_DISPLAY_H
