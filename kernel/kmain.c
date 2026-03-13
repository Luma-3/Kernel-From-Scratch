#if defined(__linux__)
#error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#error "This code must be compiled with a 32-bit target."
#endif

#include "kernel.h"
#include "kpixel_art.h"
#include "multiboot.h"
#include "ps2.h"
#include "terminal.h"
#include "vbe.h"
#include <stddef.h>
#include <stdint.h>
#include "printk.h"
#include "../klibc/utility/printf.h"

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
    init_terminal("Secondary Terminal");

    // printf("\n");
    // printf("TEst printf\n");
    // printf("Hello, World! This is a test of the printf function in the kernel.\n");
    printf("Hex: 0x%X, Dec: %d, Unsigned: %u, Char: %c, String: %s\n", 255, -42, 42, 'A', "Test string"); 
    // printf("Pointer: %p\n", (void *)0xDEADBEEF);
    // printf("Percent sign: %%\n");
    // printf("Multiple args: %d %s %c %u\n", 123, "hello", 'X', 456u);
    // printf("Edge cases: %d %d %d\n", 0, INT32_MIN, INT32_MAX);
    // printf("Hex edge cases: 0x%X 0x%X\n", 0, 0xFFFFFFFF);
    // printf("Null string: %s\n", (char *)NULL);
    // printf("Null pointer: %p\n", (void *)NULL);
    // printf("Mixing types: %d %s %c %p\n", 42, "mixed", 'M', 0xABCD);

    // printf("Drawing pixel art...\n");

    printk(KERNEL_LOG_LEVEL_INFO, "Kernel initialized successfully.\n");

    ps2_init();
    while (1) {
        ps2_keyboard_poll(); // Poll the keyboard for key events and push them

        term_poll();
    }
}
