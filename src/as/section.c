/**
 * @file section.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with the program sections.
 * Contains functions for dealing with sections and their contained encoded data
 * and instruction entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <encoding_entity.h>
#include <section.h>


/**
 * create_section
 */
Assembler_Status create_section(Section** section,
	const char* name,
	const uint32_t type,
	const uint32_t flags)
{
	*section = malloc(sizeof(Section));
	if(!section) {
		fprintf(stderr, "Error: Error allocating section\n");

		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	(*section)->name = name;
	(*section)->name_strtab_offset = 0;
	(*section)->program_counter = 0;
	(*section)->file_offset = 0;
	(*section)->size = 0;
	(*section)->flags = flags;
	(*section)->link = 0;
	(*section)->info = 0;
	(*section)->type = type;
	(*section)->encoding_entities = NULL;
	(*section)->next = NULL;

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * add_section
 */
Section* add_section(Section** section_list,
	Section* section)
{
	if(!section_list) {
		fprintf(stderr, "Error: Invalid section list provided to add section function\n");
		return NULL;
	}

	if(!section) {
		fprintf(stderr, "Error: Invalid section provided to add section function\n");
		return NULL;
	}

	if(!*section_list) {
		*section_list = section;
		return section;
	}

	Section* curr = *section_list;
	while(curr->next) {
		curr = curr->next;
	}

	curr->next = section;
	return section;
}


/**
 * find_section
 */
Section* find_section(const Section* section_list,
	const char* name)
{
	if(!section_list) {
		fprintf(stderr, "Error: Invalid section list provided to find section function.\n");
		return NULL;
	}

	const int name_len = strlen(name);
	const Section* curr = section_list;

	if(!curr) {
		return NULL;
	}

	while(curr) {
		if(strncmp(curr->name, name, name_len) == 0) {
			return (Section*)curr;
		}

		curr = curr->next;
	}

	return NULL;
}


/**
 * find_section_index
 */
ssize_t find_section_index(const Section* section_list,
	const char* name)
{
	if(!section_list) {
		return -1;
	}

	const int name_len = strlen(name);
	const Section* curr = section_list;
	ssize_t idx = 0;
	while(curr) {
		if(strncmp(curr->name, name, name_len) == 0) {
			return idx;
		}

		idx++;
		curr = curr->next;
	}

	return -1;
}


/**
 * section_add_encoding_entity
 */
Encoding_Entity* section_add_encoding_entity(Section* section,
	const Encoding_Entity* entity)
{
	if(!section) {
		fprintf(stderr, "Error: Invalid section provided to add entity function.\n");

		return NULL;
	}

	if(!entity) {
		fprintf(stderr, "Error: Invalid entity provided to add entity function.\n");

		return NULL;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Adding encoded entity to `%s`...\n", section->name);
#endif

	if(!section->encoding_entities) {
		// If there is no current head of the encoded entities linked list.
		section->encoding_entities = (Encoding_Entity*)entity;
		section->size += entity->size;

		return section->encoding_entities;
	}

	// If there is an encoded entities linked list, append the new entity
	// to the end of the list.
	Encoding_Entity* current_entity = section->encoding_entities;
	section->size += entity->size;

	while(current_entity->next != NULL) {
		current_entity = current_entity->next;
	}

	current_entity->next = (Encoding_Entity*)entity;
	return current_entity->next;
}


/**
 * free_section
 */
void free_section(Section* section)
{
	if(!section) {
		fprintf(stderr, "Error: Attempting to free NULL section.\n");

		return;
	}

	if(section->next != NULL) {
		free_section(section->next);
	}

	if(section->encoding_entities) {
#if DEBUG_ASSEMBLER == 1
		printf("Debug Assembler: Freeing entities for section `%s`...\n", section->name);
#endif
		free_encoding_entity(section->encoding_entities);
	} else {
#if DEBUG_ASSEMBLER == 1
		printf("Debug Assembler: No entities to cleanup for section `%s`...\n",
			section->name);
#endif
	}

	free(section);
}


/**
 * initialise_sections
 */
Assembler_Status initialise_sections(Section** sections)
{
	/** Holds the success status of internal operations. */
	Assembler_Status status = ASSEMBLER_STATUS_SUCCESS;
	/** Value to track the results of adding the newly created sections. */
	Section* added_section = NULL;

	// The invididual section entities.
	Section* section_null = NULL;
	Section* section_text = NULL;
	Section* section_text_rel = NULL;
	Section* section_data = NULL;
	Section* section_data_rel = NULL;
	Section* section_bss = NULL;
	Section* section_symtab = NULL;
	Section* section_shstrtab = NULL;
	Section* section_strtab = NULL;


	// The section header data will be filled as the sections are serialised.
	status = create_section(&section_null, "\0", SHT_NULL, 0);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `NULL` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	status = create_section(&section_text, ".text", SHT_PROGBITS,
		SHF_ALLOC | SHF_EXECINSTR);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `TEXT` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	// The ELF man page suggests that the flags for relocatable sections are
	// set to SHF_ALLOC, but from readelf we can see that gcc itself
	// seems to use `SHF_INFO_LINK`.
	// Refer to: 'http://www.sco.com/developers/gabi/2003-12-17/ch4.sheader.html'
	// for the undocumented flags.
	status = create_section(&section_text_rel, ".rel.text", SHT_REL, SHF_INFO_LINK);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `.rel.text` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	status = create_section(&section_data, ".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `.data` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	status = create_section(&section_data_rel, ".rel.data", SHT_REL, SHF_INFO_LINK);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `.rel.data` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	status = create_section(&section_bss, ".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `.bss` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	status = create_section(&section_symtab, ".symtab", SHT_SYMTAB, SHF_ALLOC);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `.symtab` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	status = create_section(&section_shstrtab, ".shstrtab", SHT_STRTAB, SHF_ALLOC);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `.shstrtab` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}

	status = create_section(&section_strtab, ".strtab", SHT_STRTAB, 0);
	if(!get_status(status)) {
		fprintf(stderr, "Error: creating `.strtab` section");

		goto SECTION_INIT_ALLOC_FAILURE;
	}


	added_section = add_section(sections, section_null);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_text);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_text_rel);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_data);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_data_rel);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_bss);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_symtab);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_shstrtab);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_strtab);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	// Find the index of the string table section, so we can link the symbol
	// table section to the string table section.
	ssize_t section_strtab_index = find_section_index(*sections, ".strtab");
	if(section_strtab_index == -1) {
		fprintf(stderr, "Error: Unable to find `.strtab` section index.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_symtab->link = section_strtab_index;

	// Find the index of the data section, so we can link its relevant relocation
	// entry section to it.
	ssize_t section_data_index = find_section_index(*sections, ".data");
	if(section_data_index == -1) {
		fprintf(stderr, "Error: Unable to find `.data` section index.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_data_rel->info = section_data_index;


	// Find the index of the text section, so we can link its relevant relocation
	// entry section to it.
	ssize_t section_text_index = find_section_index(*sections, ".text");
	if(section_text_index == -1) {
		fprintf(stderr, "Error: Unable to find `.text` section index.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_text_rel->info = section_text_index;


	// Find the index of the symbol table section, so we can link the program data
	// sections to it.
	ssize_t section_symtab_index = find_section_index(*sections, ".symtab");
	if(section_symtab_index == -1) {
		fprintf(stderr, "Error: Unable to find `.symtab` section index.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_data_rel->link = section_symtab_index;
	section_text_rel->link = section_symtab_index;

	return ASSEMBLER_STATUS_SUCCESS;

SECTION_INIT_ALLOC_FAILURE:
	// Free any allocated sections.
	if(section_null) {
		free_section(section_null);
	}

	if(section_text) {
		free_section(section_text);
	}

	if(section_text_rel) {
		free_section(section_text_rel);
	}

	if(section_data) {
		free_section(section_data);
	}

	if(section_data_rel) {
		free_section(section_data_rel);
	}

	if(section_bss) {
		free_section(section_bss);
	}

	if(section_symtab) {
		free_section(section_symtab);
	}

	if(section_shstrtab) {
		free_section(section_shstrtab);
	}

	if(section_strtab) {
		free_section(section_strtab);
	}

	return ASSEMBLER_ERROR_BAD_ALLOC;
}
