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
void symtab_add_symbol(Symbol_Table *symtab,
	char *name,
	Section *section,
	size_t offset) {

	symtab->n_entries++;
	symtab->symbols = realloc(symtab->symbols, sizeof(Symbol) * symtab->n_entries);

	symtab->symbols[symtab->n_entries - 1].name = strdup(name);
	symtab->symbols[symtab->n_entries - 1].section = section;
	symtab->symbols[symtab->n_entries - 1].offset = offset;

#if DEBUG_SYMBOLS == 1
	printf("Debug Assembler: Added symbol `%s` in section `%s` at `%#zx`\n",
		name, section->name, offset);
#endif
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

	int name_len = strlen(name);
	for(size_t i = 0; i < symtab->n_entries; i++) {
		if(strncmp(name, symtab->symbols[i].name, name_len) == 0) {
			return &symtab->symbols[i];
		}
	}

	return NULL;
}
