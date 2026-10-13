
#include <stdint.h>
#include "ksyms.h"
#include "kdebug.h"

static elf32_shdr *symtab = nullptr;
static elf32_shdr *strtab = nullptr;

void read_elf_sec(struct elf_sec *elf_sec)
{
	uint32_t sec_num = elf_sec->num;
	for (uint32_t i = 0; i < sec_num; ++i) {

		elf32_shdr *shdr = (elf32_shdr *)(elf_sec->addr) + i;
		if (shdr->sh_type == SHT_SYMTAB) {
			symtab = shdr;
			break;
		}
	}
	if (symtab == nullptr) {
		panic_serial("No symbol table found in ELF sections");
		return;
	}

	strtab = (elf32_shdr *)elf_sec->addr + (symtab->sh_link);
}

const char *ksym_resolve(uint32_t addr, uint32_t *offset)
{
	if (symtab == nullptr || strtab == nullptr) {
		return "Symbol table corupted";
	}

	uint32_t   nb_entry = symtab->sh_size / symtab->sh_entsize;
	Elf32_Sym *last = (Elf32_Sym *)((uintptr_t)symtab->sh_addr);

	for (uint32_t i = 0; i < nb_entry; i++) {

		Elf32_Sym *sym = (Elf32_Sym *)symtab->sh_addr + i;

		if (ELF_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_value <= addr &&
			sym->st_value > last->st_value) {
			last = sym;
		}
	}

	if (ELF_ST_TYPE(last->st_info) != STT_FUNC || last->st_value > addr) {
		*offset = 0;
		return "Unknown";
	}

	*offset = addr - last->st_value;

	return (const char *)strtab->sh_addr + last->st_name;
}
