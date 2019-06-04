#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <elf.h>


/**
 * @brief Creates the ELF file header.
 *
 * This function creates an ELF executable file header specific for this
 * particular architecture.
 * @return A pointer to the created ELF header.
 */
Elf32_Ehdr *create_elf_header(void) {
	Elf32_Ehdr *elf_header = malloc(sizeof(Elf32_Ehdr));

	char EI_IDENT[EI_NIDENT] = {
		0x7F, 'E', 'L', 'F',
		ELFCLASS32,
		ELFDATA2LSB,
		EV_CURRENT,
		ELFOSABI_SYSV,
		0,
		0, 0, 0, 0, 0, 0, 0
	};

	memcpy(elf_header->e_ident, &EI_IDENT, EI_NIDENT);
	elf_header->e_type = ET_REL;
	elf_header->e_machine = EM_MIPS;
	elf_header->e_version = EV_CURRENT;
	elf_header->e_entry = 0;
	elf_header->e_phoff = 0;
	elf_header->e_shoff = 0;
	elf_header->e_flags = 0x90000400;
	elf_header->e_ehsize = sizeof(Elf32_Ehdr);
	elf_header->e_phentsize = 0;
	elf_header->e_phnum = 0;
	elf_header->e_shentsize = sizeof(Elf32_Shdr);
	elf_header->e_shnum = 0;
	elf_header->e_shstrndx = 0;

	return elf_header;
}
