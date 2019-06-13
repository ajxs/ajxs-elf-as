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
