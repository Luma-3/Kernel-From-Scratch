#include "keyevent.h"
#if defined(__linux__)
#error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#error "This code must be compiled with a 32-bit target."
#endif

#include "ascii.h"
#include "kpixel_art.h"
#include "multiboot.h"
#include "ps2.h"
#include "terminal.h"
#include "vbe.h"
#include <stddef.h>
#include <stdint.h>

// Function test for correctly print in terminal
void term_get_keyevent() {
    struct key_event event = kdb_pop_event();
    if (event.keycode == 0 || !event.state.pressed)
        return;

    char to_print = 0;

    if (event.state.shift == 1) {
        to_print = keycode_ascii_shift[event.keycode];
    } else {
        to_print = keycode_ascii[event.keycode];
    }

    if (to_print == 0)
        return;

    // terminal_put_char(to_print);
}

void kmain(uint32_t mb_magic, uint32_t mb_info_addr) {
    if (mb_magic != MULTIBOOT1_BOOTLOADER_MAGIC) {
        return;
    }

    // multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)mb_info_addr;
    // display_boot_config_t display_boot_config = {0};
    //
    // terminal_writestring("Multiboot flags: ");
    // terminal_put_char('\n');
    // if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
    //     terminal_writestring(" - Memory information available\n");
    // }
    // terminal_write_int((int)(void *)&mbi->color_info.rgb);
    //
    // if ((mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) != 0u &&
    //     mbi->framebuffer_addr != 0u && mbi->framebuffer_type == 1u &&
    //     (mbi->framebuffer_bpp == 24u || mbi->framebuffer_bpp == 32u)) {
    //     display_boot_config.framebuffer_addr =
    //     (uint32_t)mbi->framebuffer_addr;
    //     display_boot_config.framebuffer_pitch = mbi->framebuffer_pitch;
    //     display_boot_config.framebuffer_width = mbi->framebuffer_width;
    //     display_boot_config.framebuffer_height = mbi->framebuffer_height;
    //     display_boot_config.framebuffer_bpp = mbi->framebuffer_bpp;
    //     display_boot_config.color_info.rgb.framebuffer_blue_field_position =
    //         mbi->color_info.rgb.framebuffer_blue_field_position;
    //     display_boot_config.color_info.rgb.framebuffer_blue_mask_size =
    //         mbi->color_info.rgb.framebuffer_blue_mask_size;
    //     display_boot_config.color_info.rgb.framebuffer_green_field_position =
    //         mbi->color_info.rgb.framebuffer_green_field_position;
    //     display_boot_config.color_info.rgb.framebuffer_green_mask_size =
    //         mbi->color_info.rgb.framebuffer_green_mask_size;
    //     display_boot_config.color_info.rgb.framebuffer_red_field_position =
    //         mbi->color_info.rgb.framebuffer_red_field_position;
    //     display_boot_config.color_info.rgb.framebuffer_red_mask_size =
    //         mbi->color_info.rgb.framebuffer_red_mask_size;
    //     init_display(DRIVER_VBE, &display_boot_config);
    //     draw_apple_vbe(100, 100);
    //     draw_potato_vbe(200, 100);
    //     draw_axelote_on_bucket_vbe(300, 100);
    //     draw_saturn_vbe(400, 100);
    // } else {
    //     init_display(DRIVER_VGA, NULL);
    // }
    //
    // init_terminal("Main Terminal");
    // terminal_cursor_move(0, 200);
    //
    // ps2_init();
    // while (1) {
    //     ps2_keyboard_poll(); // Poll the keyboard for key events and push
    //                          // them into the buffer
    //     term_get_keyevent();
    multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)mb_info_addr;

    //   terminal_writestring("Multiboot flags: ");
    //   terminal_put_char('\n');
    if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
        //   terminal_writestring(" - Memory information available\n");
    }

    if (vbe_detect(mbi)) {
        vbe_init(mbi);
        draw_apple_vbe(100, 100);
        draw_potato_vbe(200, 100);
        draw_axelote_on_bucket_vbe(300, 100);
        draw_saturn_vbe(400, 100);
    } else {
        return;
    }

    init_terminal("Main Terminal");
    //   terminal_cursor_move(0, 200);

    ps2_init();
    while (1) {
        ps2_keyboard_poll(); // Poll the keyboard for key events and push them
                             // into the buffer

        term_get_keyevent();
    }
}
