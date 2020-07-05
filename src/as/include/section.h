/**
 * @file section.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Section types header.
 * Contains section type definitions and functions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef SECTION_H
#define SECTION_H 1

#include <as.h>
#include <encoding-entity.h>
#include <stddef.h>
#include <stdio.h>


/**
 * @brief Section type.
 * Represents a file section.
 */
typedef struct _section {
	char* name;
	size_t name_strtab_offset;
	size_t file_offset;
	size_t program_counter;
	uint32_t type;
	uint32_t flags;
	size_t size;
	size_t info;
	size_t link;
	Encoding_Entity* encoding_entities;
	struct _section* next;
} Section;


/**
 * @brief Creates a section.
 *
 * Creates a section program entity.
 * @param name The name for the newly created section.
 * @param type The type of the newly created section.
 * @param flags The flags for the newly created section.
 * @return A pointer to the newly created section, or NULL if an error occurred.
 */
Assembler_Status create_section(Section** section,
	char* name,
	const uint32_t type,
	const uint32_t flags);

/**
 * @brief Adds a section.
 *
 * Adds a program section to the linked list of program sections.
 * @param section_list A pointer-to-pointer to the program section linked list.
 * @param section The section to add.
 * @return The added section, or NULL if an error occurred.
 */
Section* add_section(Section** section_list,
	Section* const section);

/**
 * @brief Finds a section by its name.
 *
 * Finds a program section, searching for one that matches the provided name.
 * @param sections A pointer to the program section linked list.
 * @param name The name of the section to search for.
 * @return A pointer to the section, or `NULL` if no matching section can be
 * found.
 */
Section* find_section(Section* const section_list,
	const char* name);

/**
 * @brief Finds a section's index by its name.
 *
 * Finds a program section's index in the sections linked list by its name.
 * @param sections A pointer to the program section linked list.
 * @param name The name of the section to search for.
 * @return The index of the found section in the list, or -1 if not found.
 */
ssize_t find_section_index(Section* const section_list,
	const char* name);

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
Encoding_Entity* section_add_encoding_entity(Section* const section,
	Encoding_Entity* const entity);

/**
 * @brief Frees a program section.
 *
 * Frees a program section and its contained encoded entities.  This will
 * free all of the encoded entities contained therein.
 * @param section A pointer to the section to be freed.
 * @warning This function will recursively free all encoded instruction and directive
 * entities contained in the section.
 */
void free_section(Section* section);

#endif
