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
 * @brief Frees an operand pointer.
 *
 * Frees an operand pointer. Checks if the operand type is dynamically allocated,
 * freeing it if necessary.
 * @param op A pointer to the operand to free.
 */
void free_operand(Operand *op) {
	if(!op) {
		fprintf(stderr, "Error: Invalid operand provided to free function.\n");

		return;
	}

	if(op->type == OPERAND_TYPE_STRING_LITERAL) {
		free(op->string_literal);
	} else if(op->type == OPERAND_TYPE_SYMBOL) {
		free(op->symbol);
	}
}


/**
 * @brief Frees an operand sequence.
 *
 * Frees a sequence of operands,
 * @param opseq A pointer to the operand sequence to free.
 */
void free_operand_sequence(Operand_Sequence *opseq) {
	if(!opseq) {
		fprintf(stderr, "Error: Invalid operand sequence provided to free function.\n");

		return;
	}

	for(size_t i=0; i < opseq->n_operands; i++) {
		free_operand(&opseq->operands[i]);
	}

	free(opseq->operands);
}


/**
 * @brief Frees a statement entity.
 *
 * This function frees a statement entity, it will also free all statement
 * entities linked to this statement in its list recursively.
 * @param statement The statement entity to free.
 * @warning This function frees all statements in the statement linked list
 * recursively.
 */
void free_statement(Statement *statement) {
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
 * @brief Checks the operand count for an operand sequence.
 *
 * Checks that the number of operands provided in a statement matches the expected
 * amount for the statement's instruction opcode.
 * @param expected_operand_length The expected operand length.
 * @param opseq The operand sequence to check.
 * @return A boolean indicating whether the operand count matches the expected count.
 */
bool check_operand_count(size_t expected_operand_length,
	Operand_Sequence *opseq) {

	if(!opseq) {
		fprintf(stderr, "Error: Invalid operand sequence provided to check operand function.\n");

		return false;
	}

	if(opseq->n_operands != expected_operand_length) {
		return false;
	} else {
		return true;
	}
}



/**
 * @brief Prints an instruction operand.
 *
 * This function prints information about an instruction operand.
 * @param op The operand to print information about.
 */
void print_operand(Operand* const op) {
	if(op->type == OPERAND_TYPE_NUMERIC_LITERAL) {
		printf("      Operand: Numeric Literal: `%i`", op->numeric_literal);
	} else if(op->type == OPERAND_TYPE_STRING_LITERAL) {
		printf("      Operand: String Literal: `%s`", op->string_literal);
	} else if(op->type == OPERAND_TYPE_SYMBOL) {
		printf("      Operand: Symbol Reference: `%s`", op->symbol);
	} else if(op->type == OPERAND_TYPE_REGISTER) {
		printf("      Operand: Register: `%i`", op->reg);
	} else {
		printf("      Unknown Operand Type");
	}

	if(op->offset != 0) {
		printf(" Offset: `%i`", op->offset);
	}

	if(op->flags.mask != OPERAND_MASK_NONE) {
		printf(" Mask: `%i`", op->flags.mask);
	}

	if(op->flags.shift != 0) {
		printf(" Shift: `%i`", op->flags.shift);
	}

	printf("\n");
}


/**
 * @brief Prints an operand sequence.
 *
 * This function prints an operand sequence entity, printing each operand.
 * @param opseq The operand sequence to print.
 */
void print_operand_sequence(Operand_Sequence* const opseq) {
	printf("    Operand sequence: len: `%zu`\n", opseq->n_operands);
	for(size_t i = 0; i < opseq->n_operands; i++) {
		print_operand(&opseq->operands[i]);
	}
}


/**
 * @brief Prints a directive.
 *
 * This function prints information about a directive entity.
 * @param dir The directive to print.
 */
void print_directive(Directive* const directive) {
	const char *directive_name = get_directive_string(directive);
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
void print_statement(Statement* const statement) {
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
