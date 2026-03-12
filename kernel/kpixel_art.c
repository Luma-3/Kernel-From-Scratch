#include "vbe.h"
# include <stdint.h>

// Version VBE - dessine une pomme 16x16 en pixel art (style Minecraft)
void draw_apple_vbe(uint32_t start_x, uint32_t start_y) {
    // Définition des couleurs
    const uint32_t red = 0xDC143C;         // Rouge vif
    const uint32_t dark_red = 0x8B0000;    // Rouge très foncé / bordeaux
    const uint32_t med_red = 0xB22222;     // Rouge moyen
    const uint32_t light_red = 0xFF6B8A;   // Rose clair (reflet)
    const uint32_t brown = 0x654321;       // Brun
    const uint32_t dark_brown = 0x3D2817;  // Brun foncé
    const uint32_t green = 0x228B22;       // Vert feuille
    const uint32_t dark_green = 0x006400;  // Vert foncé
    
    // Pixel art de la pomme 16x16
    // ' ' = vide, 'B' = brun, 'b' = brun foncé, 'G' = vert, 'g' = vert foncé
    // 'R' = rouge, 'r' = rouge moyen, 'D' = rouge foncé/bordeaux, 'P' = rose clair (reflet)
    const char apple[16][17] = {
        "                ",
        "        B       ",
        "       BB       ",
        "       B        ",
        "     RRRRRR     ",
        "   RRRRRRRRRR   ",
        "  RRRRRRRRRRRR  ",
        "  RRRRRRRRRRRR  ",
        "  RRRRRRRRRRRR  ",
        "  RRRRRRRRRRRR  ",
        "  RRRRRRRRRRRR  ",
        "   RRRRRRRRRR   ",
        "   RRRRRRRRRR   ",
        "    RRRRRRRR    ",
        "     RRRRRR     ",
    };

    uint32_t apple_bitmap[16 * 16] = {0};
    // Dessiner la pomme pixel par pixel avec scaling 4x pour qu'elle soit visible
    const uint32_t scale = 4;
    for (uint32_t y = 0; y < 16; y++) {
        for (uint32_t x = 0; x < 16; x++) {
            char pixel = apple[y][x];
            uint32_t color;
            
            switch (pixel) {
                case 'B': color = brown; break;
                case 'b': color = dark_brown; break;
                case 'G': color = green; break;
                case 'g': color = dark_green; break;
                case 'R': color = red; break;
                case 'r': color = med_red; break;
                case 'D': color = dark_red; break;
                case 'P': color = light_red; break;
                case ' ': continue; // Ne rien dessiner pour les espaces
                default: continue;
            }
            apple_bitmap[y * 16 + x] = color;
            
            // Dessiner un bloc de scale x scale pixels pour chaque pixel du sprite
            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    vbe_putpixel(start_x + x * scale + sx, start_y + y * scale + sy, color);
                }
            }
        }
    }
    vbe_draw_bitmap(start_x, start_y + 200, apple_bitmap, 16, 16/* , scale */);
}

void draw_potato_vbe(uint32_t start_x, uint32_t start_y) {
    // Définition des couleurs
    const uint32_t light_brown = 0xD2B48C; // Brun clair
    const uint32_t brown = 0x8B4513;       // Brun
    const uint32_t dark_brown = 0x654321;  // Brun foncé
    
    // Pixel art de la pomme 16x16
    // ' ' = vide, 'L' = brun clair, 'B' = brun, 'D' = brun foncé
    const char potato[16][17] = {
        "                ",
        "                ",
        "                ",
        "                ",
        "        BBBB    ",
        "     BBBDLLDB   ",
        "    BDLLLDDDBB  ",
        "   BLLLLLLLLDD  ",
        "  BDLLLLLDDDBD  ",
        "  BDLLLLLDDDBD  ",
        "  DLLLLDDDBBD   ",
        "   DLLLBBBDD    ",
        "    DDDDDD      "
    };
    uint32_t potato_bitmap[16 * 16] = {0};
        const uint32_t scale = 4;
    for (uint32_t y = 0; y < 16; y++) {
        for (uint32_t x = 0; x < 16; x++) {
            char pixel = potato[y][x];
            uint32_t color;
            
            switch (pixel) {
                case 'L': color = light_brown; break;
                case 'B': color = brown; break;
                case 'D': color = dark_brown; break;
                case ' ': continue; // Ne rien dessiner pour les espaces
                default: continue;
            }
            potato_bitmap[y * 16 + x] = color;
            
            // Dessiner un bloc de scale x scale pixels pour chaque pixel du sprite
            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    vbe_putpixel(start_x + x * scale + sx, start_y + y * scale + sy, color);
                }
            }
        }
    }
    vbe_draw_bitmap_scaled(start_x, start_y + 200, potato_bitmap, 16, 16, scale);
}

void draw_saturn_vbe(uint32_t start_x, uint32_t start_y) {
    // Définition des couleurs pour Saturne
    const uint32_t yellow = 0xF4D03F;        // Jaune doré (planète)
    const uint32_t dark_yellow = 0xD4AC0D;   // Jaune foncé
    const uint32_t orange = 0xE59866;        // Orange (bandes)
    const uint32_t beige = 0xFAD7A0;         // Beige clair
    const uint32_t ring_light = 0xD5DBDB;    // Anneau clair
    const uint32_t ring_med = 0xAEB6BF;      // Anneau moyen
    const uint32_t ring_dark = 0x85929E;     // Anneau foncé

    // Pixel art de Saturne 16x16 - Vue de face
    // ' ' = vide, 'Y' = jaune, 'D' = jaune foncé, 'O' = orange, 'B' = beige
    // 'L' = anneau clair, 'M' = anneau moyen, 'R' = anneau foncé
    const char saturn[16][17] = {
        "                ",
        "      DYYD      ",
        "    DDYYYYDD    ",
        "   DYYYYYYYYY   ",
        "  YOOOOOOOOOOY  ",
        " YBBBBBBBBBBBBY ",
        " YOOOOOOOOOOOOY ",
        "RMRRRRMRRRLRRRML",
        "LRRRRLRRRRRMRRRM",
        " YOOOOOOOOOOOOY ",
        " YBBBBBBBBBBBBY ",
        "  YOOOOOOOOOOY  ",
        "   DYYYYYYYYY   ",
        "    DDYYYYDD    ",
        "      DYYD      ",
        "                "
    };

    uint32_t saturn_bitmap[16 * 16] = {0};
    const uint32_t scale = 4;
    for (uint32_t y = 0; y < 16; y++) {
        for (uint32_t x = 0; x < 16; x++) {
            char pixel = saturn[y][x];
            uint32_t color;

            switch (pixel) {
                case 'Y': color = yellow; break;
                case 'D': color = dark_yellow; break;
                case 'O': color = orange; break;
                case 'B': color = beige; break;
                case 'L': color = ring_light; break;
                case 'M': color = ring_med; break;
                case 'R': color = ring_dark; break;
                case ' ': continue;
                default: continue;
            }

            saturn_bitmap[y * 16 + x] = color;
            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    vbe_putpixel(start_x + x * scale + sx, start_y + y * scale + sy, color);
                }
            }
        }
    }
    vbe_draw_bitmap_scaled(start_x, start_y + 200, saturn_bitmap, 16, 16, scale);
}

void draw_axelote_on_bucket_vbe(uint32_t start_x, uint32_t start_y) {
  // Définition des couleurs
  const uint32_t light_magenta = 0xFF77FF;   // Magenta clair
  const uint32_t dark_pink = 0xFF69B4;        // Rose foncé
  const uint32_t pink = 0xFFC0CB;       // Rose
  const uint32_t light_pink = 0xFFB6C1; //
  const uint32_t pale_pink = 0xFFB5C5;     // Rose pâle
  const uint32_t blue = 0x1E90FF;      // Bleu
  const uint32_t dark_blue = 0x00008B; // Bleu foncé
  const uint32_t dark_gray = 0xA9A9A9;    // Gris foncé
  const uint32_t gray = 0xD3D3D3;         // Gris clair
  const uint32_t light_gray = 0xDCDCDC;   // Gris très clair
  const uint32_t dark_purple = 0x800080;    // Violet foncé

  // Pixel art de l'axolote sur un seau 16x16
  // ' ' = vide, 'D' = rose foncé, 'P' = rose, 'L' = rose clair, 'A' = rose pâle, 'B' = bleu, 'b' = bleu foncé, 'G' = gris foncé, 'g' = gris clair, 'l' = gris très clair , 'V' = violet foncé, "R" = magenta clair
  const char axolote[16][17] = {
      "    D      D    ",
      "  P PD    DP P  ",
      "  DPLAAAAAALPD  ",
      "   DVAALLAAVD   ",
      "  DDLAAAAAALDD  ",
      "  GbRRRRRRRRbG  ",
      "  GBGRBBBgRGGG  ",
      "  GlgRGbBGRggG  ",
      "  GlDADgBDADgG  ",
      "  GlgDggggDggG  ",
      "  GglllgBggggG  ",
      "  GlllgggggggG  ",
      "   GglggggggG   ",
      "   GggggggggG   ",
      "    GglggggG    ",
      "     GGGGGG     "
  };


  uint32_t axolote_bitmap[16 * 16] = {0};
          const uint32_t scale = 4;
    for (uint32_t y = 0; y < 16; y++) {
        for (uint32_t x = 0; x < 16; x++) {
            char pixel = axolote[y][x];
            uint32_t color;
            
            switch (pixel) {
                case 'D': color = dark_pink; break;
                case 'P': color = pink; break;
                case 'L': color = light_pink; break;
                case 'A': color = pale_pink; break;
                case 'B': color = blue; break;
                case 'b': color = dark_blue; break;
                case 'G': color = dark_gray; break;
                case 'g': color = gray; break;
                case 'l': color = light_gray; break;
                case 'V': color = dark_purple; break;
                case 'R': color = light_magenta; break;
                case ' ': continue; // Ne rien dessiner pour les espaces
                default: continue;
            }
            axolote_bitmap[y * 16 + x] = color;
            // Dessiner un bloc de scale x scale pixels pour chaque pixel du sprite
            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    vbe_putpixel(start_x + x * scale + sx, start_y + y * scale + sy, color);
                }
            }
        }
    }
    vbe_draw_bitmap_scaled(start_x, start_y + 200, axolote_bitmap, 16, 16, scale);
}
