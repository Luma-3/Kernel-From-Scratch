#if defined(__linux__)
#error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#error "This code must be compiled with a 32-bit target."
#endif

#include "kpixel_art.h"
#include "multiboot.h"
#include "printf.h"
#include "printk.h"
#include "ps2.h"
#include "terminal.h"
#include "vbe.h"
#include <stddef.h>
#include <stdint.h>

void kmain(uint32_t mb_magic, uint32_t mb_info_addr) {
    if (mb_magic != MULTIBOOT1_BOOTLOADER_MAGIC) {
        return;
    }
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
    term_refresh(0);

    init_terminal("Secondary Terminal");

    printf("Hex: 0x%X, Dec: %d, Unsigned: %u, Char: %c, String: %s\n", 255, -42,
           42, 'A', "Test string");

    ps2_init();
    while (1) {
        ps2_keyboard_poll(); // Poll the keyboard for key events and push them

        term_poll();
    }
}