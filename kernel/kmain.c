#if defined(__linux__)
#  error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#  error "This code must be compiled with a 32-bit target."
#endif

#include <stddef.h>
#include <stdint.h>
#include "kernel.h"

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

void kinit(multiboot_info_t *mbi)
{
	read_elf_sec((struct elf_sec *)&mbi->u.elf_sec);
	init_gdt();
	vbe_init(mbi);
	ps2_init();
}

static struct s_kchecker check_integrity(uint32_t mb_magic, uint32_t mb_info_addr) {
	struct s_kchecker result = {0};

	result.serial = init_serial();
	result.magic = mb_magic == MULTIBOOT1_BOOTLOADER_MAGIC;
	if(result.magic) {
		multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)mb_info_addr;
		result.elf = (mbi->flags & MULTIBOOT_INFO_ELF_SHDR) != 0;
		result.info_mem = (mbi->flags & MULTIBOOT_INFO_MEMORY) != 0;
		result.vbe = vbe_detect(mbi);
	}
	return result;
}

static uint8_t display_integrity(struct s_kchecker status) {
	if (status.serial) {
		return FAILLURE;
	}
	if (!status.magic) {
		panic_serial("Invalid multiboot magic number");
		return FAILLURE;
	}

	if (!status.elf) {
		panic_serial("ELF section header table not present");
		return FAILLURE;
	}
	/*
	if (!status.info_mem) {
		return SUCCESS; TODO: kfs 3
	}
	*/
	if(!status.vbe) {
		panic_serial("VBE mode not present");
		return FAILLURE;
	}
	return SUCCESS;
}

void kmain(uint32_t mb_magic, uint32_t mb_info_addr)
{
	struct s_kchecker check = check_integrity(mb_magic, mb_info_addr);
	if (display_integrity(check)) {
		return; // Silent fail but no other way to report this error yet
	}

	multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)mb_info_addr;
	kinit(mbi);

	init_terminal("Main Terminal");
	term_refresh(0);

	backtrace(16);

	while (1) {
		ps2_keyboard_poll(); // Poll the keyboard for key events and push them
		term_poll();
	}
}
