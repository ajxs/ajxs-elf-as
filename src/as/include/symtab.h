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

typedef struct {
	char *name;
	Section *section;
	size_t offset;
} Symbol;


typedef struct {
	size_t n_entries;
	Symbol *symbols;
} Symbol_Table;


void print_symbol_table(Symbol_Table* const symbol_table);

Symbol* symtab_add_symbol(Symbol_Table* const symtab,
	char* name,
	Section* const section,
	const size_t offset);

Symbol* symtab_find_symbol(Symbol_Table* const symtab,
	const char* name);

ssize_t symtab_find_symbol_index(Symbol_Table* const symtab,
	const char* name);

void free_symbol_table(Symbol_Table* symtab);

#endif
