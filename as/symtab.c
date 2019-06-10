/**
 * @file symtab.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with the program symbol-table.
 * Contains functions for adding and finding symbols in the program symbol-table.
 * @version 0.1
 * @date 2019-03-09
 */

#include <as.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


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
Symbol *symtab_add_symbol(Symbol_Table *symtab,
	char *name,
	Section *section,
	size_t offset) {

	if(!symtab) {
		set_error_message("Error adding symbol: Invalid symbol table data.\n");
		return NULL;
	}

	if(!name) {
		set_error_message("Error adding symbol: Invalid symbol name.\n");
		return NULL;
	}

	if(!section) {
		set_error_message("Error adding symbol: Invalid section data.\n");
		return NULL;
	}

	symtab->n_entries++;
	symtab->symbols = realloc(symtab->symbols, sizeof(Symbol) * symtab->n_entries);
	if(!symtab->symbols) {
		set_error_message("Error adding symbol: Unable to resize symbol table array.\n");
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
Symbol *symtab_find_symbol(Symbol_Table *symtab,
	char *name) {

	if(!symtab) {
		// @ERROR
		return NULL;
	}

	if(!name) {
		// @ERROR
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
ssize_t symtab_find_symbol_index(Symbol_Table *symtab,
	char *name) {

	if(!symtab) {
		// @ERROR
		return -1;
	}

	if(!name) {
		// @ERROR
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
 * @brief Frees the symbol table.
 * Frees the symbol table, freeing all of the symbols contained therein.
 * @param symtab The symbol table to free.
 * @warning Deletes all entries, and the table itself.
 */
void free_symbol_table(Symbol_Table *symtab) {

	if(!symtab) {
		// @ERROR
		return;
	}

	for(size_t i = 0; i < symtab->n_entries; i++) {
		free(symtab->symbols[i].name);
	}

	free(symtab->symbols);
}
