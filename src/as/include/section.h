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

#include <stddef.h>
#include <stdio.h>


typedef struct {
	char *symbol_name;
	size_t offset;
	uint32_t type;
} Reloc_Entry;


typedef struct _encoding_entity {
	size_t address;
	size_t size;
	uint8_t *data;
	size_t n_reloc_entries;
	Reloc_Entry *reloc_entries;
	struct _encoding_entity *next;
} Encoding_Entity;


typedef struct _section {
	char *name;
	size_t name_strtab_offset;
	size_t file_offset;
	size_t program_counter;
	uint32_t type;
	uint32_t flags;
	size_t size;
	size_t info;
	size_t link;
	Encoding_Entity *encoding_entities;
	struct _section *next;
} Section;


Section *create_section(char* name,
	const uint32_t type,
	const uint32_t flags);

Section* add_section(Section** section_list,
	Section* const section);

Section* find_section(Section* const section_list,
	const char* name);

ssize_t find_section_index(Section* const section_list,
	const char* name);

Encoding_Entity *section_add_encoding_entity(Section* const section,
	Encoding_Entity* const entity);

void free_section(Section* section);

#endif
