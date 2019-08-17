/**
 * @file codegen.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for generation of encoded binary machine-code from instructions.
 * Contains functions for generation of machine-code from parsed instructions specific
 * to the MIPS architecture.
 * The main entry point is the `encode_instruction` function, which accepts a
 * statement containing an instruction, discerns its type and encodes it accordingly.
 * @version 0.1
 * @date 2019-03-09
 */

#include <as.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <codegen.h>
#include <directive.h>
#include <instruction.h>
#include <statement.h>
#include <symtab.h>




/**
 * @brief Encodes an I type instruction.
 *
 * Encodes an I-type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param symtab The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param opcode The operand encoding.
 * @param rs The rs field to encode.
 * @param rt The rt field to encode.
 * @param imm The imm operand to encode.
 * @param program_counter The current program_counter.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_i_type(Encoding_Entity** encoded_instruction,
	Symbol_Table* const symtab,
	const uint8_t opcode,
	const uint8_t rs,
	const uint8_t rt,
	const Operand imm,
	const size_t program_counter) {

	*encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!*encoded_instruction) {
		fprintf(stderr, "Error: Error allocating encoded instruction\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	(*encoded_instruction)->n_reloc_entries = 0;
	(*encoded_instruction)->reloc_entries = NULL;

	uint32_t* encoding = malloc(sizeof(uint32_t));
	if(!*encoded_instruction) {
		free(encoded_instruction);

		// @TODO: Global error handler.
		fprintf(stderr, "Error: Error allocating instruction encoding\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	*encoding = opcode << 26;
	*encoding |= rs << 21;
	*encoding |= rt << 16;

	uint32_t immediate = 0;
	if(imm.type == OPERAND_TYPE_NUMERIC_LITERAL) {
		// If the operand is a numeric literal, the raw value is encoded.
		immediate = imm.numeric_literal;
	} else if(imm.type == OPERAND_TYPE_SYMBOL) {
		// If the operand is a symbolic reference, we encode the immediate value
		// as the symbol's offset, and then create a relocation entry so that the
		// symbol can be linked correctly.
		Symbol *symbol = symtab_find_symbol(symtab, imm.symbol);
		if(!symbol) {
			// cleanup.
			free(encoding);
			free(encoded_instruction);

			fprintf(stderr, "Error: Error finding symbol `%s`", imm.symbol);
			return ASSEMBLER_ERROR_MISSING_SYMBOL;
		}

		immediate = symbol->offset;

		(*encoded_instruction)->n_reloc_entries = 1;
		(*encoded_instruction)->reloc_entries = malloc(sizeof(Reloc_Entry));
		if(!(*encoded_instruction)->reloc_entries) {
			// cleanup.
			free(encoding);
			free(encoded_instruction);

			fprintf(stderr, "Error: Error allocating relocation entries");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		(*encoded_instruction)->reloc_entries[0].type = R_MIPS_PC16;
		if(imm.flags.mask == OPERAND_MASK_HIGH) {
			// If this is the higher component of a symbol.
			// Most likely the result of a macro expansion. Refer to the macro
			// expansion logic for the relevant architecture.
			(*encoded_instruction)->reloc_entries[0].type = R_MIPS_HI16;
		} else if(imm.flags.mask == OPERAND_MASK_LOW) {
			(*encoded_instruction)->reloc_entries[0].type = R_MIPS_LO16;
		}

		(*encoded_instruction)->reloc_entries[0].symbol_name = imm.symbol;
		(*encoded_instruction)->reloc_entries[0].offset = program_counter;
	} else {
		// cleanup.
		free(encoding);
		free(encoded_instruction);

		// If the immediate is of any other type, it is an error.
		fprintf(stderr, "Error: Bad operand type `%u` for immediate type instruction",
			imm.type);
		return CODEGEN_ERROR_BAD_OPERAND_TYPE;
	}

	*encoding |= (immediate & 0xFFFF);

	(*encoded_instruction)->size = 4;
	(*encoded_instruction)->data = encoding;
	(*encoded_instruction)->next = NULL;

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * @brief Encodes an r-type type instruction.
 *
 * Encodes an r-type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param opcode The operand encoding.
 * @param rd The rd field to encode.
 * @param rt The rt field to encode.
 * @param rs The rs field to encode.
 * @param sa The sa field to encode.
 * @param func The func field to encode.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_r_type(Encoding_Entity** encoded_instruction,
	const uint8_t opcode,
	const uint8_t rd,
	const uint8_t rs,
	const uint8_t rt,
	const uint8_t sa,
	const uint8_t func) {

	*encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!*encoded_instruction) {
		fprintf(stderr, "Error allocating encoded instruction.\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	// The encoding entity is declared on the heap, since a pointer to this data
	// will be stored in the `encoded_instruction` entity.
	uint32_t* encoding = malloc(sizeof(uint32_t));
	if(!encoding) {
		// Cleanup instruction data.
		free(encoded_instruction);

		fprintf(stderr, "Error allocating instruction encoding\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	*encoding = opcode << 26;
	*encoding |= rs << 21;
	*encoding |= rt << 16;
	*encoding |= rd << 11;
	// Truncated to 5 bits.
	*encoding |= (sa & 0x1F) << 6;
	*encoding |= func;

	(*encoded_instruction)->n_reloc_entries = 0;
	(*encoded_instruction)->reloc_entries = NULL;

	(*encoded_instruction)->size = 4;
	(*encoded_instruction)->data = encoding;
	(*encoded_instruction)->next = NULL;

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * @brief Encodes an offset type instruction.
 *
 * Encodes an offset type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param opcode The operand encoding.
 * @param rt The rt field to encode.
 * @param reg The reg operand to encode.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Encoding_Entity *encode_offset_type(char *error_message,
	uint8_t opcode,
	uint8_t rt,
	Operand offset_reg) {

	// Unlike GAS, this assembler currently does not support using symbols as an offset value.
	if(offset_reg.type != OPERAND_TYPE_REGISTER) {
		sprintf(error_message, "Bad operand type `%u` for offset-type instruction.",
			offset_reg.type);
		return NULL;
	}

	// Truncate to 16bits.
	uint16_t offset = offset_reg.offset & 0xFFFF;
	uint8_t base = encode_operand_register(offset_reg.reg);

	uint32_t encoding = opcode << 26;
	encoding |= base << 21;
	encoding |= rt << 16;
	encoding |= offset;

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!encoded_instruction) {
		sprintf(error_message, "Error allocating encoded instruction.\n");
		return NULL;
	}

	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	if(!encoded_instruction->data) {
		// cleanup.
		free(encoded_instruction);

		sprintf(error_message, "Error allocating encoded instruction data.\n");
		return NULL;
	}

	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


/**
 * @brief Encodes a J type instruction.
 *
 * Encodes a J-type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * See: https://stackoverflow.com/questions/7877407/jump-instruction-in-mips-assembly#7877528
 * https://stackoverflow.com/questions/6950230/how-to-calculate-jump-target-address-and-branch-target-address
 * @param symtab The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param opcode The operand encoding.
 * @param imm The imm operand to encode.
 * @param program_counter The current program_counter.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Encoding_Entity *encode_j_type(char *error_message,
	Symbol_Table *symtab,
	uint8_t opcode,
	Operand imm,
	size_t program_counter) {
	uint32_t encoding = opcode << 26;

	if(!symtab) {
		sprintf(error_message, "Invalid symbol table provided to encoding function.\n");
		return NULL;
	}

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!encoded_instruction) {
		sprintf(error_message, "Error allocating encoded instruction.\n");
		return NULL;
	}

	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	uint32_t immediate = 0;
	if(imm.type == OPERAND_TYPE_NUMERIC_LITERAL) {
		immediate = imm.numeric_literal;
	} else if(imm.type == OPERAND_TYPE_SYMBOL) {
		Symbol *symbol = symtab_find_symbol(symtab, imm.symbol);

		encoded_instruction->n_reloc_entries = 1;
		encoded_instruction->reloc_entries = malloc(sizeof(Reloc_Entry));
		if(!encoded_instruction->reloc_entries) {
			// cleanup.
			free(encoded_instruction);

			sprintf(error_message, "Error allocating relocation entries.\n");
			return NULL;
		}

		encoded_instruction->reloc_entries[0].type = R_MIPS_26;
		// encoded_instruction->reloc_entries[0].symbol = symbol;
		encoded_instruction->reloc_entries[0].offset = program_counter;

		immediate = symbol->offset;
	} else {
		// cleanup.
		free(encoded_instruction);

		char error_message[ERROR_MSG_MAX_LEN];
		sprintf(error_message, "Bad operand type `%u` for jump type instruction.",
			imm.type);
		return NULL;
	}

	immediate = (immediate & 0x0FFFFFFF) >> 2;

	// Truncate to 26bits.
	encoding |= (immediate & 0x7FFFFFF);

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	if(!encoded_instruction->data) {
		// cleanup.
		free(encoded_instruction);

		sprintf(error_message, "Error allocating encoded instruction data.");
		return NULL;
	}

	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


/**
 * @brief Encodes an Instruction entity.
 *
 * Encodes an instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param symtab The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param instruction The parsed instruction entity to encode.
 * @param program_counter The current program counter. This represents the current
 * place of the instruction within the current encoding context, which is the
 * current program section.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_instruction(Encoding_Entity **encoded_instruction,
	Symbol_Table *symtab,
	Instruction *instruction,
	size_t program_counter) {

	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to encoding function.\n");
		return CODEGEN_ERROR_INVALID_ARGS;
	}

	if(!instruction) {
		fprintf(stderr, "Error: Invalid instruction provided to encoding function.\n");
		return CODEGEN_ERROR_INVALID_ARGS;
	}

	const char *opcode_name = get_opcode_string(instruction->opcode);
	if(!opcode_name) {
		fprintf(stderr, "Error: Unable to get opcode name for `%i`.\n", instruction->opcode);
		return CODEGEN_ERROR_BAD_OPCODE;
	}

	/** The error messaged used in error handling in this function. */
	char *error_message = malloc(ERROR_MSG_MAX_LEN);
	if(!error_message) {
		fprintf(stderr, "Error: Error allocating error message string.\n");
		return CODEGEN_ERROR_BAD_ALLOC;
	}

	uint8_t rd = 0;
	uint8_t rs = 0;
	uint8_t rt = 0;
	uint8_t sa = 0;

	switch(instruction->opcode) {
		case OPCODE_ADD:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encode_r_type(encoded_instruction, 0, rd, rs, rt, 0, 0x20);
			break;
		case OPCODE_ADDI:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encode_i_type(encoded_instruction, symtab, 0x8, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_ADDIU:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encode_i_type(encoded_instruction, symtab, 0x9, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_ADDU:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encode_r_type(encoded_instruction, 0, rd, rs, rt, 0, 0x21);
			break;
		case OPCODE_AND:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encode_r_type(encoded_instruction, 0, rd, rs, rt, 0, 0x24);
			break;
		case OPCODE_ANDI:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encode_i_type(encoded_instruction, symtab, 0xC, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BAL:
			if(!check_operand_count(1, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			encode_i_type(encoded_instruction, symtab, 1, 0, 0x11,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BEQ:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encode_i_type(encoded_instruction, symtab, 0x4, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BGEZ:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encode_i_type(encoded_instruction, symtab, 0x14, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BNE:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encode_i_type(encoded_instruction, symtab, 0x5, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_J:
			if(!check_operand_count(1, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			*encoded_instruction = encode_j_type(error_message, symtab, 0x2,
				instruction->opseq.operands[0], program_counter);
			break;
		case OPCODE_JAL:
			if(!check_operand_count(1, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			*encoded_instruction = encode_j_type(error_message, symtab, 0x3,
				instruction->opseq.operands[0], program_counter);
			break;
		case OPCODE_JALR:
			if(!check_operand_count(1, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			if(instruction->opseq.n_operands == 1) {
				rd = 0x1F;
			} else {
				rd = encode_operand_register(instruction->opseq.operands[0].reg);
				rs = encode_operand_register(instruction->opseq.operands[1].reg);
			}

			encode_r_type(encoded_instruction, 0, rd, rs, 0, 0, 0x9);
			break;
		case OPCODE_JR:
			if(!check_operand_count(1, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			encode_r_type(encoded_instruction, 0, 0, rs, 0, 0, 0x9);
			break;
		case OPCODE_LB:
			if(!check_operand_count(2, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			*encoded_instruction = encode_offset_type(error_message, 0x20, rt,
				instruction->opseq.operands[1]);
			break;
		case OPCODE_LBU:
			if(!check_operand_count(2, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			*encoded_instruction = encode_offset_type(error_message, 0x24, rt,
				instruction->opseq.operands[1]);
			break;
		case OPCODE_LUI:
			if(!check_operand_count(2, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encode_i_type(encoded_instruction, symtab, 0xF, 0, rt,
				instruction->opseq.operands[1], program_counter);
			break;
		case OPCODE_LW:
			if(!check_operand_count(2, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			*encoded_instruction = encode_offset_type(error_message, 0x23, rt,
				instruction->opseq.operands[1]);
			break;
		case OPCODE_MUH:
		case OPCODE_MUHU:
		case OPCODE_MUL:
		case OPCODE_MULU:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);

			if(instruction->opcode == OPCODE_MUH) {
				encode_r_type(encoded_instruction, 0, rd, rs, rt, 0x3, 0x18);
			} else if(instruction->opcode == OPCODE_MUHU) {
				encode_r_type(encoded_instruction, 0, rd, rs, rt, 0x3, 0x19);
			} else if(instruction->opcode == OPCODE_MUL) {
				encode_r_type(encoded_instruction, 0, rd, rs, rt, 0x2, 0x18);
			} else if(instruction->opcode == OPCODE_MULU) {
				encode_r_type(encoded_instruction, 0, rd, rs, rt, 0x2, 0x19);
			}
			break;
		case OPCODE_MULT:
		case OPCODE_MULTU:
			// Deprecated opcodes.
			// `encoded_entity` is still NULL, so the error handler will catch this.
			error_message = "Instruction deprecated in `MIPS32r6`.";
			break;
		case OPCODE_NOP:
			if(!check_operand_count(0, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			encode_r_type(encoded_instruction, 0, 0, 0, 0, 0, 0);
			break;
		case OPCODE_OR:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encode_r_type(encoded_instruction, 0, rd, rs, rt, 0, 0x25);
			break;
		case OPCODE_ORI:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encode_i_type(encoded_instruction, symtab, 0xD, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_SB:
			if(!check_operand_count(2, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			*encoded_instruction = encode_offset_type(error_message, 0x28, rt,
				instruction->opseq.operands[1]);
			break;
		case OPCODE_SH:
			if(!check_operand_count(2, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			*encoded_instruction = encode_offset_type(error_message, 0x29, rt,
				instruction->opseq.operands[1]);
			break;
		case OPCODE_SLL:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			sa = instruction->opseq.operands[2].numeric_literal;
			encode_r_type(encoded_instruction, 0, rd, 0, rt, sa, 0x0);
			break;
		case OPCODE_SUB:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encode_r_type(encoded_instruction, 0, rd, rs, rt, 0, 0x22);
			break;
		case OPCODE_SUBU:
			if(!check_operand_count(3, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encode_r_type(encoded_instruction, 0, rd, rs, rt, 0, 0x23);
			break;
		case OPCODE_SW:
			if(!check_operand_count(2, &instruction->opseq)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			*encoded_instruction = encode_offset_type(error_message, 0x2B, rt,
				instruction->opseq.operands[1]);
			break;
		case OPCODE_SYSCALL:
			// @TODO: Investigate use of `code` field.
			encode_r_type(encoded_instruction, 0, 0, 0, 0, 0, 0xC);
			break;
		case OPCODE_UNKNOWN:
		default:
			fprintf(stderr, "Error: Unrecognised Opcode `%i`.\n", instruction->opcode);
			return CODEGEN_ERROR_BAD_OPCODE;
	}

	if(!*encoded_instruction) {
		// Add the error message returned from the encoding function to a more
		// generatlised error message that prints the instruction.
		fprintf(stderr, "Error: Error encoding instruction `%s`: %s\n",
			opcode_name, error_message);

		free(error_message);

		return CODEGEN_ERROR_BAD_ALLOC;
	}

#if DEBUG_CODEGEN == 1
	printf("Debug Codegen: Encoded instruction `%s` at `0x%zx` as `0x%x`.\n",
		opcode_name, program_counter, 0);
#endif

	return ASSEMBLER_STATUS_SUCCESS;

INSTRUCTION_OPERAND_COUNT_MISMATCH:
	free(error_message);

	fprintf(stderr, "Error: Operand count mismatch for instruction `%s`.\n", opcode_name);
	return CODEGEN_ERROR_BAD_ALLOC;
}


Encoding_Entity *encode_directive(Symbol_Table *symtab,
	Directive *directive,
	size_t program_counter) {

	(void)program_counter;
	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to encoding function.\n");
		return NULL;
	}

	if(!directive) {
		fprintf(stderr, "Error: Invalid directive provided to encoding function.\n");
		return NULL;
	}

	Encoding_Entity *encoded_entity = malloc(sizeof(Encoding_Entity));
	if(!encoded_entity) {
		fprintf(stderr, "Error: Error allocating encoding entity.\n");
		return NULL;
	}

	encoded_entity->n_reloc_entries = 0;
	encoded_entity->reloc_entries = NULL;
	encoded_entity->next = NULL;

	size_t total_len = 0;
	size_t count = 0;
	size_t fill_size = 0;
	size_t string_len;
	uint8_t *data = NULL;
	size_t curr_pos = 0;

	const char *directive_name = get_directive_string(*directive);
	if(!directive_name) {
		// cleanup.
		free(encoded_entity);

		fprintf(stderr, "Error: Unable to get directive type for `%i`.\n", directive->type);
		return NULL;
	}

#if DEBUG_CODEGEN == 1
	printf("Debug Codegen: Encoding directive type `%u`...", directive->type);
#endif

	switch(directive->type) {
		case DIRECTIVE_ASCII:
			if(directive->opseq.n_operands < 1) {
				goto DIRECTIVE_OPERAND_COUNT_MISMATCH;
			}

			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				// Iterate through each string operand.
				string_len = strlen(directive->opseq.operands[i].string_literal);
				curr_pos = total_len;
				total_len += string_len;
				data = realloc(data, total_len);
				if(!data) {
					fprintf(stderr, "Error: Error allocating directive data.\n");
					return NULL;
				}

				memcpy(data + curr_pos, &directive->opseq.operands[i].string_literal, string_len);
			}

			encoded_entity->size = total_len;
			encoded_entity->data = data;
			break;
		case DIRECTIVE_STRING:
		case DIRECTIVE_ASCIZ:
			if(directive->opseq.n_operands < 1) {
				goto DIRECTIVE_OPERAND_COUNT_MISMATCH;
			}

			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				string_len = strlen(directive->opseq.operands[i].string_literal);
				curr_pos = total_len;
				total_len += (1 + string_len);    // Take NUL terminator into account.

				data = realloc(data, total_len);
				if(!data) {
					fprintf(stderr, "Error: Error allocating directive data.\n");
					return NULL;
				}

				memcpy(data + curr_pos, directive->opseq.operands[i].string_literal, string_len);

				// Add NULL terminator.
				data[total_len - 1] = '\0';
			}

			encoded_entity->size = total_len;
			encoded_entity->data = data;
			break;
		case DIRECTIVE_BYTE:
			break;
		case DIRECTIVE_FILL:
			break;
		case DIRECTIVE_LONG:
			break;
		case DIRECTIVE_SHORT:
			break;
		case DIRECTIVE_SIZE:
			break;
		case DIRECTIVE_SKIP:
			break;
		case DIRECTIVE_SPACE:
		case DIRECTIVE_WORD:
			if(directive->opseq.n_operands < 1) {
				goto DIRECTIVE_OPERAND_COUNT_MISMATCH;
			}

			total_len = sizeof(uint32_t) * directive->opseq.n_operands;

			uint32_t *word_data = malloc(total_len);
			if(!word_data) {
				fprintf(stderr, "Error: Error allocating directive data.\n");
				return NULL;
			}

			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				// Create an array of each of the word operands.
				if(directive->opseq.operands[i].type == OPERAND_TYPE_SYMBOL) {
					Symbol *symbol = symtab_find_symbol(symtab,
						directive->opseq.operands[i].string_literal);
					if(!symbol) {
						// cleanup.
						free(word_data);

						fprintf(stderr, "Error: Error finding symbol `%s`.\n",
							directive->opseq.operands[i].string_literal);
						return NULL;
					}

					word_data[i] = symbol->offset;
				} else if(directive->opseq.operands[i].type == OPERAND_TYPE_NUMERIC_LITERAL) {
					word_data[i] = directive->opseq.operands[i].numeric_literal;
				} else {
					// cleanup.
					free(word_data);

					fprintf(stderr, "Error: Invalid operand type for `%s` directive.", directive_name);
					return NULL;
				}
			}

			data = malloc(total_len);
			if(!data) {
				// cleanup.
				free(word_data);

				fprintf(stderr, "Error: Error allocating directive data.");
				return NULL;
			}

			memcpy(data, word_data, total_len);
			encoded_entity->size = total_len;
			encoded_entity->data = data;
			encoded_entity->next = NULL;

			free(word_data);
			break;
		// Non-encoded directives.
		case DIRECTIVE_ALIGN:
		case DIRECTIVE_BSS:
		case DIRECTIVE_DATA:
		case DIRECTIVE_GLOBAL:
		case DIRECTIVE_TEXT:
		case DIRECTIVE_UNKNOWN:
			fprintf(stderr, "Error: Invalid non-encoded directive type.\n");
			return NULL;
		default:
			fprintf(stderr, "Error: Unknown directive type.\n");
			return NULL;
	}


#if DEBUG_CODEGEN == 1
	printf("Produced output with len: `%lu`.\n", encoded_entity->size);
#endif

	return encoded_entity;

DIRECTIVE_OPERAND_COUNT_MISMATCH:
	fprintf(stderr, "Error: Operand count mismatch for directive `%s`.\n", directive_name);
	return NULL;
}
