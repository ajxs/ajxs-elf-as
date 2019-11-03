/**
 * @file encoding-entity.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for working with encoding entities.
 * Contains functions for working with encoding entitites.
 * @version 0.1
 * @date 2019-03-09
 */

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <encoding-entity.h>


/**
 * free_encoding_entity
 */
void free_encoding_entity(Encoding_Entity* entity)
{
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
