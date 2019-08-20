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


/**
 * @brief The type of a directive.
 *
 * Indicates the type of the directive. These are non-architecture specific.
 */
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

void free_directive(Directive *directive);

const char *get_directive_string(Directive* const directive);

Directive_Type parse_directive_symbol(char* const directive_symbol);

void print_directive(Directive* const directive);

#endif
