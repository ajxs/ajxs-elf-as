/**
 * @file directive.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Statement header.
 * Contains directive definitions and specific functions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef DIRECTIVE_H
#define DIRECTIVE_H 1

#include <operand.h>
#include <symtab.h>


typedef enum {
	DIRECTIVE_UNKNOWN,
	DIRECTIVE_ALIGN,
	DIRECTIVE_ASCII,
	DIRECTIVE_ASCIZ,
	DIRECTIVE_BSS,
	DIRECTIVE_BYTE,
	DIRECTIVE_DATA,
	DIRECTIVE_FILL,
	DIRECTIVE_GLOBAL,
	DIRECTIVE_LONG,
	DIRECTIVE_SHORT,
	DIRECTIVE_SIZE,
	DIRECTIVE_SKIP,
	DIRECTIVE_SPACE,
	DIRECTIVE_STRING,
	DIRECTIVE_TEXT,
	DIRECTIVE_WORD
} Directive_Type;


typedef struct {
	Directive_Type type;
	Operand_Sequence opseq;
} Directive;

void print_directive(Directive dir);

void free_directive(Directive *directive);

const char *get_directive_string(Directive dir);

Directive_Type parse_directive_symbol(char *directive_symbol);

Encoding_Entity *encode_directive(Symbol_Table *symtab,
	Directive *directive,
	size_t program_counter);

#endif
