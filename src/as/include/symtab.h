/**
 * @file symtab.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Symbol table header.
 * Contains symbol table definitions and functions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef SYMTAB_H
#define SYMTAB_H 1

#include <section.h>

/**
 * @brief Symbol type.
 */
typedef struct {
	char* name;
	Section* section;
	size_t offset;
} Symbol;


/**
 * @brief Symbol table type.
 * Contains all of the individual symbols in a program.
 */
typedef struct {
	size_t n_entries;
	Symbol* symbols;
} Symbol_Table;


/**
 * @brief Prints a symbol table.
 *
 * This function prints all of the entries inside a symbol table.
 * @param symbol_table The symbol table to print.
 */
void print_symbol_table(Symbol_Table* const symbol_table);

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
	const size_t offset);

/**
 * @brief Finds a symbol in the symbol-table.
 *
 * Finds a symbol contained in the symbol table.
 * @param symtab A pointer to symbol table to find the symbol in.
 * @param name The name of the symbol to search for.
 * @return A pointer to the first symbol matching the supplied name,
 * or `NULL` if none exists.
 */
Symbol* symtab_find_symbol(Symbol_Table* const symtab,
	const char* name);

/**
 * @brief Finds a symbol in the symbol-table.
 *
 * Finds a symbol contained in the symbol table.
 * @param symtab A pointer to symbol table to find the symbol in.
 * @param name The name of the symbol to search for.
 * @return A pointer to the first symbol matching the supplied name,
 * or `NULL` if none exists.
 */
ssize_t symtab_find_symbol_index(Symbol_Table* const symtab,
	const char* name);


/**
 * @brief Frees the symbol table.
 * Frees the symbol table, freeing all of the symbols contained therein.
 * @param symtab The symbol table to free.
 * @warning Deletes all entries, and the table itself.
 */
void free_symbol_table(Symbol_Table* symtab);

#endif
