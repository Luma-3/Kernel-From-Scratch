#if defined(__linux__)
#error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#error "This code must be compiled with a 32-bit target."
#endif

#include <stdint.h>
#include <stddef.h>
#include "../boot/multiboot.h"
#include "../klibc/string/string.h"
#include "../klibc/math/math.h"
#include "../klibc/memory/mem.h"
#include "../drivers/display/display.h"
#include "../drivers/input/ps2/ps2.h"
#include "../terminal/terminal.h"

struct s_test {
  int a;
  long b;
};


// Version VGA - dessine une pomme avec des caractères
void draw_apple_vga(uint32_t start_x, uint32_t start_y) {
    // Tige (haut)
    terminal_put_pixel(start_x + 4, start_y + 0, COLOR_BROWN);
    terminal_put_pixel(start_x + 4, start_y + 1, COLOR_BROWN);
    
    // Feuille
    terminal_put_pixel(start_x + 5, start_y + 1, COLOR_GREEN);
    terminal_put_pixel(start_x + 6, start_y + 1, COLOR_GREEN);
    terminal_put_pixel(start_x + 6, start_y + 2, COLOR_GREEN);
    
    // Haut de la pomme - ligne 2
    terminal_put_pixel(start_x + 3, start_y + 2, COLOR_RED);
    terminal_put_pixel(start_x + 4, start_y + 2, COLOR_RED);
    terminal_put_pixel(start_x + 5, start_y + 2, COLOR_RED);
    
    // Ligne 3 - élargissement
    terminal_put_pixel(start_x + 2, start_y + 3, COLOR_RED);
    terminal_put_pixel(start_x + 3, start_y + 3, COLOR_LIGHT_RED);
    terminal_put_pixel(start_x + 4, start_y + 3, COLOR_RED);
    terminal_put_pixel(start_x + 5, start_y + 3, COLOR_RED);
    terminal_put_pixel(start_x + 6, start_y + 3, COLOR_RED);
    
    // Ligne 4 - partie la plus large
    terminal_put_pixel(start_x + 1, start_y + 4, COLOR_RED);
    terminal_put_pixel(start_x + 2, start_y + 4, COLOR_LIGHT_RED);
    terminal_put_pixel(start_x + 3, start_y + 4, COLOR_LIGHT_RED);
    terminal_put_pixel(start_x + 4, start_y + 4, COLOR_RED);
    terminal_put_pixel(start_x + 5, start_y + 4, COLOR_RED);
    terminal_put_pixel(start_x + 6, start_y + 4, COLOR_RED);
    terminal_put_pixel(start_x + 7, start_y + 4, COLOR_RED);
    
    // Ligne 5
    terminal_put_pixel(start_x + 1, start_y + 5, COLOR_RED);
    terminal_put_pixel(start_x + 2, start_y + 5, COLOR_RED);
    terminal_put_pixel(start_x + 3, start_y + 5, COLOR_RED);
    terminal_put_pixel(start_x + 4, start_y + 5, COLOR_RED);
    terminal_put_pixel(start_x + 5, start_y + 5, COLOR_RED);
    terminal_put_pixel(start_x + 6, start_y + 5, COLOR_RED);
    terminal_put_pixel(start_x + 7, start_y + 5, COLOR_RED);
    
    // Ligne 6 - rétrécissement
    terminal_put_pixel(start_x + 1, start_y + 6, COLOR_RED);
    terminal_put_pixel(start_x + 2, start_y + 6, COLOR_RED);
    terminal_put_pixel(start_x + 3, start_y + 6, COLOR_RED);
    terminal_put_pixel(start_x + 4, start_y + 6, COLOR_RED);
    terminal_put_pixel(start_x + 5, start_y + 6, COLOR_RED);
    terminal_put_pixel(start_x + 6, start_y + 6, COLOR_RED);
    terminal_put_pixel(start_x + 7, start_y + 6, COLOR_RED);
    
    // Ligne 7
    terminal_put_pixel(start_x + 2, start_y + 7, COLOR_RED);
    terminal_put_pixel(start_x + 3, start_y + 7, COLOR_RED);
    terminal_put_pixel(start_x + 4, start_y + 7, COLOR_RED);
    terminal_put_pixel(start_x + 5, start_y + 7, COLOR_RED);
    terminal_put_pixel(start_x + 6, start_y + 7, COLOR_RED);
    
    // Ligne 8 - bas de la pomme
    terminal_put_pixel(start_x + 3, start_y + 8, COLOR_RED);
    terminal_put_pixel(start_x + 4, start_y + 8, COLOR_RED);
    terminal_put_pixel(start_x + 5, start_y + 8, COLOR_RED);
}


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
            
            // Dessiner un bloc de scale x scale pixels pour chaque pixel du sprite
            for (uint32_t sy = 0; sy < scale; sy++) {
                for (uint32_t sx = 0; sx < scale; sx++) {
                    terminal_put_pixel(start_x + x * scale + sx, start_y + y * scale + sy, color);
                }
            }
        }
    }
}

void kmain(uint32_t mb_magic, uint32_t mb_info_addr) {
  if (mb_magic != MULTIBOOT1_BOOTLOADER_MAGIC) {
    return;
  }

  multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)mb_info_addr;
  display_boot_config_t display_boot_config = {0};

  terminal_writestring("Multiboot flags: ");
  terminal_put_char('\n');
  if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
      terminal_writestring(" - Memory information available\n");
  }
  terminal_write_int((int)(void *)&mbi->color_info.rgb);

  if ((mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) != 0u &&
      mbi->framebuffer_addr != 0u &&
      mbi->framebuffer_type == 1u &&
      (mbi->framebuffer_bpp == 24u || mbi->framebuffer_bpp == 32u)) {
      display_boot_config.framebuffer_addr = (uint32_t)mbi->framebuffer_addr;
      display_boot_config.framebuffer_pitch = mbi->framebuffer_pitch;
      display_boot_config.framebuffer_width = mbi->framebuffer_width;
      display_boot_config.framebuffer_height = mbi->framebuffer_height;
      display_boot_config.framebuffer_bpp = mbi->framebuffer_bpp;
      display_boot_config.color_info.rgb.framebuffer_blue_field_position = mbi->color_info.rgb.framebuffer_blue_field_position;
      display_boot_config.color_info.rgb.framebuffer_blue_mask_size = mbi->color_info.rgb.framebuffer_blue_mask_size;
      display_boot_config.color_info.rgb.framebuffer_green_field_position = mbi->color_info.rgb.framebuffer_green_field_position;
      display_boot_config.color_info.rgb.framebuffer_green_mask_size = mbi->color_info.rgb.framebuffer_green_mask_size;
      display_boot_config.color_info.rgb.framebuffer_red_field_position = mbi->color_info.rgb.framebuffer_red_field_position;
      display_boot_config.color_info.rgb.framebuffer_red_mask_size = mbi->color_info.rgb.framebuffer_red_mask_size;
    init_display(DRIVER_VBE, &display_boot_config);
    draw_apple_vbe(100, 100);
  }else {
    init_display(DRIVER_VGA, NULL);
    draw_apple_vga(10, 10);
  }

  init_terminal("Main Terminal");

  // init_display(preferred_driver, preferred_driver == DRIVER_VBE ? &display_boot_config : NULL);

  // char hello[] = "Hello, kernel World!\n";
  // if (k_strcmp(hello, "Hello World!\n") != 0) {
  //     terminal_writestring("k_strcmp worked correctly.\n");
  // }
  // if (k_strncmp(hello, "Hello, kernel", 5) == 0) {
  //     terminal_writestring("k_strncmp worked correctly.\n");
  // }
  // if (k_strrchr(hello, '\n') == (hello + (k_strrchr(hello, '\n') - hello))) {
  //   terminal_writestring("k_strrchr worked correctly.\n");
  // }
  // if (k_strchr(hello, ',') == (hello + (k_strchr(hello, ',') - hello))) {
  //   terminal_writestring("k_strchr worked correctly.\n");
  // }

  // const int a = -5;
  // const int b = 10;
  // if (k_abs(a) == 5) {
  //     terminal_writestring("k_abs worked correctly.\n");
  // }
  // if (k_max(a, b) == b) {
  //     terminal_writestring("k_max worked correctly.\n");
  // }
  // if (k_min(a, b) == a) {
  //     terminal_writestring("k_min worked correctly.\n");
  // }
  // const long x = -100L;
  // const long y = 200L;
  // if (k_labs(x) == 100L) {
  //     terminal_writestring("k_labs worked correctly.\n");
  // }
  // if (k_lmax(x, y) == y) {
  //     terminal_writestring("k_lmax worked correctly.\n");
  // }
  // if (k_lmin(x, y) == x) {
  //     terminal_writestring("k_lmin worked correctly.\n");
  // }

  // struct s_test test_struct = {42, 123456789L};
  // k_memset(&test_struct, 0, 4);
  // if (test_struct.a == 0 && test_struct.b == 123456789L) {
  //     terminal_writestring("k_memset worked correctly.\n");
  // }

  // terminal_writestring(hello);
}
