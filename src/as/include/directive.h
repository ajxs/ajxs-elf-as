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

/**
 * @brief Frees a directive.
 *
 * Frees a directive and the operands that it contains.
 * @param directive The directive to free.
 */
void free_directive(Directive *directive);

/**
 * @brief Gets the string representation of a directive type.
 *
 * This function returns the string representation of a directive type.
 * @param dir The directive to get the string for.
 * @returns a pointer to the string containing the directive name.
 */
const char *get_directive_string(Directive* const directive);

/**
 * @brief Parses a directive symbol.
 *
 * Parses a directive string, returning a value.
 * @param directive_symbol The directive string to parse.
 */
Directive_Type parse_directive_symbol(char* const directive_symbol);

/**
 * @brief Prints a directive.
 *
 * This function prints information about a directive entity.
 * @param dir The directive to print.
 */
void print_directive(Directive* const directive);

#endif
