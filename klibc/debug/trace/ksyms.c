
#include <stdint.h>
#include "ksyms.h"

static elf32_shdr *symtab = nullptr;
static elf32_shdr *strtab = nullptr;

void read_elf_sec(struct elf_sec *elf_sec)
{
	uint32_t sec_num = elf_sec->num;
	for (uint32_t i = 0; i < sec_num; i++) {
		elf32_shdr *shdr = (elf32_shdr *)(elf_sec->addr) + i;
		if (shdr->sh_type == SHT_SYMTAB) {
			symtab = shdr;
			break;
		}
	}

	strtab = (elf32_shdr *)elf_sec->addr + (symtab->sh_link);
}

const char *ksym_resolve(uint32_t addr, uint32_t *offset)
{
	uint32_t   nb_entry = symtab->sh_size / symtab->sh_entsize;
	Elf32_Sym *last = (Elf32_Sym *)symtab->sh_addr;

	for (uint32_t i = 0; i < nb_entry; i++) {
		Elf32_Sym *sym = (Elf32_Sym *)symtab->sh_addr + i;
		if (ELF_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_value <= addr) {
			last = last->st_value < sym->st_value ? sym : last;
		}
	}

	*offset = addr - last->st_value;

	return (const char *)strtab->sh_addr + last->st_name;
}
