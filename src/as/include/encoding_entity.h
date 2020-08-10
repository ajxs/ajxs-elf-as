/**
 * @file encoding-entity.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Encoding_Entity types header.
 * Contains encoding entity type definitions and functions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef ENCODING_ENTITY_H
#define ENCODING_ENTITY_H 1

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>


/**
 * Relocation Entry type.
 */
typedef struct {
	char* symbol_name;
	size_t offset;
	uint32_t type;
} Reloc_Entry;


/**
 * Assembler encoding entity type.
 * This type represents a single encoded entity. This can be either an instruction,
 * or an encoded directive, of variable length.
 */
typedef struct _encoding_entity {
	size_t address;
	size_t size;
	uint8_t* data;
	size_t n_reloc_entries;
	Reloc_Entry* reloc_entries;
	struct _encoding_entity* next;
} Encoding_Entity;


/**
 * @brief Frees an encoded entity.
 *
 * Frees an encoded directive or instruction entity and its contained structures.
 * This will free any linked entities recursively.
 * @param entity A pointer to the entity to be freed.
 * @warning This function will recursively free any linked entities.
 */
void free_encoding_entity(Encoding_Entity* entity);

#endif
