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
	char **labels;
	Statement_Type type;
	union {
		Instruction instruction;
		Directive directive;
	};
	size_t line_num;
	struct statement *next;
} Statement;


ssize_t get_statement_size(Statement *statement);

void free_statement(Statement *statement);

void print_statement(Statement *statement);

#endif
