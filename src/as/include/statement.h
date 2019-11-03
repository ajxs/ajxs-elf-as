/**
 * @file statement.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Statement header.
 * Contains statement definitions and specific functions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef STATEMENT_H
#define STATEMENT_H 1

#include <as.h>
#include <directive.h>
#include <instruction.h>
#include <operand.h>
#include <stdbool.h>
#include <stddef.h>


typedef enum {
	STATEMENT_TYPE_EMPTY,
	STATEMENT_TYPE_DIRECTIVE,
	STATEMENT_TYPE_INSTRUCTION,
} Statement_Type;


typedef struct statement {
	size_t n_labels;
	char** labels;
	Statement_Type type;
	union {
		Instruction instruction;
		Directive directive;
	};
	size_t line_num;
	struct statement* next;
} Statement;


/**
 * @brief Gets the size of a statement entity.
 *
 * Returns the number of bytes required to encode a specific statement.
 * @param statement The statement to encode.
 * @returns The number of bytes required to encode the entity, or -1 if an error
 * occurred.
 */
Assembler_Status get_statement_size(Statement* statement,
	size_t* statement_size);

/**
 * @brief Frees a statement entity.
 *
 * This function frees a statement entity, it will also free all statement
 * entities linked to this statement in its list recursively.
 * @param statement The statement entity to free.
 * @warning This function frees all statements in the statement linked list
 * recursively.
 */
void free_statement(Statement* statement);

void print_statement(Statement* statement);

#endif
