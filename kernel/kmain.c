#if defined(__linux__)
#error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#error "This code must be compiled with a 32-bit target."
#endif

#include <stdint.h>
#include "ps2.h"
#include "ascii.h"
#include <stddef.h>
#include "multiboot.h"
#include "string.h"
#include "math.h"
#include "mem.h"
#include "ps2.h"
#include "kpixel_art.h"
#include "vbe.h"


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
  }else {
    return;
  }

//   init_terminal("Main Terminal");
//   terminal_cursor_move(0, 200);

    ps2_init();
    while (1) {
        ps2_keyboard_poll(); // Poll the keyboard for key events and push them
                             // into the buffer

        struct key_event event =
            kdb_pop_event(); // Pop a key event from the buffer

        if (event.keycode != 0 &&
            event.state.pressed) { // If the event is not empty
            char ascii_char = keycode_to_ascii[event.keycode];
            if (event.state.shift) {
                // Handle shift for letters
                if (ascii_char >= 'a' && ascii_char <= 'z') {
                    ascii_char -= 32; // Convert to uppercase
                }
            }
            if (ascii_char) {
                // terminal_put_char(ascii_char); // Print the ASCII character
            }
        }
    }
}
