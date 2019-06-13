/**
 * @file statement.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with program statements.
 * Contains functions for working with parsed statement entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <as.h>


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
		set_error_message("Invalid statement provided to free function.");
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
		set_error_message("Invalid operand sequence provided to check operand function.");
		return false;
	}

	if(opseq->n_operands != expected_operand_length) {
		return false;
	} else {
		return true;
	}
}
