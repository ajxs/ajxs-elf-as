/**
 * @file elf.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Architecture specific ELF encoding functions.
 * Contains architecture specific functions for encoding specific ELF entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <elf.h>


/**
 * create_elf_header
 */
Assembler_Status create_elf_header(Elf32_Ehdr** elf_header)
{
	*elf_header = malloc(sizeof(Elf32_Ehdr));
	if(!elf_header) {
		fprintf(stderr, "Error allocating ELF Header.\n");
		free(elf_header);

		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	/** The ELF identity header. */
	char EI_IDENT[EI_NIDENT] = {
		0x7F, 'E', 'L', 'F',
		ELFCLASS32,
		ELFDATA2LSB,
		EV_CURRENT,
		ELFOSABI_SYSV,
		0,
		0, 0, 0, 0, 0, 0, 0
	};

	memcpy((*elf_header)->e_ident, &EI_IDENT, EI_NIDENT);
	(*elf_header)->e_type = ET_REL;
	(*elf_header)->e_machine = EM_MIPS;
	(*elf_header)->e_version = EV_CURRENT;
	(*elf_header)->e_entry = 0;
	(*elf_header)->e_phoff = 0;
	(*elf_header)->e_shoff = 0;
	(*elf_header)->e_flags = 0x90000400;
	(*elf_header)->e_ehsize = sizeof(Elf32_Ehdr);
	(*elf_header)->e_phentsize = 0;
	(*elf_header)->e_phnum = 0;
	(*elf_header)->e_shentsize = sizeof(Elf32_Shdr);
	(*elf_header)->e_shnum = 0;
	(*elf_header)->e_shstrndx = 0;

	return ASSEMBLER_STATUS_SUCCESS;
}
