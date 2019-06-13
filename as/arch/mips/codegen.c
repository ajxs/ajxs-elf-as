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


Encoding_Entity *encode_r_type(uint8_t opcode,
	uint8_t rd,
	uint8_t rs,
	uint8_t rt,
	uint8_t sa,
	uint8_t func) {

	uint32_t encoding = 0;
	encoding |= opcode << 26;
	encoding |= rs << 21;
	encoding |= rt << 16;
	encoding |= rd << 11;
	// Truncated to 5 bits.
	encoding |= (sa & 0x1F) << 6;
	encoding |= func;

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!encoded_instruction) {
		set_error_message("Error allocating encoded instruction.");
		return NULL;
	}

	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	if(!encoded_instruction->data) {
		set_error_message("Error allocating encoded instruction data.");
		return NULL;
	}

	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


Encoding_Entity *encode_offset_type(uint8_t opcode,
	uint8_t rt,
	Operand offset_reg) {

	// Unlike GAS, this assembler currently does not support using symbols as an offset value.
	if(offset_reg.type != OPERAND_TYPE_REGISTER) {
		char error_message[ERROR_MSG_MAX_LEN];
		sprintf(error_message, "Bad operand type `%u` for offset-type instruction.",
			offset_reg.type);
		set_error_message(error_message);
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
		set_error_message("Error allocating encoded instruction.");
		return NULL;
	}

	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	if(!encoded_instruction->data) {
		set_error_message("Error allocating encoded instruction data.");
		return NULL;
	}

	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


Encoding_Entity *encode_i_type(Symbol_Table *symtab,
	uint8_t opcode,
	uint8_t rs,
	uint8_t rt,
	Operand imm,
	size_t program_counter) {

	uint32_t encoding = opcode << 26;
	encoding |= rs << 21;
	encoding |= rt << 16;

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!encoded_instruction) {
		set_error_message("Error allocating encoded instruction.");
		return NULL;
	}

	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

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
			char error_message[ERROR_MSG_MAX_LEN];
			sprintf(error_message, "Error finding symbol `%s`.", imm.symbol);
			set_error_message(error_message);
			return NULL;
		}

		immediate = symbol->offset;

		encoded_instruction->n_reloc_entries = 1;
		encoded_instruction->reloc_entries = malloc(sizeof(Reloc_Entry));
		if(!encoded_instruction->reloc_entries) {
			set_error_message("Error allocating relocation entries.");
			return NULL;
		}

		encoded_instruction->reloc_entries[0].type = R_MIPS_PC16;
		if(imm.flags.mask == OPERAND_MASK_HIGH) {
			// If this is the higher component of a symbol.
			// Most likely the result of a macro expansion. Refer to the macro
			// expansion logic for the relevant architecture.
			encoded_instruction->reloc_entries[0].type = R_MIPS_HI16;
		} else if(imm.flags.mask == OPERAND_MASK_LOW) {
			encoded_instruction->reloc_entries[0].type = R_MIPS_LO16;
		}

		encoded_instruction->reloc_entries[0].symbol = symbol;
		encoded_instruction->reloc_entries[0].offset = program_counter;
	} else {
		// If the immediate is of any other type, it is an error.
		char error_message[ERROR_MSG_MAX_LEN];
		sprintf(error_message, "Bad operand type `%u` for immediate type instruction.",
			imm.type);
		set_error_message(error_message);
		return NULL;
	}

	encoding |= (immediate & 0xFFFF);

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	if(!encoded_instruction->data) {
		set_error_message("Error allocating encoded instruction data.");
		return NULL;
	}

	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


// Parses the immediate operand to a J type instruction.
// See: https://stackoverflow.com/questions/7877407/jump-instruction-in-mips-assembly#7877528
// https://stackoverflow.com/questions/6950230/how-to-calculate-jump-target-address-and-branch-target-address
Encoding_Entity *encode_j_type(Symbol_Table *symtab,
	uint8_t opcode,
	Operand imm,
	size_t program_counter) {
	uint32_t encoding = opcode << 26;

	if(!symtab) {
		set_error_message("Invalid symbol table provided to encoding function.");
		return NULL;
	}

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!encoded_instruction) {
		set_error_message("Error allocating encoded instruction.");
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
			set_error_message("Error allocating relocation entries.");
			return NULL;
		}

		encoded_instruction->reloc_entries[0].type = R_MIPS_26;
		encoded_instruction->reloc_entries[0].symbol = symbol;
		encoded_instruction->reloc_entries[0].offset = program_counter;

		immediate = symbol->offset;
	} else {
		char error_message[ERROR_MSG_MAX_LEN];
		sprintf(error_message, "Bad operand type `%u` for jump type instruction.",
			imm.type);
		set_error_message(error_message);
		return NULL;
	}

	immediate = (immediate & 0x0FFFFFFF) >> 2;

	// Truncate to 26bits.
	encoding |= (immediate & 0x7FFFFFF);

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	if(!encoded_instruction->data) {
		set_error_message("Error allocating encoded instruction data.");
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
 * curent program section.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Encoding_Entity *encode_instruction(Symbol_Table *symtab,
	Instruction *instruction,
	size_t program_counter) {

	if(!symtab) {
		set_error_message("Invalid symbol table provided to encoding function.");
		return NULL;
	}

	if(!instruction) {
		set_error_message("Invalid instruction provided to encoding function.");
		return NULL;
	}

	/** The error messaged used in error handling in this function. */
	char error_message[ERROR_MSG_MAX_LEN];
	char *opcode_name = get_opcode_string(instruction->opcode);

	/** The resulting encoding entity. */
	Encoding_Entity *encoded_entity = NULL;
	uint8_t rd = 0;
	uint8_t rs = 0;
	uint8_t rt = 0;
	uint8_t sa = 0;

	switch(instruction->opcode) {
		case OPCODE_ADD:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x20);
			break;
		case OPCODE_ADDI:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encoded_entity = encode_i_type(symtab, 0x8, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_ADDIU:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encoded_entity = encode_i_type(symtab, 0x9, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_ADDU:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x21);
			break;
		case OPCODE_AND:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x24);
			break;
		case OPCODE_ANDI:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encoded_entity = encode_i_type(symtab, 0xC, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BAL:
			if(!instruction_check_operand_length(1, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			encoded_entity = encode_i_type(symtab, 1, 0, 0x11,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BEQ:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encoded_entity = encode_i_type(symtab, 0x4, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BGEZ:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encoded_entity = encode_i_type(symtab, 0x14, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_BNE:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encoded_entity = encode_i_type(symtab, 0x5, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_J:
			if(!instruction_check_operand_length(1, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			encoded_entity = encode_j_type(symtab, 0x2,
				instruction->opseq.operands[0], program_counter);
			break;
		case OPCODE_JAL:
			if(!instruction_check_operand_length(1, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			encoded_entity = encode_j_type(symtab, 0x3,
				instruction->opseq.operands[0], program_counter);
			break;
		case OPCODE_JALR:
			if(!instruction_check_operand_length(1, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			if(instruction->opseq.n_operands == 1) {
				rd = 0x1F;
			} else {
				rd = encode_operand_register(instruction->opseq.operands[0].reg);
				rs = encode_operand_register(instruction->opseq.operands[1].reg);
			}

			encoded_entity = encode_r_type(0, rd, rs, 0, 0, 0x9);
			break;
		case OPCODE_JR:
			if(!instruction_check_operand_length(1, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_r_type(0, 0, rs, 0, 0, 0x9);
			break;
		case OPCODE_LB:
			if(!instruction_check_operand_length(2, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_offset_type(0x20, rt, instruction->opseq.operands[1]);
			break;
		case OPCODE_LBU:
			if(!instruction_check_operand_length(2, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_offset_type(0x24, rt, instruction->opseq.operands[1]);
			break;
		case OPCODE_LUI:
			if(!instruction_check_operand_length(2, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_i_type(symtab, 0xF, 0, rt,
				instruction->opseq.operands[1], program_counter);
			break;
		case OPCODE_LW:
			if(!instruction_check_operand_length(2, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_offset_type(0x23, rt, instruction->opseq.operands[1]);
			break;
		case OPCODE_MUH:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x3, 0x18);
			break;
		case OPCODE_MUHU:
			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x3, 0x19);
			break;
		case OPCODE_MUL:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x2, 0x18);
			break;
		case OPCODE_MULU:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x2, 0x19);
			break;
		case OPCODE_MULT:
		case OPCODE_MULTU:
			// Deprecated opcodes.

			break;
		case OPCODE_NOP:
			if(!instruction_check_operand_length(0, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			encoded_entity = encode_r_type(0, 0, 0, 0, 0, 0);
			break;
		case OPCODE_OR:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x25);
			break;
		case OPCODE_ORI:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);
			encoded_entity = encode_i_type(symtab, 0xD, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_SB:
			if(!instruction_check_operand_length(2, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_offset_type(0x28, rt, instruction->opseq.operands[1]);
			break;
		case OPCODE_SH:
			if(!instruction_check_operand_length(2, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_offset_type(0x29, rt, instruction->opseq.operands[1]);
			break;
		case OPCODE_SLL:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			sa = instruction->opseq.operands[2].numeric_literal;
			encoded_entity = encode_r_type(0, rd, 0, rt, sa, 0x0);
			break;
		case OPCODE_SUB:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x22);
			break;
		case OPCODE_SUBU:
			if(!instruction_check_operand_length(3, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x23);
			break;
		case OPCODE_SW:
			if(!instruction_check_operand_length(2, instruction)) {
				goto INSTRUCTION_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			encoded_entity = encode_offset_type(0x2B, rt, instruction->opseq.operands[1]);
			break;
		case OPCODE_SYSCALL:
			// @TODO: Investigate use of `code` field.
			encoded_entity = encode_r_type(0, 0, 0, 0, 0, 0xC);
			break;
		case OPCODE_UNKNOWN:
		default:
			sprintf(error_message, "Unrecognised Opcode `%i`.", instruction->opcode);
			set_error_message(error_message);
			return NULL;
	}

#if DEBUG_CODEGEN == 1
	printf("Debug Codegen: Encoded instruction `` at `0x%lx` as `0x%x`.\n",
		opcode_name, program_counter, *(uint32_t *)encoded_entity->data);
#endif

	if(!encoded_entity) {
		// Add the error message returned from the encoding function to a more
		// generatlised error message that prints the instruction.
		sprintf(error_message, "Error encoding instruction `%s`: %s",
			opcode_name, assembler_error_msg);
		set_error_message(error_message);
		return NULL;
	}

	return encoded_entity;

INSTRUCTION_OPERAND_COUNT_MISMATCH:
	sprintf(error_message, "Operand count mismatch for instruction `%s`.", opcode_name);
	set_error_message(error_message);
	return NULL;
}


Encoding_Entity *encode_directive(Symbol_Table *symtab,
	Directive *directive,
	size_t program_counter) {

	if(!symtab) {
		set_error_message("Invalid symbol table provided to encoding function.");
		return NULL;
	}

	if(!directive) {
		set_error_message("Invalid directive provided to encoding function.");
		return NULL;
	}

	Encoding_Entity *encoded_entity = malloc(sizeof(Encoding_Entity));
	if(!encoded_entity) {
		set_error_message("Error allocating encoding entity.");
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


	char error_message[ERROR_MSG_MAX_LEN];
	char *directive_name = get_directive_string(*directive);

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
					set_error_message("Error allocating directive data.");
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
					set_error_message("Error allocating directive data.");
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
				set_error_message("Error allocating directive data.");
				return NULL;
			}

			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				// Create an array of each of the word operands.
				if(directive->opseq.operands[i].type == OPERAND_TYPE_SYMBOL) {
					Symbol *symbol = symtab_find_symbol(symtab,
						directive->opseq.operands[i].string_literal);
					if(!symbol) {
						sprintf(error_message, "Error finding symbol `%s`.",
							directive->opseq.operands[i].string_literal);
						set_error_message(error_message);
						return NULL;
					}

					word_data[i] = symbol->offset;
				} else if(directive->opseq.operands[i].type == OPERAND_TYPE_NUMERIC_LITERAL) {
					word_data[i] = directive->opseq.operands[i].numeric_literal;
				} else {
					sprintf(error_message, "Invalid operand type for `%s` directive.",
						directive_name);
					set_error_message(error_message);
					return NULL;
				}
			}

			data = malloc(total_len);
			if(!data) {
				set_error_message("Error allocating directive data.");
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
			set_error_message("Invalid non-encoded directive type.");
			return NULL;
		default:
			set_error_message("Unknown directive type.");
			return NULL;
	}

#if DEBUG_CODEGEN == 1
	printf("Produced output with len: `%lu`.\n", encoded_entity->size);
#endif

	return encoded_entity;

DIRECTIVE_OPERAND_COUNT_MISMATCH:
	sprintf(error_message, "Operand count mismatch for directive `%s`.", directive_name);
	set_error_message(error_message);
	return NULL;
}
