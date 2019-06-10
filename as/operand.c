/**
 * @file operand.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with operand entities.
 * Contains functions for dealing with operand entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <token.h>


/**
 * @brief Frees an operand pointer.
 *
 * Frees an operand pointer. Checks if the operand type is dynamically allocated,
 * freeing it if necessary.
 * @param op A pointer to the operand to free.
 */
void free_operand(Operand *op) {
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
	for(size_t i=0; i<opseq->n_operands; i++) {
		free_operand(&opseq->operands[i]);
	}
}
