#include <as.h>


Section *create_section(char *name,
	uint32_t type,
	uint32_t flags);

Section *add_section(Section **section_list,
	Section *section);

Section *find_section(Section *section_list,
	const char *name);

ssize_t find_section_index(Section *section_list,
	const char *name);

Encoding_Entity *section_add_encoding_entity(Section *section,
	Encoding_Entity *entity);

void free_section(Section *section);
