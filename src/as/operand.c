/**
 * @file operand.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for working with operands.
 * Contains functions for working with operand entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <operand.h>


/**
 * check_operand_count
 */
bool check_operand_count(const size_t expected_operand_length,
	const Operand_Sequence* opseq)
{
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
 * free_operand
 */
void free_operand(Operand* op)
{
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
 * free_operand_sequence
 */
void free_operand_sequence(Operand_Sequence* opseq)
{
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
 * print_operand
 */
void print_operand(const Operand* op)
{
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
 * print_operand_sequence
 */
void print_operand_sequence(const Operand_Sequence* opseq)
{
	printf("    Operand sequence: len: `%zu`\n", opseq->n_operands);
	for(size_t i = 0; i < opseq->n_operands; i++) {
		print_operand(&opseq->operands[i]);
	}
}
