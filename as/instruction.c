/**
 * @file instruction.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with instruction entities.
 * Contains functions for dealing with instruction entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <token.h>
#include <as.h>


/**
 * @brief Frees an instruction.
 *
 * Frees an instruction and the operands that it contains.
 * @param instruction The instruction to free.
 */
void free_instruction(Instruction *instruction) {
	if(!instruction) {
		set_error_message("Invalid instruction provided to free function.");
		return;
	}

	free_operand_sequence(&instruction->opseq);
}


/**
 * @brief Checks the operand length for an instruction.
 *
 * Checks that the number of operands provided in a statement matches the expected
 * amount for the statement's instruction opcode.
 * @param expected_operand_length The expected operand length.
 * @param instruction The instruction to check.
 * @return A boolean indicating whether the operand count matches the  expected count.
 */
bool instruction_check_operand_length(size_t expected_operand_length,
	Instruction *instruction) {

	if(!instruction) {
		set_error_message("Invalid instruction provided to check operand function.");
		return false;
	}

	if(instruction->opseq.n_operands != expected_operand_length) {
		return false;
	} else {
		return true;
	}
}
