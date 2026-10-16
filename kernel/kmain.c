#if defined(__linux__)
#  error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#  error "This code must be compiled with a 32-bit target."
#endif

#include <stddef.h>
#include <stdint.h>
#include "kernel/kernel.h"

#include "arch/i386/gdt/gdt.h"
#include "klibc/debug/kdebug.h"
#include "demo/kpixel_art.h"
#include "boot/multiboot.h"
#include "klibc/utility/printf.h"
#include "drivers/input/ps2/ps2.h"
#include "drivers/serial/serial.h"
#include "terminal/terminal.h"
#include "drivers/display/display.h"
#include "drivers/display/vbe/vbe.h"
#include "klibc/debug/trace/ksyms.h"
#include "terminal/command.h"
#include "arch/i386/mm/mminfo_ram.h"
#include "arch/i386/mm/pmm.h"
#include "arch/i386/mm/buddy.h"


#include "arch/i386/mm/pmm.h"
#include "arch/i386/mm/vmm.h"
#include "arch/i386/mm/buddy.h"
#include "klibc/utility/printf.h"

void test_buddy_allocator(void) {
	printf("\n--- TEST DU BUDDY ALLOCATOR & DES ZONES ---\n");
	printf("[1] Allocation d'un bloc d'ordre 3 (8 pages = 32 Ko) dans ZONE_NORMAL...\n");
	struct page *p1 = alloc_pages(ZONE_NORMAL, 3);
	if (p1) {
		printf("    Succes -> PFN: %d, Zone: %d, Phys: 0x%llx\n",
		       (int)page_to_pfn(p1), (int)p1->zone_id, page_to_phys(p1));
	}

	printf("[2] Allocation d'un bloc d'ordre 1 (2 pages = 8 Ko) dans ZONE_DMA...\n");
	struct page *p2 = alloc_pages(ZONE_DMA, 1);
	if (p2) {
		printf("    Succes -> PFN: %d, Zone: %d, Phys: 0x%llx\n",
		       (int)page_to_pfn(p2), (int)p2->zone_id, page_to_phys(p2));
	}

	print_buddy_stats();

	printf("[3] Liberation des blocs alloues (Test du Coalescing par zone)...\n");
	free_pages(p1, 3);
	free_pages(p2, 1);

	printf("[4] Statistiques apres fusion complete :\n");
	print_buddy_stats();
}

void kinit(multiboot_info_t *mbi)
{
	read_elf_sec((struct elf_sec *)&mbi->u.elf_sec);
	init_gdt();
	display_init(mbi);
	init_pmm(mbi);
	init_vmm();
	ps2_init();
	init_command_system();
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
	#if SERIAL_SUPPORT
	if (status.serial) {
		return FAILLURE;
	}
	#endif
	if (!status.magic) {
		panic_serial("Invalid multiboot magic number");
		return FAILLURE;
	}

	if (!status.elf) {
		panic_serial("ELF section header table not present");
		return FAILLURE;
	}
	
	if (!status.info_mem) {
		panic_serial("Memory information not present");
		return FAILLURE;
	}
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
	memory_detection_info_t mem_info = detect_memory(mbi);

	init_terminal("Main Terminal");
	term_refresh(0);
	printf("Total usable RAM: %llu bytes\n", mem_info.total_usable_ram);
	printf("Maximum physical address: %llu bytes\n", mem_info.max_physical_addr);

	print_buddy_stats();
	test_buddy_allocator();
	backtrace(16);
	(void)term_read_line("trucOs>");
	while (1) {
		ps2_keyboard_poll(); // Poll the keyboard for key events and push them
		uint8_t *line = term_read_line("trucOs>");
		if(line) {
			execute_command(line);
		}
	}
}
