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


Assembler_Status encode_directive(Encoding_Entity** encoded_directive,
	Symbol_Table* const symtab,
	Directive* const directive,
	const size_t program_counter) {

	// To avoid non-use warning. This is included as part of the function prototype
	// as it may be required for some directive.
	(void)program_counter;

	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to encoding function\n");
		return CODEGEN_ERROR_INVALID_ARGS;
	}

	if(!directive) {
		fprintf(stderr, "Error: Invalid directive provided to encoding function\n");
		return CODEGEN_ERROR_INVALID_ARGS;
	}

	*encoded_directive = malloc(sizeof(Encoding_Entity));
	if(!encoded_directive) {
		fprintf(stderr, "Error: Error allocating encoding entity\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	(*encoded_directive)->n_reloc_entries = 0;
	(*encoded_directive)->reloc_entries = NULL;
	(*encoded_directive)->next = NULL;

	size_t total_len = 0;
	size_t count = 0;
	size_t fill_size = 0;
	size_t string_len;
	uint8_t *data = NULL;
	size_t curr_pos = 0;

	const char *directive_name = get_directive_string(directive);
	if(!directive_name) {
		// cleanup.
		free(encoded_directive);

		fprintf(stderr, "Error: Unable to get directive type for `%i`\n",
			directive->type);
		return CODEGEN_ERROR_BAD_OPCODE;
	}

	switch(directive->type) {
		case DIRECTIVE_ASCII:
			if(directive->opseq.n_operands < 1) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				// Iterate through each string operand.
				string_len = strlen(directive->opseq.operands[i].string_literal);
				curr_pos = total_len;
				total_len += string_len;
				data = realloc(data, total_len);
				if(!data) {
					fprintf(stderr, "Error: Error allocating directive data\n");
					return ASSEMBLER_ERROR_BAD_ALLOC;
				}

				memcpy(data + curr_pos, &directive->opseq.operands[i].string_literal,
					string_len);
			}

			(*encoded_directive)->size = total_len;
			(*encoded_directive)->data = data;
			break;
		case DIRECTIVE_STRING:
		case DIRECTIVE_ASCIZ:
			if(directive->opseq.n_operands < 1) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				string_len = strlen(directive->opseq.operands[i].string_literal);
				curr_pos = total_len;
				total_len += (1 + string_len);    // Take NUL terminator into account.

				data = realloc(data, total_len);
				if(!data) {
					fprintf(stderr, "Error: Error allocating directive data\n");
					return CODEGEN_ERROR_BAD_ALLOC;
				}

				memcpy(data + curr_pos, directive->opseq.operands[i].string_literal, string_len);

				// Add NULL terminator.
				data[total_len - 1] = '\0';
			}

			(*encoded_directive)->size = total_len;
			(*encoded_directive)->data = data;
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
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			total_len = sizeof(uint32_t) * directive->opseq.n_operands;

			uint32_t *word_data = malloc(total_len);
			if(!word_data) {
				fprintf(stderr, "Error: Error allocating directive data.\n");
				return ASSEMBLER_ERROR_BAD_ALLOC;
			}

			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				// Create an array of each of the word operands.
				if(directive->opseq.operands[i].type == OPERAND_TYPE_SYMBOL) {
					Symbol *symbol = symtab_find_symbol(symtab,
						directive->opseq.operands[i].string_literal);
					if(!symbol) {
						// cleanup.
						free(word_data);

						fprintf(stderr, "Error: Error finding symbol `%s`\n",
							directive->opseq.operands[i].string_literal);
						return CODEGEN_ERROR_MISSING_SYMBOL;
					}

					word_data[i] = symbol->offset;
				} else if(directive->opseq.operands[i].type == OPERAND_TYPE_NUMERIC_LITERAL) {
					word_data[i] = directive->opseq.operands[i].numeric_literal;
				} else {
					// cleanup.
					free(word_data);
					return CODEGEN_ERROR_BAD_OPERAND_TYPE;
				}
			}

			data = malloc(total_len);
			if(!data) {
				// cleanup.
				free(word_data);

				fprintf(stderr, "Error: Error allocating directive data");
				return ASSEMBLER_ERROR_BAD_ALLOC;
			}

			memcpy(data, word_data, total_len);
			(*encoded_directive)->size = total_len;
			(*encoded_directive)->data = data;
			(*encoded_directive)->next = NULL;

			free(word_data);
			break;
		// Non-encoded directives.
		case DIRECTIVE_ALIGN:
		case DIRECTIVE_BSS:
		case DIRECTIVE_DATA:
		case DIRECTIVE_GLOBAL:
		case DIRECTIVE_TEXT:
		case DIRECTIVE_UNKNOWN:
			fprintf(stderr, "Error: Invalid non-encoded directive type\n");
			return CODEGEN_ERROR_BAD_OPCODE;
		default:
			fprintf(stderr, "Error: Unknown directive type\n");
			return CODEGEN_ERROR_BAD_OPCODE;
	}

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * @brief Encodes an I type instruction.
 *
 * Encodes an I-type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param symbol_table The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param opcode The operand encoding.
 * @param rs The rs field to encode.
 * @param rt The rt field to encode.
 * @param imm The imm operand to encode.
 * @param program_counter The current program_counter.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_i_type(Encoding_Entity** encoded_instruction,
	Symbol_Table* const symbol_table,
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
		Symbol *symbol = symtab_find_symbol(symbol_table, imm.symbol);
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

		(*encoded_instruction)->reloc_entries[0].symbol_name = imm.symbol;
		(*encoded_instruction)->reloc_entries[0].offset = program_counter;
		if(imm.flags.mask == OPERAND_MASK_HIGH) {
			// If this is the higher component of a symbol.
			// Most likely the result of a macro expansion. Refer to the macro
			// expansion logic for the relevant architecture.
			(*encoded_instruction)->reloc_entries[0].type = R_MIPS_HI16;
		} else if(imm.flags.mask == OPERAND_MASK_LOW) {
			(*encoded_instruction)->reloc_entries[0].type = R_MIPS_LO16;
		} else {
			(*encoded_instruction)->reloc_entries[0].type = R_MIPS_PC16;
		}
	} else {
		// cleanup.
		free(encoding);
		free(encoded_instruction);

		// If the immediate is of any other type, it is an error.
		fprintf(stderr, "Error: Bad operand type `%u` for immediate type instruction",
			imm.type);
		return CODEGEN_ERROR_BAD_OPERAND_TYPE;
	}

	*encoding |= immediate & 0xFFFF;

	(*encoded_instruction)->size = 4;
	(*encoded_instruction)->data = (uint8_t*)encoding;
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
	(*encoded_instruction)->data = (uint8_t*)encoding;
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
Assembler_Status encode_offset_type(Encoding_Entity** encoded_instruction,
	const uint8_t opcode,
	const uint8_t rt,
	const Operand offset_reg) {

	// Unlike GAS, this assembler currently does not support using symbols as an offset value.
	if(offset_reg.type != OPERAND_TYPE_REGISTER) {
		fprintf(stderr, "Error: Bad operand type `%u` for offset-type instruction",
			offset_reg.type);
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	*encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!*encoded_instruction) {
		fprintf(stderr, "Error: Error allocating encoded instruction\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	// Truncate to 16bits.
	uint16_t offset = offset_reg.offset & 0xFFFF;
	uint8_t base = encode_operand_register(offset_reg.reg);

	uint32_t* encoding = malloc(sizeof(uint32_t));
	if(!encoding) {
		// cleanup.
		free(encoded_instruction);

		fprintf(stderr, "Error: Error allocating encoded instruction data\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	*encoding = opcode << 26;
	*encoding |= base << 21;
	*encoding |= rt << 16;
	*encoding |= offset;

	(*encoded_instruction)->n_reloc_entries = 0;
	(*encoded_instruction)->reloc_entries = NULL;

	(*encoded_instruction)->size = 4;

	(*encoded_instruction)->data = (uint8_t*)encoding;
	(*encoded_instruction)->next = NULL;

	return ASSEMBLER_STATUS_SUCCESS;
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
Assembler_Status encode_j_type(Encoding_Entity** encoded_instruction,
	Symbol_Table* const symbol_table,
	const uint8_t opcode,
	const Operand imm,
	const size_t program_counter) {

	if(!symbol_table) {
		fprintf(stderr, "Error: Invalid symbol table provided to encoding function\n");
		return ASSEMBLER_ERROR_MISSING_SYMBOL;
	}

	*encoded_instruction = malloc(sizeof(Encoding_Entity));
	if(!*encoded_instruction) {
		fprintf(stderr, "Error: Error allocating encoded instruction\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	(*encoded_instruction)->n_reloc_entries = 0;
	(*encoded_instruction)->reloc_entries = NULL;

	uint32_t immediate = 0;
	if(imm.type == OPERAND_TYPE_NUMERIC_LITERAL) {
		immediate = imm.numeric_literal;
	} else if(imm.type == OPERAND_TYPE_SYMBOL) {
		Symbol *symbol = symtab_find_symbol(symbol_table, imm.symbol);

		(*encoded_instruction)->n_reloc_entries = 1;
		(*encoded_instruction)->reloc_entries = malloc(sizeof(Reloc_Entry));
		if(!(*encoded_instruction)->reloc_entries) {
			// cleanup.
			free(encoded_instruction);

			fprintf(stderr, "Error: Error allocating relocation entries\n");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		(*encoded_instruction)->reloc_entries[0].type = R_MIPS_26;
		(*encoded_instruction)->reloc_entries[0].symbol_name = symbol->name;
		(*encoded_instruction)->reloc_entries[0].offset = program_counter;

		immediate = symbol->offset;
	} else {
		// cleanup.
		free(encoded_instruction);

		fprintf(stderr, "Error: Bad operand type for jump type instruction");
		return ASSEMBLER_ERROR_BAD_OPERAND_TYPE;
	}

	immediate = (immediate & 0x0FFFFFFF) >> 2;


	uint32_t* encoding = malloc(sizeof(uint32_t));
	if(!encoding) {
		// cleanup.
		free(encoded_instruction);

		fprintf(stderr, "Error: Error allocating encoded instruction data\n");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	*encoding = opcode << 26;

	// Truncate to 26bits.
	*encoding |= (immediate & 0x7FFFFFF);

	(*encoded_instruction)->size = 4;
	(*encoded_instruction)->data = (uint8_t*)encoding;
	(*encoded_instruction)->next = NULL;

	return ASSEMBLER_STATUS_SUCCESS;
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
Assembler_Status encode_instruction(Encoding_Entity** encoded_instruction,
	Symbol_Table* const symtab,
	Instruction* const instruction,
	const size_t program_counter) {

	if(!symtab) {
		fprintf(stderr, "Error: Invalid symbol table provided to encoding function\n");
		return CODEGEN_ERROR_INVALID_ARGS;
	}

	if(!instruction) {
		fprintf(stderr, "Error: Invalid instruction provided to encoding function\n");
		return CODEGEN_ERROR_INVALID_ARGS;
	}

	uint8_t opcode = 0;
	uint8_t func = 0;
	uint8_t rd = 0;
	uint8_t rs = 0;
	uint8_t rt = 0;
	uint8_t sa = 0;

	Assembler_Status status = ASSEMBLER_STATUS_SUCCESS;

	switch(instruction->opcode) {
		case OPCODE_ADD:
		case OPCODE_ADDU:
		case OPCODE_AND:
		case OPCODE_MUH:
		case OPCODE_MUHU:
		case OPCODE_MUL:
		case OPCODE_MULU:
		case OPCODE_OR:
		case OPCODE_SUB:
		case OPCODE_SUBU:
			if(!check_operand_count(3, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			if(instruction->opcode == OPCODE_ADD) {
				func = 0x20;
			} else if(instruction->opcode == OPCODE_ADDU) {
				func = 0x21;
			} else if(instruction->opcode == OPCODE_AND) {
				func = 0x24;
			} else if(instruction->opcode == OPCODE_MUH) {
				sa = 0x3;
				func = 0x18;
			} else if(instruction->opcode == OPCODE_MUHU) {
				sa = 0x3;
				func = 0x19;
			} else if(instruction->opcode == OPCODE_MUL) {
				sa = 0x2;
				func = 0x18;
			} else if(instruction->opcode == OPCODE_MULU) {
				sa = 0x2;
				func = 0x19;
			} else if(instruction->opcode == OPCODE_OR) {
				func = 0x25;
			} else if(instruction->opcode == OPCODE_SUB) {
				func = 0x22;
			} else if(instruction->opcode == OPCODE_SUBU) {
				func = 0x23;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			rs = encode_operand_register(instruction->opseq.operands[1].reg);
			rt = encode_operand_register(instruction->opseq.operands[2].reg);
			status = encode_r_type(encoded_instruction, opcode, rd, rs, rt, 0, func);
			break;
		case OPCODE_ADDI:
		case OPCODE_ADDIU:
		case OPCODE_ANDI:
		case OPCODE_BEQ:
		case OPCODE_BGEZ:
		case OPCODE_BNE:
		case OPCODE_ORI:
			if(!check_operand_count(3, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			if(instruction->opcode == OPCODE_ADDI) {
				opcode = 0x8;
			} else if(instruction->opcode == OPCODE_ADDIU) {
				opcode = 0x9;
			} else if(instruction->opcode == OPCODE_ANDI) {
				opcode = 0xC;
			} else if(instruction->opcode == OPCODE_BEQ) {
				opcode = 0x4;
			} else if(instruction->opcode == OPCODE_BGEZ) {
				opcode = 0x14;
			} else if(instruction->opcode == OPCODE_BNE) {
				opcode = 0x5;
			} else if(instruction->opcode == OPCODE_ORI) {
				opcode = 0xD;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			rt = encode_operand_register(instruction->opseq.operands[1].reg);

			status = encode_i_type(encoded_instruction, symtab, opcode, rs, rt,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_LB:
		case OPCODE_LBU:
		case OPCODE_LW:
		case OPCODE_SB:
		case OPCODE_SH:
		case OPCODE_SW:
			if(instruction->opcode == OPCODE_LB) {
				opcode = 0x20;
			} else if(instruction->opcode == OPCODE_LBU) {
				opcode = 0x24;
			} else if(instruction->opcode == OPCODE_LW) {
				opcode = 0x23;
			} else if(instruction->opcode == OPCODE_SB) {
				opcode = 0x28;
			} else if(instruction->opcode == OPCODE_SH) {
				opcode = 0x29;
			} else if(instruction->opcode == OPCODE_SW) {
				opcode = 0x2B;
			}

			if(!check_operand_count(2, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			status = encode_offset_type(encoded_instruction, opcode, rt,
				instruction->opseq.operands[1]);
			break;
		case OPCODE_BAL:
			if(!check_operand_count(1, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			status = encode_i_type(encoded_instruction, symtab, 1, 0, 0x11,
				instruction->opseq.operands[2], program_counter);
			break;
		case OPCODE_J:
		case OPCODE_JAL:
			if(!check_operand_count(1, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			if(instruction->opcode == OPCODE_J) {
				opcode = 0x2;
			} else if(instruction->opcode == OPCODE_J) {
				opcode = 0x3;
			}

			status = encode_j_type(encoded_instruction, symtab, opcode,
				instruction->opseq.operands[0], program_counter);
			break;
		case OPCODE_JALR:
			if(!check_operand_count(1, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			if(instruction->opseq.n_operands == 1) {
				rd = 0x1F;
			} else {
				rd = encode_operand_register(instruction->opseq.operands[0].reg);
				rs = encode_operand_register(instruction->opseq.operands[1].reg);
			}

			status = encode_r_type(encoded_instruction, 0, rd, rs, 0, 0, 0x9);
			break;
		case OPCODE_JR:
			if(!check_operand_count(1, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			rs = encode_operand_register(instruction->opseq.operands[0].reg);
			status = encode_r_type(encoded_instruction, 0, 0, rs, 0, 0, 0x9);
			break;
		case OPCODE_LUI:
			if(!check_operand_count(2, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			rt = encode_operand_register(instruction->opseq.operands[0].reg);
			status = encode_i_type(encoded_instruction, symtab, 0xF, 0, rt,
				instruction->opseq.operands[1], program_counter);
			break;
		case OPCODE_MULT:
		case OPCODE_MULTU:
			fprintf(stderr, "Instruction deprecated in `MIPS32r6`\n");
			status = CODEGEN_ERROR_DEPRECATED_OPCODE;
			break;
		case OPCODE_NOP:
			if(!check_operand_count(0, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			status = encode_r_type(encoded_instruction, 0, 0, 0, 0, 0, 0);
			break;
		case OPCODE_SLL:
			if(!check_operand_count(3, &instruction->opseq)) {
				return CODEGEN_ERROR_OPERAND_COUNT_MISMATCH;
			}

			rd = encode_operand_register(instruction->opseq.operands[0].reg);
			sa = instruction->opseq.operands[2].numeric_literal;
			status = encode_r_type(encoded_instruction, 0, rd, 0, rt, sa, 0x0);
			break;
		case OPCODE_SYSCALL:
			// @TODO: Investigate use of `code` field.
			status = encode_r_type(encoded_instruction, 0, 0, 0, 0, 0, 0xC);
			break;
		case OPCODE_UNKNOWN:
		default:
			fprintf(stderr, "Error: Unrecognised Opcode\n");
			return CODEGEN_ERROR_BAD_OPCODE;
	}

	return status;
}


char* get_encoding_as_string(Encoding_Entity* encoded_instruction) {
	char *representation = malloc(ERROR_MSG_MAX_LEN);
	sprintf(representation, "0x%x", (uint32_t)encoded_instruction->data);

	return representation;
}
