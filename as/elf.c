#include <stdlib.h>
#include <elf.h>
#include <as.h>


Elf32_Shdr *encode_section_header(Section *section) {
	Elf32_Shdr *section_header = malloc(sizeof(Elf32_Shdr));
	if(!section_header) {
		// @ERROR
		return NULL;
	}

	section_header->sh_name = section->name_strtab_offset;
	section_header->sh_type = section->type;
	section_header->sh_flags = section->flags;
	section_header->sh_addr = 0;
	section_header->sh_offset = section->file_offset;
	section_header->sh_size = section->size;
	section_header->sh_link = section->link;
	section_header->sh_info = section->info;
	section_header->sh_addralign = 0;
	section_header->sh_entsize = 0;

	if(section->type == SHT_SYMTAB) {
		section_header->sh_entsize = sizeof(Elf32_Sym);
	} else if(section->type == SHT_REL) {
		section_header->sh_entsize = sizeof(Elf32_Rel);
	}

	return section_header;
}
