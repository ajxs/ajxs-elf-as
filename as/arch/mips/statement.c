/**
 * @file statement.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Architecture-specific functions for working with statement entities.
 * Architecture-specific functionality for working with statement entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <statement.h>


/**
 * @brief Gets the string representation of an operand.
 *
 * This function returns the string representation of an operand.
 * @param op The operand to get the string for.
 * @returns a pointer to the string containing the operand name.
 */
const char *get_opcode_string(Opcode op) {
	if(op == OPCODE_ADDIU) {
		return "ADDIU";
	} else if(op == OPCODE_ADDI) {
		return "ADDI";
	} else if(op == OPCODE_ADDU) {
		return "ADDU";
	} else if(op == OPCODE_ADD) {
		return "ADD";
	} else if(op == OPCODE_BAL) {
		return "BAL";
	} else if(op == OPCODE_BEQZ) {
		return "BEQZ";
	} else if(op == OPCODE_BEQ) {
		return "BEQ";
	} else if(op == OPCODE_BGEZ) {
		return "BGEZ";
	} else if(op == OPCODE_BNE) {
		return "BNE";
	} else if(op == OPCODE_BLEZ) {
		return "BLEZ";
	} else if(op == OPCODE_DIV) {
		return "DIV";
	} else if(op == OPCODE_JALR) {
		return "JALR";
	} else if(op == OPCODE_JAL) {
		return "JAL";
	} else if(op == OPCODE_JR) {
		return "JR";
	} else if(op == OPCODE_J) {
		return "J";
	} else if(op == OPCODE_LA) {
		return "LA";
	} else if(op == OPCODE_LBU) {
		return "LBU";
	} else if(op == OPCODE_LB) {
		return "LB";
	} else if(op == OPCODE_LHU) {
		return "LHU";
	} else if(op == OPCODE_LI) {
		return "LI";
	} else if(op == OPCODE_LUI) {
		return "LUI";
	} else if(op == OPCODE_LW) {
		return "LW";
	} else if(op == OPCODE_MUHU) {
		return "MUHU";
	} else if(op == OPCODE_MUH) {
		return "MUH";
	} else if(op == OPCODE_MUL) {
		return "MUL";
	} else if(op == OPCODE_MULU) {
		return "MULU";
	} else if(op == OPCODE_NOR) {
		return "NOR";
	} else if(op == OPCODE_NOP) {
		return "NOP";
	} else if(op == OPCODE_ORI) {
		return "ORI";
	} else if(op == OPCODE_OR) {
		return "OR";
	} else if(op == OPCODE_SB) {
		return "SB";
	} else if(op == OPCODE_SH) {
		return "SH";
	} else if(op == OPCODE_SLL) {
		return "SLL";
	} else if(op == OPCODE_SUBU) {
		return "SUBU";
	} else if(op == OPCODE_SUB) {
		return "SUB";
	} else if(op == OPCODE_SW) {
		return "SW";
	} else if(op == OPCODE_SYSCALL) {
		return "SYSCALL";
	}

	return "UNKNOWN";
}


/**
 * @brief Gets the size of a statement entity.
 *
 * Returns the number of bytes required to encode a specific statement.
 * @param statement The statement to encode.
 * @returns The number of bytes required to encode the entity, or -1 if an error
 * occurred.
 */
ssize_t get_statement_size(Statement *statement) {
	if(!statement) {
		fprintf(stderr, "Invalid statement provided to get statement size function.\n");
		return -1;
	}

	size_t total_len = 0;
	size_t count = 0;
	size_t fill_size = 0;
	size_t string_len;

	if(statement->type == STATEMENT_TYPE_INSTRUCTION) {
		return 4;
	}

	if(statement->type == STATEMENT_TYPE_DIRECTIVE) {
		switch(statement->directive.type) {
			case DIRECTIVE_ALIGN:
			case DIRECTIVE_DATA:
			case DIRECTIVE_BSS:
			case DIRECTIVE_SIZE:
			case DIRECTIVE_TEXT:
			case DIRECTIVE_GLOBAL:
				return 0;
			case DIRECTIVE_ASCII:
				for(size_t i=0; i<statement->directive.opseq.n_operands; i++) {
					string_len = strlen(statement->directive.opseq.operands[i].string_literal);
					total_len += string_len;
				}

				return total_len;
			case DIRECTIVE_STRING:
			case DIRECTIVE_ASCIZ:
				for(size_t i=0; i<statement->directive.opseq.n_operands; i++) {
					// Extra 1 added to account for the trailing NULL byte.
					string_len = strlen(statement->directive.opseq.operands[i].string_literal);
					total_len += (string_len + 1);
				}

				return total_len;
			case DIRECTIVE_BYTE:
				return 1;
			case DIRECTIVE_SHORT:
				return 2;
			case DIRECTIVE_LONG:
				return 4;
			case DIRECTIVE_WORD:
				return 4;
			case DIRECTIVE_FILL:
				count = statement->directive.opseq.operands[0].numeric_literal;
				fill_size = statement->directive.opseq.operands[1].numeric_literal;
				if(fill_size > 8) {
					// Fill size is capped at 8, as per GAS docs.
					// https://ftp.gnu.org/old-gnu/Manuals/gas-2.9.1/html_chapter/as_7.html#SEC91
					fill_size = 8;
				}

				return count;
			case DIRECTIVE_SKIP:
			case DIRECTIVE_SPACE:
				return statement->directive.opseq.operands[0].numeric_literal;
			default:
				fprintf(stderr, "Unknown directive type in get statement size function.\n");
				return -1;
		}
	}

	if(statement->type == STATEMENT_TYPE_EMPTY) {
		return 0;
	}

	fprintf(stderr, "Unknown statement type in get statement size function.\n");
	return -1;
}
