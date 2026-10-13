#include "vbe.h"
#include "kpixel_art.h"
#include "pit.h"
#include "keyevent.h"
#include "ps2.h"
#include <stdint.h>


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

static const char party_parrot_frames[10][25][36] = {
    // Frame 0
    {
        "                                   ",
        "                  WWWKWWKWW        ",
        "                 WWKDDDKdKW        ",
        "                WKDDDDDDDDDK       ",
        "               WDDdBBBBBBdDDW      ",
        "              WDDdDdBdDDDDdDK      ",
        "             WDDddDdBDdKKDDDDW     ",
        "             KDdBBDDBDKKKKDDDK     ",
        "            WDDBBBBBBDKKKgDBDDW    ",
        "            dDdBBBBBBDKKKKDBBDK    ",
        "           WDDBBBBBBBDKKKKDBBDDW   ",
        "           WDDBBBBBBBDKKKdDBBdDW   ",
        "          WDDBBBBBBBBddKKDBBBdDK   ",
        "          WDDBBBBBBBBdDKdDdBBBDK   ",
        "          DDdBBBBBBBBdDDDDBBBdDK   ",
        "         WDDddBBBBBBBBdDDdBBBdDW   ",
        "        WDDBBdBBBBBBBBBddBBBBDDW   ",
        "       WDDdBBdBBBBBBBBBBBBBBBDK    ",
        "     WWDDBBBBddddBBBBBBBBBBBdDW    ",
        "    WKDDdBBBBBBddddBBBBBBBBBBDW    ",
        "    KDdBBBBBBBBBBBBBBBBBBBBBdDW    ",
        "   WDDdBBBBBBBBBBBBBBBBBBBBBBDW    ",
        "  WDDBBBBBBBBBBBBBBBBBBBBBBBdDK    ",
        " WKDdBBBBBBBBBBBBBBBBBBBBBBBBDD    ",
        " WKDDDDDDDDDDDDDDDDDDDDDDDDDDDD    ",
    },
    // Frame 1
    {
        "             WKDDDDDDK             ",
        "            WWDDDDDDDDW            ",
        "           WDDDdBBdBdDDK           ",
        "          WKDdBBBBBBBBDDW          ",
        "         WdDddDBdDDDDdDDd          ",
        "         KDdBdDdDdKKdDDDDW         ",
        "        WDDBBdDBDKKKKDDDDDW        ",
        "        WDDBBBBBDKKKKDBBdDK        ",
        "        KDBBBBBBDKKKKDBBBDDW       ",
        "       WDDBBBBBBDKKgKDBBBBDW       ",
        "       WDdBBBBBBDgKKKDBBBdDK       ",
        "       KDdBBBBBBdDKKDdBBBBDK       ",
        "       KDBBBBBBBdDddDdBBBdDK       ",
        "       KDdBBBBBBBDDDDBBBBBDK       ",
        "       KDBBBBBBBBdDDBBBBBdDW       ",
        "       KDDBBBBBBBBBdBBBBBBDW       ",
        "      WDDdBBBBBBBBBBBBBBBdDK       ",
        "      WDDdddBBBBBBBBBBBBBBDDW      ",
        "     KDDdBBddddBBBBBBBBBBBdDK      ",
        "    WDDdBBBBBBBBBBBBBBBBBBBDDW     ",
        "   KDDBBBBBBBBBBBBBBBBBBBBBdDK     ",
        "  WDDdBBBBBBBBBBBBBBBBBBBBBBDDW    ",
        " WKDdBBBBBBBBBBBBBBBBBBBBBBBdDW    ",
        " KdDdBBBBBBBBBBBBBBBBBBBBBBBBDK    ",
        " WDDDDDDDDDDDDDDDDDDDDDDDDDDDDK    ",
    },
    // Frame 2
    {
        "                                   ",
        "         WWKDDdDDKW                ",
        "        WKDDdBBBDDdW               ",
        "       WDDDBBBBBBBDdW              ",
        "      WDDddBBdDddBDDK              ",
        "     WDDBdDBdDddDDDDDW             ",
        "     KDBBDDBDdKKKDDDDK             ",
        "    WDDBBBdBDdKKKddBDDW            ",
        "    WDdBBBBBddKKKDdBdDdW           ",
        "    KDdBBBBBDdKKKddBBdDW           ",
        "   WdDBBBBBBddKKKDdBBBDdW          ",
        "   WDDBBBBBBdDKKKDBBBBdDW          ",
        "   WDdBBBBBBBDdKDDBBBBdDW          ",
        "   WDDBBBBBBBDDDDDBBBBBDK          ",
        "   WDDBBBBBBBdDDDBBBBBdDW          ",
        "    KDBBBBBBBBdDdBBBBBdDW          ",
        "    KDdBBBBBBBBBBBBBBBdDKW         ",
        "    WDDdBBBBBBBBBBBBBBBdDKW        ",
        "    KDgddddBBBBBBBBBBBBBdDDW       ",
        "   WKDdBddddBBBBBBBBBBBBBBDDW      ",
        "   WDDBBBddddBBBBBBBBBBBBBBDDW     ",
        "  WKDdBBBBBBBBBBBBBBBBBBBBBBDDW    ",
        "  WDDBBBBBBBBBBBBBBBBBBBBBBBdDDW   ",
        "  WDdBBBBBBBBBBBBBBBBBBBBBBBBBDDW  ",
        "  WDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD  ",
    },
    // Frame 3
    {
        "                                   ",
        "        WWWWWW                     ",
        "       WKKKddWW                    ",
        "     WWDDDDDDDDWW                  ",
        "    WKDDdBBBBDDDKW                 ",
        "   WKDDBBBBBBBBdDKW                ",
        "  WKDdDDBBDDDDdDDDW                ",
        "  WDDBDDddDKddDDDDDW               ",
        "  KDBBddBDdKKKDDDdDW               ",
        " WDDBBBBBDdKKKddBBDDW              ",
        " WDdBBBBBddKKKDdBBdDW              ",
        "WWDdBBBBBDdKKKDdBBBDKW             ",
        "WKDBBBBBBddKKKDdBBBDDW             ",
        "WKDdBBBBBBDKKKDBBBBBDW             ",
        "WKDBBBBBBBDdKDDBBBBdDW             ",
        "WWDdBBBBBBDDDDDBBBBdDW             ",
        " WDdBBBBBBBDDDBBBBBdDW             ",
        " WDDBBBBBBBdDBBBBBBBDDdW           ",
        " WDDBBBBBBBBBBBBBBBBBDDDKW         ",
        "  KDdBBBBBBBBBBBBBBBBBBDDDKW       ",
        "  KDdddddBBBBBBBBBBBBBBBdDDKW      ",
        "  WDBddddddBBBBBBBBBBBBBBBDDDW     ",
        "  KDBBBBdddBBBBBBBBBBBBBBBBDDDW    ",
        "  KDBBBBBBBBBBBBBBBBBBBBBBBBBDDK   ",
        "  KDDDDDDDDDDDDDDDDDDDDDDDDDDDDK   ",
    },
    // Frame 4
    {
        "                                   ",
        "                                   ",
        "       WWWKWWW                     ",
        "      WdDDDDDKW                    ",
        "    WKDDddddDDDK                   ",
        "   WdDDBBBBBBBDDW                  ",
        "  WKDBBBBBBBBBBDDW                 ",
        "  KDDBDdBDDDDDdDDDW                ",
        " WDDBdDdBDKKKdDDDDK                ",
        "WKDdBBDdBDKKKKDDdDDW               ",
        "WDDBBBBBdDKKKdDBBdDK               ",
        "KDDBBBBBBDKKKKDBBBdDW              ",
        "dDBBBBBBBDKKKgDBBBBDK              ",
        "DDdBBBBBBDKKKddBBBBDdW             ",
        "DDBBBBBBBDdKKDBBBBBDKW             ",
        "dDdBBBBBBdDdDDBBBBBDK              ",
        "DDBBBBBBBdDDDdBBBBBDKW             ",
        "KDDBBBBBBBdDDBBBBBBdDK             ",
        "WDDBBBBBBBBdBBBBBBBBDDKW           ",
        "WdDdBBBBBBBBBBBBBBBBBdDDDW         ",
        "WdDddBBBBBBBBBBBBBBBBBddDDKW       ",
        "WDDddddddBBBBBBBBBBBBBBBdDDdW      ",
        "WDDBddddddBBBBBBBBBBBBBBBBDDDW     ",
        "WDDBBBBBBddBBBBBBBBBBBBBBBBdDDW    ",
        "WDDDDDDDDDDDDDDDDDDDDDDDDDDDDDK    ",
    },
    // Frame 5
    {
        "                                   ",
        "                                   ",
        "                                   ",
        "        WWWWWWW                    ",
        "       WWKDdDKKWW                  ",
        "     WWDDDDDDDDDDW                 ",
        "    WKDDdBBBBBddDDW                ",
        "    KDDBBdBBBBBBBDDW               ",
        "   WDDBBdDdBDDDDdDDW               ",
        "   KDdBBBDDDDKKdDDDd               ",
        "  WDDBBBBdBDKKKKDdDDW              ",
        "  WDdBBBBBBDgKKKDBBdDK             ",
        "  WDdBBBBBBDKKKKDBBBDDW            ",
        "  KDdBBBBBBDKKKKDBBBBDK            ",
        " WDdBBBBBBBDdKKKDBBBBDDW           ",
        " WDDBBBBBBBBDKKddBBBBdDW           ",
        " WKDBBBBBBBdDdKDdBBBBdDK           ",
        "  WDBBBBBBBBDDDDdBBBBBDK           ",
        "  KDBBBBBBBBdDDdBBBBBBDK           ",
        " WdDdBBBBBBBBddBBBBBBBDDW          ",
        " WDDdBBBBBBBBBBBBBBBBBdDDW         ",
        "WWDdddddddBBBBBBBBBBBBBBDDW        ",
        "WKDBddddddddBBBBBBBBBBBBdDDW       ",
        "WKDdBBBdBdddddBBBBBBBBBBBdDKW      ",
        " WDDDDDDDDDDDDDDDDDDDDDDDDDDW      ",
    },
    // Frame 6
    {
        "                                   ",
        "                                   ",
        "                                   ",
        "                                   ",
        "              WWWWW                ",
        "           WWWWKKKKWW              ",
        "          WKDDDDDDDDDW             ",
        "         WDDDddBBBddDDW            ",
        "       WKDDBBBBBBBBBBDDW           ",
        "       WDDBBdDBBBBBBBdDK           ",
        "      WDDBBBDDBBDDDDDDDK           ",
        "     WKDBBBBdDBDDKKKDDDDW          ",
        "     WDdBBBBBBBddKKKDddDK          ",
        "    WKDdBBBBBBBDdKKKddBDDW         ",
        "    WDdBBBBBBBBddKKKDdBdDW         ",
        "    WDdBBBBBBBBDdKKKddBBDKW        ",
        "    WDBBBBBBBBBddKKKDdBBDDW        ",
        "    WDdBBBBBBBBBDKKdDBBBdDW        ",
        "    WDdBBBBBBBBBDDdDDBBBDDW        ",
        "    KDDdBBBBBBBBdDDDdBBBdDW        ",
        "   WDDBddBBBBBBBBDDdBBBBDKW        ",
        "   KDdBddddBBBBBBBBBBBBdDW         ",
        "  WDdBBddddddBBBBBBBBBBDDW         ",
        " WDDBBBBBBdddddBBBBBBBBBDK         ",
        " WDDDDDDDDDDDDDDDDDDDDDDDW         ",
    },
    // Frame 7
    {
        "                                   ",
        "                                   ",
        "                                   ",
        "                                   ",
        "                                   ",
        "                 WWWWWW            ",
        "               WWKDDDDdWW          ",
        "             WWDDDDddDDDDW         ",
        "            WKDDdBBBBBBdDDW        ",
        "           WKDDBBBBBBBBBBDK        ",
        "           WDdBBdDBBDDDDDDDW       ",
        "          WDDBBBdDddDKKKDDDW       ",
        "          KDdBBBdDBdDKKKdDDK       ",
        "         WDDBBBBBBBdDKKKdDDDW      ",
        "        WdDBBBBBBBBddKKKddDDW      ",
        "        KDdBBBBBBBBdDKKKdDBDKW     ",
        "       WDDBBBBBBBBBBDKKKddBDdW     ",
        "       KDDBBBBBBBBBBDdKKDBBdDW     ",
        "      WDDBdBBBBBBBBBDDdDDBBDDW     ",
        "     WdDdBdBBBBBBBBBBDDDDBBDDW     ",
        "     KDdBBddBBBBBBBBBDDDBBBDKW     ",
        "    WDDBBBBddddBBBBBBBBBBBdDW      ",
        "  WKDDBBBBBdddddddBBBBBBBBDDW      ",
        " WKDDBBBBBBBBBBBdddBBBBBBBdDW      ",
        " WDDDDDDDDDDDDDDDDDDDDDDDDDDW      ",
    },
    // Frame 8
    {
        "                                   ",
        "                                   ",
        "                                   ",
        "                                   ",
        "                     WWWW          ",
        "                  WWKdDDKKW        ",
        "                WWDDDDDDDDDK       ",
        "               WKDDddBBBBdDDW      ",
        "              WDDdBDDBBddddDK      ",
        "             WDDdBBDDBDDDdDDDW     ",
        "             KDdBBBDDBDKKKKDDW     ",
        "            WDDBBBBBBBDKKKKDDDW    ",
        "            KDdBBBBBBBDgKKKDdDW    ",
        "           WDDBBBBBBBBDKKKKDBDDW   ",
        "          WDDBBBBBBBBBDKKKKDBDDW   ",
        "         WDDdBBBBBBBBBDdKKgDBBDK   ",
        "         KDDdBBBBBBBBBBDKKDdBdDK   ",
        "        WDDddBBBBBBBBBBDDDDdBBDK   ",
        "       WDDBdddBBBBBBBBBDDDdBBdDW   ",
        "      WDDdBBdddBBBBBBBBBDDdBBBDW   ",
        "    WWDDBBBBBddBBBBBBBBBBBBBBDDW   ",
        "   WKDDBBBBBBddddddBBBBBBBBBdDW    ",
        "  WDDDBBBBBBBBBddddddBBBBBBBDDW    ",
        "WWDDDBBBBBBBBBBBBBBBBBBBBBBBDDW    ",
        "WKDDDDDDDDDDDDDDDDDDDDDDDDDDDK     ",
    },
    // Frame 9
    {
        "                                   ",
        "                                   ",
        "                     WWWWW         ",
        "                     WWKWW         ",
        "                  WWDDDDDDDWW      ",
        "                WWdDDDdBdDDDKW     ",
        "               WdDDdddBBBBBDDW     ",
        "              WDDddBDDBBddDdDdW    ",
        "             WKDBBBBDDBDDddDDDW    ",
        "             WDDBBBBBBBDdKKKDDdW   ",
        "            WDDBBBBBBBBDKKKKDdDKW  ",
        "            WDdBBBBBBBBDdKKKDPDDW  ",
        "           WDDBBBBBBBBBDKKKKDPdDD  ",
        "           KDDBBBBBBBBBDdKKKDdBDD  ",
        "          WDDdBBBBBBBBBddKKKDBBDD  ",
        "          KDdddBBBBBBBBBDdKDDBBDD  ",
        "         KDdBddBBBBBBBBdDDDDdBBDD  ",
        "        WDDBBBdBBBBBBBBBBDDDBBBDD  ",
        "       WKDBBBBddBBBBBBBBBDDBBBdDD  ",
        "      WDDdBBBBBdBBBBBBBBBBBBBBDDW  ",
        "     WdDdBBBBBBdddddBBBBBBBBBBDKW  ",
        "    WKDdBBBBBBBBBBddddBBBBBBBBDK   ",
        "   WdDBBBBBBBBBBBBBBBBBBBBBBBdDW   ",
        " WWDDdBBBBBBBBBBBBBBBBBBBBBBBBDKW  ",
        " WdDDDDDDDDDDDDDDDDDDDDDDDDDDDDKW  ",
    },
};


static const uint32_t parrot_body_colors[10] = {
    0xFF8C8C, // Frame 0: Coral / Light Red
    0xFFD68C, // Frame 1: Orange-Yellow
    0x8CFF8C, // Frame 2: Lime Green
    0x8CFFFF, // Frame 3: Cyan
    0x8CB5FF, // Frame 4: Sky Blue
    0xD68CFF, // Frame 5: Purple / Lavender
    0xFF8CFF, // Frame 6: Magenta / Pink
    0xFF6BF7, // Frame 7: Hot Pink
    0xFF6BB5, // Frame 8: Deep Rose
    0xFF6B6B  // Frame 9: Red
};

void draw_party_parrot_frame_vbe(uint32_t start_x, uint32_t start_y, uint32_t frame, uint32_t scale) {
    if (scale == 0) {
        scale = 1;
    }
    frame = frame % 10;
    const uint32_t body_color = parrot_body_colors[frame];
    const uint32_t dark_outline = 0x213908;
    const uint32_t med_outline = 0x395A08;
    const uint32_t accent_green = 0x427B00;
    const uint32_t beak = 0x7B8C6B;
    const uint32_t cheek = 0xFF8C8C;


    for (uint32_t y = 0; y < 25; y++) {
        for (uint32_t x = 0; x < 35; x++) {
            char pixel = party_parrot_frames[frame][y][x];
            uint32_t color;

            switch (pixel) {
                case 'D': color = dark_outline; break;
                case 'd': color = med_outline; break;
                case 'g': color = accent_green; break;
                case 'K': color = beak; break;
                case 'B': color = body_color; break;
                case 'P': color = cheek; break;
                case 'W':
                case ' ':
                default: continue;
            }


            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    vbe_putpixel(start_x + x * scale + sx, start_y + y * scale + sy, color);
                }
            }
        }
    }
}

void draw_party_parrot_vbe(uint32_t start_x, uint32_t start_y) {
    draw_party_parrot_frame_vbe(start_x, start_y, 0, 4);
}

void demo_party_parrot_vbe(uint32_t start_x, uint32_t start_y, uint32_t scale, uint32_t loops, uint32_t delay_ms) {
    if (scale == 0) {
        scale = 4;
    }
    if (delay_ms == 0) {
        delay_ms = 50; // 50ms per frame as in the GIF (20 fps)
    }

    uint32_t width = 35 * scale;
    uint32_t height = 25 * scale;

    // Drain any leftover events (e.g. Enter key release from command prompt)
    while (ps2_has_data()) {
        ps2_keyboard_poll();
        (void)kbd_pop_event();
    }

    uint32_t current_loop = 0;
    while (loops == 0 || current_loop < loops) {
        for (uint32_t f = 0; f < 10; f++) {
            // Check for keypress to allow exiting demo early without blocking
            if (ps2_has_data()) {
                ps2_keyboard_poll();
                struct key_event event = kbd_pop_event();
                if (event.keycode != 0 && event.state.pressed) {
                    return;
                }
            }

            // Clear bounding box with black to prevent ghosting between frames
            vbe_fill_rect(start_x, start_y, width, height, 0x000000);

            // Draw current frame
            draw_party_parrot_frame_vbe(start_x, start_y, f, scale);

            // PIT sleep delay
            pit_sleep_ms(delay_ms);
        }
        current_loop++;
    }
}

