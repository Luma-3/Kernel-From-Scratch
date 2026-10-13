#ifndef KSYMS_H
#define KSYMS_H

#include <stdint.h>

#define SHT_SYMTAB 2
#define SHT_STRTAB 3

#define STT_FUNC 2

struct elf_sec {
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx; // index of naming string table
} __attribute__((packed));

typedef struct {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} elf32_shdr;

typedef struct {
	uint32_t st_name;  // offset dans .strtab -> nom du symbole
	uint32_t st_value; // adresse du symbole
	uint32_t st_size;
	uint8_t	 st_info; // type (fonction, objet...) + binding
	uint8_t	 st_other;
	uint16_t st_shndx;
} Elf32_Sym;

#define ELF_ST_TYPE(info) ((info) & 0xF)

void read_elf_sec(struct elf_sec *elf_sec);

const char *ksym_resolve(uint32_t addr, uint32_t *offset);

#endif // !KSYMS_H
