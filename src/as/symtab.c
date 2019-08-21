/**
 * @file symtab.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with the program symbol-table.
 * Contains functions for adding and finding symbols in the program symbol-table.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <section.h>
#include <symtab.h>


/**
 * @brief Frees the symbol table.
 * Frees the symbol table, freeing all of the symbols contained therein.
 * @param symtab The symbol table to free.
 * @warning Deletes all entries, and the table itself.
 */
void free_symbol_table(Symbol_Table* symtab) {
	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to free function\n");

		return;
	}

	for(size_t i = 0; i < symtab->n_entries; i++) {
		free(symtab->symbols[i].name);
	}

	free(symtab->symbols);
}


/**
 * @brief Prints a symbol table.
 *
 * This function prints all of the entries inside a symbol table.
 * @param symbol_table The symbol table to print.
 */
void print_symbol_table(Symbol_Table* const symbol_table) {
	if(!symbol_table) {
		fprintf(stderr, "Error: Invalid symbol table provided to print function\n");
		return;
	}

	for(size_t i = 0; i < symbol_table->n_entries; i++) {
		if(symbol_table->symbols[i].section) {
			// Allow for null symbol entry.
			printf("  Symbol: `%s`", symbol_table->symbols[i].name);
			printf(" in section `%s`", symbol_table->symbols[i].section->name);
			printf(" at `%#zx`\n", symbol_table->symbols[i].offset);
		}
	}
}


/**
 * @brief Adds a symbol to the symbol-table.
 *
 * Adds a symbol to the symbol table.
 * @param symtab A pointer to symbol table to add the symbol to.
 * @param name The name for the new symbol. This is the name by which it will be
 * referenced.
 * @param section A pointer to the section that contains this symbol.
 * @param offset The offset of the symbol being added in the section.
 * @warning @p symtab is modified in this function. The symbol entry array
 * is resized to accomodate the new symbol.
 */
Symbol* symtab_add_symbol(Symbol_Table* const symtab,
	char* name,
	Section* const section,
	const size_t offset) {

	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to add symbol function\n");
		return NULL;
	}

	if(!name) {
		fprintf(stderr, "Error: Invalid symbol provided to add symbol function\n");
		return NULL;
	}

	if(!section) {
		fprintf(stderr, "Error: Invalid section data provided to add symbol function\n");
		return NULL;
	}

	symtab->n_entries++;
	symtab->symbols = realloc(symtab->symbols, sizeof(Symbol) * symtab->n_entries);
	if(!symtab->symbols) {
		fprintf(stderr, "Error: Error resizing symbol table array\n");
		return NULL;
	}

	symtab->symbols[symtab->n_entries - 1].name = strdup(name);
	symtab->symbols[symtab->n_entries - 1].section = section;
	symtab->symbols[symtab->n_entries - 1].offset = offset;

#if DEBUG_SYMBOLS == 1
	printf("Debug Assembler: Added symbol `%s` in section `%s` at `%#zx`\n",
		name, section->name, offset);
#endif

	return &symtab->symbols[symtab->n_entries - 1];
}


/**
 * @brief Finds a symbol in the symbol-table.
 *
 * Finds a symbol contained in the symbol table.
 * @param symtab A pointer to symbol table to find the symbol in.
 * @param name The name of the symbol to search for.
 * @return A pointer to the first symbol matching the supplied name, or `NULL` if none exists.
 */
Symbol* symtab_find_symbol(Symbol_Table* const symtab,
	const char* name) {

	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to find symbol function\n");
		return NULL;
	}

	if(!name) {
		fprintf(stderr, "Error: Invalid name provided to find symbol function\n");
		return NULL;
	}

	int name_len = strlen(name);
	for(size_t i = 0; i < symtab->n_entries; i++) {
		if(strncmp(name, symtab->symbols[i].name, name_len) == 0) {
			return &symtab->symbols[i];
		}
	}

	return NULL;
}


/**
 * @brief Finds a symbol in the symbol-table.
 *
 * Finds a symbol contained in the symbol table.
 * @param symtab A pointer to symbol table to find the symbol in.
 * @param name The name of the symbol to search for.
 * @return A pointer to the first symbol matching the supplied name, or `NULL` if none exists.
 */
ssize_t symtab_find_symbol_index(Symbol_Table* const symtab,
	const char* name) {

	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to find symbol function\n");
		return -1;
	}

	if(!name) {
		fprintf(stderr, "Error: Invalid name provided to find symbol function\n");
		return -1;
	}

	int name_len = strlen(name);
	for(size_t i = 0; i < symtab->n_entries; i++) {
		if(strncmp(name, symtab->symbols[i].name, name_len) == 0) {
			return i;
		}
	}

	return -1;
}


/**
 * @brief Populates the ELF symbol table.
 *
 * This function parses through the program symbol table and encodes the necessary
 * ELF entities to write to the final assembled ELF file.
 * This function will add all of the necessary encoded entities to the symbol
 * table and string table sections.
 * @param sections A pointer to the section linked list.
 * @param symbol_table A pointer to the symbol table.
 * @warning This function modifies the sections.
 * @return A status entity indicating whether or not the pass was successful.
 */
Assembler_Status populate_symtab(Section* sections,
	Symbol_Table* symbol_table) {

	if(!sections) {
		fprintf(stderr, "Error: Invalid section data populating symbol table\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	if(!symbol_table) {
		fprintf(stderr, "Error: Invalid symbol table data populating symbol table\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	Section* strtab = find_section(sections, ".strtab");
	if(!strtab) {
		fprintf(stderr, "Error: Unable to locate `.strtab` section populating symbol table\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	Section* symtab = find_section(sections, ".symtab");
	if(!symtab) {
		fprintf(stderr, "Error: Unable to locate .symtab section populating symbol table\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}


	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity *added_entity = NULL;

	// Add the initial null byte to strtab as per ELF specification.
	Encoding_Entity *null_byte_entity = malloc(sizeof(Encoding_Entity));
	if(!null_byte_entity) {
		fprintf(stderr, "Error: Error allocating null byte symbol entity\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	null_byte_entity->n_reloc_entries = 0;
	null_byte_entity->reloc_entries = NULL;

	null_byte_entity->size = 1;
	null_byte_entity->data = malloc(1);
	if(!null_byte_entity->data) {
		fprintf(stderr, "Error: Error allocating null byte symbol entity data\n");

		// Cleanup.
		free(null_byte_entity);
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	null_byte_entity->data[0] = '\0';

	null_byte_entity->next = NULL;

	added_entity = section_add_encoding_entity(strtab, null_byte_entity);
	if(!added_entity) {
		// Error message should already be set.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

#if DEBUG_OUTPUT == 1
	printf("Debug Output: Added null byte to .strtab.\n");
#endif

	for(size_t i = 0; i < symbol_table->n_entries; i++) {
		// Add each symbol name to the string table, and each symbol entry to the
		// symbol table section.

		Elf32_Sym symbol_entry;
		symbol_entry.st_name = strtab->size;
		symbol_entry.st_value = symbol_table->symbols[i].offset;
		symbol_entry.st_size = 0;
		symbol_entry.st_info = 0;
		symbol_entry.st_other = 0;

		ssize_t shndx = 0;
		if(symbol_table->symbols[i].section) {
			// Take into account that we need to successfully parse the null symbol
			// entry. The null entry has zero for the section header index.
			shndx = find_section_index(sections,
				symbol_table->symbols[i].section->name);
		}

		// If we could not match the section index, abort.
		if(shndx == -1) {
			char missing_section_error_message[ERROR_MSG_MAX_LEN];
			sprintf(missing_section_error_message, "Unable to find section index for: `%s`",
				symbol_table->symbols[i].section->name);
			fprintf(stderr, "%s\n", missing_section_error_message);

			return ASSEMBLER_ERROR_MISSING_SECTION;
		}

		// Get the section index.
		symbol_entry.st_shndx = shndx;

#if DEBUG_OUTPUT == 1
	printf("Debug Output: Matched section index: `%i` for symbol name `%s`\n",
		symbol_entry.st_shndx, symbol_table->symbols[i].name);
#endif

		size_t symbol_entry_size = sizeof(Elf32_Sym);

		// Create an encoding entity for each symbol entry, this will be encoded
		// in the symbol table section during the writing of the section data.
		Encoding_Entity *symbol_entry_entity = malloc(sizeof(Encoding_Entity));
		if(!symbol_entry_entity) {
			fprintf(stderr, "Error: Error allocating symbol entity\n");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_entry_entity->n_reloc_entries = 0;
		symbol_entry_entity->reloc_entries = NULL;

		symbol_entry_entity->size = symbol_entry_size;
		symbol_entry_entity->data = malloc(symbol_entry_size);
		if(!symbol_entry_entity->data) {
			fprintf(stderr, "Error: Error allocating symbol entity data\n");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_entry_entity->data = memcpy(symbol_entry_entity->data,
			&symbol_entry, symbol_entry_size);

		symbol_entry_entity->next = NULL;

		added_entity = section_add_encoding_entity(symtab, symbol_entry_entity);
		if(!added_entity) {
			// Error message should already be set.
			return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
		}

#if DEBUG_OUTPUT == 1
	printf("Debug Output: Adding symbol: `%s` to .symtab at offset `0x%lx`\n",
		symbol_table->symbols[i].name, symtab->size);
#endif

		// Create an encoding entity for each symbol name, this will be encoded
		// in the string table during the writing of the section data.
		Encoding_Entity *symbol_name_entity = malloc(sizeof(Encoding_Entity));
		if(!symbol_name_entity) {
			fprintf(stderr, "Error: Error allocating symbol name entity\n");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_name_entity->n_reloc_entries = 0;
		symbol_name_entity->reloc_entries = NULL;

		size_t symbol_name_len = strlen(symbol_table->symbols[i].name) + 1;
		symbol_name_entity->size = symbol_name_len;
		symbol_name_entity->data = malloc(symbol_name_len);
		if(!symbol_name_entity->data) {
			fprintf(stderr, "Error: Error allocating symbol name entity data\n");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_name_entity->data = memcpy(symbol_name_entity->data,
			symbol_table->symbols[i].name, symbol_name_len);
		symbol_name_entity->data[symbol_name_len-1] = '\0';

		symbol_name_entity->next = NULL;

		added_entity = section_add_encoding_entity(strtab, symbol_name_entity);
		if(!added_entity) {
			// Error message should already be set.
			return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
		}

#if DEBUG_OUTPUT == 1
	printf("Debug Output: Added symbol name: `%s` to .strtab at offset `0x%lx`\n",
		symbol_table->symbols[i].name, strtab->size);
#endif
	}

	return ASSEMBLER_STATUS_SUCCESS;
}
