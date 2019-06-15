#include <as.h>


void print_symbol_table(Symbol_Table *symbol_table);
Symbol *symtab_find_symbol(Symbol_Table *symtab,
	char *label);

Symbol *symtab_add_symbol(Symbol_Table *symtab,
	char *name,
	Section *section,
	size_t offset);

Symbol *symtab_find_symbol(Symbol_Table *symtab,
	char *name);

ssize_t symtab_find_symbol_index(Symbol_Table *symtab,
	char *name);

void free_symbol_table(Symbol_Table *symtab);
