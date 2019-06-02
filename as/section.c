/**
 * @file section.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with the program sections.
 * Contains functions for dealing with sections and their contained encoded data
 * and instruction entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>


/**
 * @brief Adds a section.
 *
 * Adds a program section to the linked list of program sections.
 * @param section_list A pointer-to-pointer to the program section linked list.
 * @param section The section to add.
 */
void add_section(Section **section_list,
	Section *section) {

	if(!*section_list) {
		*section_list = section;
		return;
	}

	Section *curr = *section_list;
	while(curr->next) {
		curr = curr->next;
	}

	curr->next = section;
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
Section *find_section(Section *section_list,
	const char *name) {

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
ssize_t find_section_index(Section *section_list,
	const char *name) {

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
 */
void section_add_encoding_entity(Section *section,
	Encoding_Entity *entity) {

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Adding encoded entity to `%s`...\n", section->name);
#endif

	if(!section->encoding_entities) {
		// If there is no current head of the encoded entities linked list.
		section->encoding_entities = entity;
		section->size += entity->size;
	} else {
		// If there is an encoded entities linked list, append the new entity
		// to the end of the list.
		Encoding_Entity *current_entity = section->encoding_entities;
		section->size += entity->size;

		while(current_entity->next) {
			current_entity = current_entity->next;
		}

		current_entity->next = entity;
	}
}


/**
 * @brief Frees an encoded entity.
 *
 * Frees an encoded directive or instruction entity and its contained structures.
 * This will free any linked entities recursively.
 * @param entity A pointer to the entity to be freed.
 * @warning This function will recursively free any linked entities.
 */
void free_encoding_entity(Encoding_Entity *entity) {
	if(!entity) {
		return;
	}

	if(entity->reloc_entries) {
		free(entity->reloc_entries);
	}

	if(entity->next) {
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
		return;
	}

	if(section->next) {
		free_section(section->next);
	}

	if(section->encoding_entities) {
#if DEBUG_ASSEMBLER == 1
		printf("Debug Assembler: Freeing entities for section `%s`...\n", section->name);
#endif
		free_encoding_entity(section->encoding_entities);
	}

	free(section);
}
