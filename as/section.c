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
#include <section.h>


void free_encoding_entity(Encoding_Entity* entity);


/**
 * @brief Creates a section.
 *
 * Creates a section program entity.
 * @param name The name for the newly created section.
 * @param type The type of the newly created section.
 * @param flags The flags for the newly created section.
 * @return A pointer to the newly created section, or NULL if an error occurred.
 */
Section *create_section(char* name,
	const uint32_t type,
	const uint32_t flags) {

	Section *section = malloc(sizeof(Section));
	if(!section) {
		fprintf(stderr, "Error: Error allocating section.\n");
		return NULL;
	}

	section->name = name;
	section->name_strtab_offset = 0;
	section->program_counter = 0;
	section->file_offset = 0;
	section->size = 0;
	section->flags = flags;
	section->link = 0;
	section->info = 0;
	section->type = type;
	section->encoding_entities = NULL;
	section->next = NULL;

	return section;
}


/**
 * @brief Adds a section.
 *
 * Adds a program section to the linked list of program sections.
 * @param section_list A pointer-to-pointer to the program section linked list.
 * @param section The section to add.
 * @return The added section, or NULL if an error occurred.
 */
Section* add_section(Section** section_list,
	Section* const section) {

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

	Section *curr = *section_list;
	while(curr->next) {
		curr = curr->next;
	}

	curr->next = section;
	return section;
}


/**
 * @brief Finds a section by its name.
 *
 * Finds a program section, searching for one that matches the provided name.
 * @param sections A pointer to the program section linked list.
 * @param name The name of the section to search for.
 * @return A pointer to the section, or `NULL` if no matching section can be
 * found.
 */
Section *find_section(Section* const section_list,
	const char* name) {

	if(!section_list) {
		fprintf(stderr, "Error: Invalid section list provided to find section function.\n");
		return NULL;
	}

	int name_len = strlen(name);
	Section *curr = section_list;

	if(!curr) {
		return NULL;
	}

	while(curr) {
		if(strncmp(curr->name, name, name_len) == 0) {
			return curr;
		}

		curr = curr->next;
	}

	return NULL;
}


/**
 * @brief Finds a section's index by its name.
 *
 * Finds a program section's index in the sections linked list by its name.
 * @param sections A pointer to the program section linked list.
 * @param name The name of the section to search for.
 * @return The index of the found section in the list, or -1 if not found.
 */
ssize_t find_section_index(Section* const section_list,
	const char* name) {

	if(!section_list) {
		return -1;
	}

	int name_len = strlen(name);
	Section *curr = section_list;
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
 * @brief Adds an encoded entity to a section.
 *
 * Adds an encoded instruction or directive entity to a program section.
 * The entity will be added to the end of the end of the encoded entities linked
 * list.
 * @param section A pointer to the program section to add the encoded
 * entity to.
 * @param entity The encoded entity to add to the section.
 * @return Returns the added encoding entity or NULL in the case of error.
 */
Encoding_Entity *section_add_encoding_entity(Section* const section,
	Encoding_Entity* const entity) {

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
		section->encoding_entities = entity;
		section->size += entity->size;

		return section->encoding_entities;
	}

	// If there is an encoded entities linked list, append the new entity
	// to the end of the list.
	Encoding_Entity *current_entity = section->encoding_entities;
	section->size += entity->size;

	while(current_entity->next != NULL) {
		current_entity = current_entity->next;
	}

	current_entity->next = entity;
	return current_entity->next;
}


/**
 * @brief Frees an encoded entity.
 *
 * Frees an encoded directive or instruction entity and its contained structures.
 * This will free any linked entities recursively.
 * @param entity A pointer to the entity to be freed.
 * @warning This function will recursively free any linked entities.
 */
void free_encoding_entity(Encoding_Entity* entity) {
	if(!entity) {
		fprintf(stderr, "Error: Attempting to free NULL encoded entity.\n");

		return;
	}

	if(entity->data != NULL) {
		free(entity->data);
	}

	if(entity->reloc_entries != NULL) {
		free(entity->reloc_entries);
	}

	if(entity->next != NULL) {
		free_encoding_entity(entity->next);
	}

	free(entity);
}


/**
 * @brief Frees a program section.
 *
 * Frees a program section and its contained encoded entities.  This will
 * free all of the encoded entities contained therein.
 * @param section A pointer to the section to be freed.
 * @warning This function will recursively free all encoded instruction and directive
 * entities contained in the section.
 */
void free_section(Section *section) {
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
 * @brief Creates and initialises the executable sections.
 *
 * This function creates all of the sections required to generate a relocatable
 * ELF file. This will create all of the sections, as well as their relocation
 * entry sections.
 * Creates a linked list of the sections.
 * @param sections A pointer-to-pointer to the section data.
 * @return A status result object showing the result of the process.
 */
Assembler_Status initialise_sections(Section **sections) {
	/** Used for holding the error messages printed by this function. */
	const char* error_message = NULL;
	/** Holds the number of chars written to stderr in the error handler. */

	// The invididual section entities.
	Section *section_null = NULL;
	Section *section_text = NULL;
	Section *section_text_rel = NULL;
	Section *section_data = NULL;
	Section *section_data_rel = NULL;
	Section *section_bss = NULL;
	Section *section_symtab = NULL;
	Section *section_shstrtab = NULL;
	Section *section_strtab = NULL;


	// The section header data will be filled as the sections are serialised.
	section_null = create_section("\0", SHT_NULL, 0);
	if(!section_null) {
		error_message = "Error creating `NULL` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	section_text = create_section(".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
	if(!section_text) {
		error_message = "Error creating `.text` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	// The ELF man page suggests that the flags for relocatable sections are
	// set to SHF_ALLOC, but from readelf we can see that gcc itself
	// seems to use `SHF_INFO_LINK`.
	// Refer to: 'http://www.sco.com/developers/gabi/2003-12-17/ch4.sheader.html'
	// for the undocumented flags.
	section_text_rel = create_section(".rel.text", SHT_REL, SHF_INFO_LINK);
	if(!section_text_rel) {
		error_message = "Error creating `.rel.text` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	section_data = create_section(".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
	if(!section_data) {
		error_message = "Error creating `.data` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	section_data_rel = create_section(".rel.data", SHT_REL, SHF_INFO_LINK);
	if(!section_data_rel) {
		error_message = "Error creating `.rel.data` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	section_bss = create_section(".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE);
	if(!section_bss) {
		error_message = "Error creating `.bss` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	section_symtab = create_section(".symtab", SHT_SYMTAB, SHF_ALLOC);
	if(!section_symtab) {
		error_message = "Error creating `.symtab` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	section_shstrtab = create_section(".shstrtab", SHT_STRTAB, SHF_ALLOC);
	if(!section_shstrtab) {
		error_message = "Error creating `.shstrtab` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}

	section_strtab = create_section(".strtab", SHT_STRTAB, 0);
	if(!section_strtab) {
		error_message = "Error creating `.strtab` section.";
		goto SECTION_INIT_ALLOC_FAILURE;
	}


	/** Value to track the results of adding the newly created sections. */
	Section *added_section = NULL;

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
	// Print error message.
	fprintf(stderr, "Error: %s\n", error_message);

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
