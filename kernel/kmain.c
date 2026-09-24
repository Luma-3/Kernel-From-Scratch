#if defined(__linux__)
#  error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#  error "This code must be compiled with a 32-bit target."
#endif

#include <stddef.h>
#include <stdint.h>

#include "i386/gdt/gdt.h"
#include "kdebug.h"
#include "kpixel_art.h"
#include "multiboot.h"
#include "printf.h"
#include "ps2.h"
#include "serial.h"
#include "terminal.h"
#include "vbe.h"
#include "klibc/debug/trace/ksyms.h"

void kmain(uint32_t mb_magic, uint32_t mb_info_addr)
{
	if (init_serial()) {
		return; // Silent fail but no other way to report this error yet
	}

	if (mb_magic != MULTIBOOT1_BOOTLOADER_MAGIC) {
		panic_serial("Invalid multiboot magic number");
		return;
	}
	multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)mb_info_addr;

	if (!(mbi->flags & MULTIBOOT_INFO_ELF_SHDR)) {
		panic_serial("ELF section header table not present");
		return;
	}

	init_gdt();

	//   terminal_writestring("Multiboot flags: ");
	//   terminal_put_char('\n');
	if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
		//   terminal_writestring(" - Memory information available\n");
	}

	if (vbe_detect(mbi)) {
		vbe_init(mbi);
		// draw_apple_vbe(100, 100);
		// draw_potato_vbe(200, 100);
		// draw_axelote_on_bucket_vbe(300, 100);
		// draw_saturn_vbe(400, 100);
	}
	else {
		panic_serial("VBE not detected");
		return;
	}

	init_terminal("Main Terminal");
	term_refresh(0);
	read_elf_sec((struct elf_sec *)&mbi->u.elf_sec);

	ps2_init();

	while (1) {
		ps2_keyboard_poll(); // Poll the keyboard for key events and push
							 // them

		term_poll();
	}
}
