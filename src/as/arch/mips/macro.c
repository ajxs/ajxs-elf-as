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
 * expand_macro_la
 */
Assembler_Status expand_macro_la(Statement* macro) {
#if DEBUG_MACRO == 1
	printf("Debug Macro: Expanding `LA` pseudo-instruction\n");
#endif

	if(!check_operand_count(2, &macro->instruction.opseq)) {
		// Check operand length is equal to 2, if not abort.
		fprintf(stderr, "Operand count mismatch for `LA` pseudo-instruction\n");
		return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
	}

	if(macro->instruction.opseq.operands[1].type == OPERAND_TYPE_SYMBOL) {
		// If the immediate operand is a symbol the pseudo-instruction will be
		// represented by an `LUI` instruction wtih an additional `ORI` instruction
		// appended. These will load the symbol address into memory.
		// The `LUI` instruction will load the MSB of the symbol address, and an ORI
		// instruction loading the LSB of the address.

		// Create the expansion instruction.
		// This instruction will be appended to the original instruction.
		Statement* expansion = malloc(sizeof(Statement));
		if(!expansion) {
			fprintf(stderr, "Error: Error allocating statement for macro expansion\n");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		expansion->n_labels = 0;
		expansion->labels = NULL;

		expansion->type = STATEMENT_TYPE_INSTRUCTION;
		expansion->instruction.opcode = OPCODE_ORI;
		expansion->instruction.opseq.n_operands = 3;
		expansion->instruction.opseq.operands = malloc(sizeof(Operand) * 3);
		if(!expansion->instruction.opseq.operands) {
			free(expansion);

			fprintf(stderr, "Error: Error allocating operand sequence for macro expansion\n");
			return ASSEMBLER_ERROR_BAD_ALLOC;
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
			Statement* expansion = malloc(sizeof(Statement));
			if(!expansion) {
				fprintf(stderr, "Error allocating statement for macro expansion\n");
				return ASSEMBLER_ERROR_BAD_ALLOC;
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

				fprintf(stderr, "Error: Error allocating operand sequence for macro expansion\n");
				return ASSEMBLER_ERROR_BAD_ALLOC;
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
				fprintf(stderr, "Error: Error allocating operand sequence for macro expansion\n");
				return ASSEMBLER_ERROR_BAD_ALLOC;
			}

			macro->instruction.opseq.operands[2] = macro->instruction.opseq.operands[1];
			macro->instruction.opseq.operands[1] = macro->instruction.opseq.operands[0];
			macro->instruction.opseq.operands[0].type = OPERAND_TYPE_REGISTER;
			macro->instruction.opseq.operands[0].reg = REGISTER_$ZERO;
		}
	} else {
		// If the original expanded instruction uses any other kind of immediate
		// operand type throw an error and abort.
		return ASSEMBLER_ERROR_BAD_OPERAND_TYPE;
	}

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * expand_branch_delay
 */
Assembler_Status expand_branch_delay(Statement* macro) {
#if DEBUG_MACRO == 1
	printf("Debug Macro: Expanding branch delay macro...\n");
#endif

	// Create the expansion instruction which will store the inserted `NOP`.
	Statement* expansion = malloc(sizeof(Statement));
	if(!expansion) {
		fprintf(stderr, "Error allocating statement for macro expansion\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
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

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * expand_macro_move
 */
Assembler_Status expand_macro_move(Statement* macro) {
#if DEBUG_MACRO == 1
	printf("Debug Macro: Expanding `MOVE` pseudo-instruction...\n");
#endif

	if(!check_operand_count(2, &macro->instruction.opseq)) {
		fprintf(stderr, "Operand count mismatch for `MOVE` pseudo-instruction\n");
		return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
	}

	// The `MOVE` pseudo-instruction is analogous to an ADD instruction between
	// one register and $zero so we replace the opcode with an `ADD`, and then
	// add a final operand referencing the $zero register.
	macro->instruction.opcode = OPCODE_ADD;
	macro->instruction.opseq.n_operands = 3;
	macro->instruction.opseq.operands = realloc(macro->instruction.opseq.operands,
		sizeof(Operand) * 3);
	if(!macro->instruction.opseq.operands) {
		fprintf(stderr, "Error allocating operand sequence for macro expansion\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	macro->instruction.opseq.operands[2].type = OPERAND_TYPE_REGISTER;
	macro->instruction.opseq.operands[2].reg = REGISTER_$ZERO;

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * expand_macros
 */
Assembler_Status expand_macros(Statement* statements) {
	Statement* curr = statements;

	Assembler_Status macro_process_status;

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

		// If an error was encountered, return here.
		// The error will have been set in the expansion function.
		if(macro_process_status != ASSEMBLER_STATUS_SUCCESS) {
			return ASSEMBLER_ERROR_MACRO_EXPANSION;
		}

		curr = curr->next;
	}


	return ASSEMBLER_STATUS_SUCCESS;
}
