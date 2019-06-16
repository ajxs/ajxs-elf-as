/**
 * @file macro.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for expansion of macros and pseudo-instructions.
 * Contains functions for expansion of MIPS architecture specific
 * macros and pseudo-instructions.
 * These functions are invoked prior to the first assembler pass. After all of
 * the statements have been parsed, expansion of macros can begin. This needs to
 * be done at this point to ensure that the program counter position
 * is correctly calculated.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <macro.h>
#include <statement.h>

/**
 * @brief Expands a `la` or `li` psuedo-instruction.
 *
 * This function expands these pseudo-instructions, replacing them with the correct
 * instruction statements in order to perform the prescribed operation.
 * The resulting instructions are highly dependent on the format of the statements,
 * with different formats of operands yielding different results.
 * @param macro The pseudo-instruction statement.
 * @warning @macro is modified in this function. Additional statements may be
 * appended to the end of this statement.
 */
Expand_Macro_Result_Status expand_macro_la(Statement *macro) {
#if DEBUG_MACRO == 1
	printf("Debug Macro: Expanding `LA` pseudo-instruction...\n");
#endif

	if(!check_operand_count(2, &macro->instruction.opseq)) {
		// Check operand length is equal to 2, if not abort.
		fprintf(stderr, "Operand count mismatch for `LA` pseudo-instruction.\n");
		return EXPAND_MACRO_FAILURE;
	}

	if(macro->instruction.opseq.operands[1].type == OPERAND_TYPE_SYMBOL) {
		// If the immediate operand is a symbol the pseudo-instruction will be
		// represented by an `LUI` instruction wtih an additional `ORI` instruction
		// appended. These will load the symbol address into memory.
		// The `LUI` instruction will load the MSB of the symbol address, and an ORI
		// instruction loading the LSB of the address.

		// Create the expansion instruction.
		// This instruction will be appended to the original instruction.
		Statement *expansion = malloc(sizeof(Statement));
		if(!expansion) {
			fprintf(stderr, "Error allocating statement for macro expansion.\n");
			return EXPAND_MACRO_FAILURE;
		}

		expansion->n_labels = 0;
		expansion->labels = NULL;

		expansion->type = STATEMENT_TYPE_INSTRUCTION;
		expansion->instruction.opcode = OPCODE_ORI;
		expansion->instruction.opseq.n_operands = 3;
		expansion->instruction.opseq.operands = malloc(sizeof(Operand) * 3);
		if(!expansion->instruction.opseq.operands) {
			fprintf(stderr, "Error allocating operand sequence for macro expansion.\n");
			return EXPAND_MACRO_FAILURE;
		}

		expansion->instruction.opseq.operands[0] = macro->instruction.opseq.operands[0];
		expansion->instruction.opseq.operands[1] = macro->instruction.opseq.operands[0];

		expansion->instruction.opseq.operands[2].type = OPERAND_TYPE_SYMBOL;
		expansion->instruction.opseq.operands[2].symbol =
			strdup(macro->instruction.opseq.operands[1].symbol);

		// The expansion immediate operand will contain the lower half of the
		// immediate value.
		expansion->instruction.opseq.operands[2].flags.mask = OPERAND_MASK_LOW;

		// Set the expanded second instruction to point at the original next instruction.
		// This ensures that the instruction is properly 'inserted'.
		expansion->next = macro->next;

		// Update the original instruction.
		macro->instruction.opcode = OPCODE_LUI;

		// The macro immediate operand will contain the higher half of the
		// immediate value.
		macro->instruction.opseq.operands[1].flags.mask = OPERAND_MASK_HIGH;

		// Point the next pointer of the original instruction at the expansion instruction.
		macro->next = expansion;
	} else if(macro->instruction.opseq.operands[1].type == OPERAND_TYPE_NUMERIC_LITERAL) {
		// If the Immediate Operand is a numeric literal.

		if(macro->instruction.opseq.operands[1].numeric_literal > 0xFFFF) {
			// If the immediate value is above 16bits in size, it will be expanded to use
			// an LUI instruction loading the MSB of the numeric literal, and an ORI
			// instruction loading the LSB.

			// Create the expansion instruction to store the `ORI` instruction.
			Statement *expansion = malloc(sizeof(Statement));
			if(!expansion) {
				fprintf(stderr, "Error allocating statement for macro expansion.\n");
				return EXPAND_MACRO_FAILURE;
			}

			expansion->n_labels = 0;
			expansion->labels = NULL;

			expansion->type = STATEMENT_TYPE_INSTRUCTION;
			expansion->instruction.opcode = OPCODE_ORI;

			// Use the modified operands from the original pseudo-instruction.
			expansion->instruction.opseq.n_operands = 3;
			expansion->instruction.opseq.operands = malloc(sizeof(Operand) * 3);
			if(!expansion->instruction.opseq.operands) {
				free(expansion);

				fprintf(stderr, "Error allocating operand sequence for macro expansion.\n");
				return EXPAND_MACRO_FAILURE;
			}

			expansion->instruction.opseq.operands[0] = macro->instruction.opseq.operands[0];
			expansion->instruction.opseq.operands[1] = macro->instruction.opseq.operands[0];

			expansion->instruction.opseq.operands[2].type = OPERAND_TYPE_NUMERIC_LITERAL;

			// Truncate the immediate value to 16bits.
			expansion->instruction.opseq.operands[2].numeric_literal =
				macro->instruction.opseq.operands[1].numeric_literal & 0xFFFF;

			// Set the expanded second instruction to point at the original next instruction.
			// This ensures that the instruction is properly 'inserted'.
			expansion->next = macro->next;

			// Update the original instruction to be an LUI instruction.
			macro->instruction.opcode = OPCODE_LUI;

			// Use upper 16bits.
			macro->instruction.opseq.operands[1].numeric_literal =
				(macro->instruction.opseq.operands[1].numeric_literal >> 16) & 0xFFFF;
			macro->next = expansion;
		} else {
			// If the instruction is below or equal to 16-bits in length, an `ADDIU`
			// instruction is used to represent the pseudo-instruction.

			// The original instruction is modified to use the `ADDIU` opcode.
			macro->instruction.opcode = OPCODE_ADDIU;
			macro->instruction.opseq.n_operands = 3;
			macro->instruction.opseq.operands = realloc(macro->instruction.opseq.operands,
				sizeof(Operand) * 3);
			if(!macro->instruction.opseq.operands) {
				fprintf(stderr, "Error allocating operand sequence for macro expansion.\n");
				return EXPAND_MACRO_FAILURE;
			}

			macro->instruction.opseq.operands[2] = macro->instruction.opseq.operands[1];
			macro->instruction.opseq.operands[1] = macro->instruction.opseq.operands[0];
			macro->instruction.opseq.operands[0].type = OPERAND_TYPE_REGISTER;
			macro->instruction.opseq.operands[0].reg = REGISTER_$ZERO;
		}
	} else {
		// If the original expanded instruction uses any other kind of immediate
		// operand type throw an error and abort.
		return EXPAND_MACRO_FAILURE;
	}

	return EXPAND_MACRO_SUCCESS;
}


/**
 * @brief Expands a branch delay instruction.
 *
 * This function expands any branching instructions to insert a NOP in the branch
 * delay slot. This funcionality is as-per GCC's functionality.
 * @param macro The branching instruction statement.
 * @warning @p macro is modified in this function.
 */
Expand_Macro_Result_Status expand_branch_delay(Statement *macro) {
#if DEBUG_MACRO == 1
	printf("Debug Macro: Expanding branch delay macro...\n");
#endif

	// Create the expansion instruction which will store the inserted `NOP`.
	Statement *expansion = malloc(sizeof(Statement));
	if(!expansion) {
		fprintf(stderr, "Error allocating statement for macro expansion.\n");
		return EXPAND_MACRO_FAILURE;
	}

	expansion->n_labels = 0;
	expansion->labels = NULL;

	expansion->type = STATEMENT_TYPE_INSTRUCTION;
	expansion->instruction.opcode = OPCODE_NOP;
	expansion->instruction.opseq.n_operands = 0;
	expansion->instruction.opseq.operands = NULL;

	// Set the expanded second instruction to point at the original next of the macro.
	expansion->next = macro->next;

	// Update the original instruction.
	macro->next = expansion;

	return EXPAND_MACRO_SUCCESS;
}


/**
 * @brief Expands a `move` pseudo-instruction.
 *
 * This function expands the MIPS `move` pseudo-instruction. This instruction is
 * analogous to adding a number to $0 and storing the result in a register. So
 * the instruction is converted to this form.
 * @param macro The `move` instruction statement.
 * @warning @p macro is modified in this function.
 */
Expand_Macro_Result_Status expand_macro_move(Statement *macro) {
#if DEBUG_MACRO == 1
	printf("Debug Macro: Expanding `MOVE` pseudo-instruction...\n");
#endif

	if(!check_operand_count(2, &macro->instruction.opseq)) {
		fprintf(stderr, "Operand count mismatch for `MOVE` pseudo-instruction.\n");
		return EXPAND_MACRO_FAILURE;
	}

	// The `MOVE` pseudo-instruction is analogous to an ADD instruction between
	// one register and $zero so we replace the opcode with an `ADD`, and then
	// add a final operand referencing the $zero register.
	macro->instruction.opcode = OPCODE_ADD;
	macro->instruction.opseq.n_operands = 3;
	macro->instruction.opseq.operands =
		realloc(macro->instruction.opseq.operands, sizeof(Operand) * 3);
	if(!macro->instruction.opseq.operands) {
		fprintf(stderr, "Error allocating operand sequence for macro expansion.\n");
		return EXPAND_MACRO_FAILURE;
	}

	macro->instruction.opseq.operands[2].type = OPERAND_TYPE_REGISTER;
	macro->instruction.opseq.operands[2].reg = REGISTER_$ZERO;

	return EXPAND_MACRO_SUCCESS;
}


/**
 * @brief Expands all of the macro statements in the program.
 *
 * This function iterates through all of the program statements, checking whether
 * each one is a macro. Any macros encountered will be 'expanded'. This process
 * typically involves the modification of the macro statement itself, as well as
 * potentially appending further statements to it. This is accomplished by adding
 * a new link to the `statements` linked list.
 * @param statements The linked list of parsed statements.
 * @warning @p statements is modified by this function.
 */
Assembler_Process_Result expand_macros(Statement *statements) {
	Statement *curr = statements;

	Expand_Macro_Result_Status macro_process_status;

	while(curr) {
		if(curr->type == STATEMENT_TYPE_INSTRUCTION) {
			switch(curr->instruction.opcode) {
				case OPCODE_LA:
				case OPCODE_LI:
					macro_process_status = expand_macro_la(curr);
					break;
				case OPCODE_BAL:
				case OPCODE_BEQ:
				case OPCODE_BEQZ:
				case OPCODE_BGEZ:
				case OPCODE_BLEZ:
				case OPCODE_BNE:
				case OPCODE_JAL:
				case OPCODE_JR:
					macro_process_status = expand_branch_delay(curr);
					break;
				case OPCODE_MOVE:
					macro_process_status = expand_macro_move(curr);
					break;
				default:
					break;
			}
		}

		if(macro_process_status != EXPAND_MACRO_SUCCESS) {
			// If an error was encountered, return here.
			// The error will have been set in the expansion function.
			return ASSEMBLER_ERROR_MACRO_EXPANSION;
		}

		curr = curr->next;
	}


	return ASSEMBLER_PROCESS_SUCCESS;
}
