/**
 * @file statement.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with program statements.
 * Contains functions for working with parsed statement entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <instruction.h>
#include <directive.h>
#include <statement.h>


/**
 * free_statement
 */
void free_statement(Statement* statement)
{
	if(!statement) {
		fprintf(stderr, "Error: Invalid statement provided to free function.\n");

		return;
	}

	if(statement->next) {
		free_statement(statement->next);
	}

	for(size_t i = 0; i < statement->n_labels; i++) {
		free(statement->labels[i]);
	}

	free(statement->labels);

	if(statement->type == STATEMENT_TYPE_DIRECTIVE) {
		free_directive(&statement->directive);
	} else if(statement->type == STATEMENT_TYPE_INSTRUCTION) {
		free_instruction(&statement->instruction);
	}

	free(statement);
}


/**
 * print_directive
 */
void print_directive(Directive* const directive)
{
	const char* directive_name = get_directive_string(directive);
	printf("  Directive: Type: `%s`\n", directive_name);
	if(directive->opseq.n_operands > 0) {
		print_operand_sequence(&directive->opseq);
	}
}


/**
 * @brief Prints a statement.
 *
 * This function prints information about a statement entity.
 * @param statement The statement to print.
 */
void print_statement(Statement* const statement)
{
	if(!statement) {
		fprintf(stderr, "Error: Invalid statement provided to print function\n");

		return;
	}

	printf("Debug Parser: Statement: Type: `%i`\n", statement->type);
	if(statement->n_labels > 0) {
		printf("  Labels: `%zu`:\n", statement->n_labels);
		for(size_t i=0; i<statement->n_labels; i++) {
			printf("    Label: `%s`\n", statement->labels[i]);
		}
	}

	if(statement->type == STATEMENT_TYPE_DIRECTIVE) {
		print_directive(&statement->directive);
	} else if(statement->type == STATEMENT_TYPE_INSTRUCTION) {
		print_instruction(statement->instruction);
	}
}
