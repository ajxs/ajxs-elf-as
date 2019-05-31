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
	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


Encoding_Entity *encode_offset_type(uint8_t opcode,
	uint8_t rt,
	Operand offset_reg) {

	// Unlike GAS, this assembler currently does not support using symbols as an offset value.
	if(offset_reg.type != OPERAND_TYPE_REGISTER) {
		printf("Bad operand type for offset\n");
	}

	// Truncate to 16bits.
	uint16_t offset = offset_reg.offset & 0xFFFF;
	uint8_t base = encode_operand_register(offset_reg.value.reg);

	uint32_t encoding = opcode << 26;
	encoding |= base << 21;
	encoding |= rt << 16;
	encoding |= offset;

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


Encoding_Entity *encode_i_type(Symbol_Table *symtab,
	uint8_t opcode,
	uint8_t rs,
	uint8_t rt,
	Operand imm) {

	uint32_t encoding = opcode << 26;
	encoding |= rs << 21;
	encoding |= rt << 16;

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	uint32_t immediate = 0;
	if(imm.type == OPERAND_TYPE_NUMERIC_LITERAL) {
		immediate = imm.value.numeric_literal;
	} else if(imm.type == OPERAND_TYPE_SYMBOL) {
		Symbol *symbol = symtab_find_symbol(symtab, imm.value.symbol);

		// We only ever need one reloc entry in an I type?
		// @TODO: Verify.
		encoded_instruction->n_reloc_entries = 1;
		encoded_instruction->reloc_entries = malloc(sizeof(Reloc_Entry));
		// Add RELOC info.

		immediate = symbol->offset;
	} else {
		printf("BAD IMM TYPE: `%u` - `%u`\n", opcode, imm.type);
		return NULL;
	}

	encoding |= (immediate & 0xFFFF);

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
	encoded_instruction->data = memcpy(encoded_instruction->data, &encoding, 4);
	encoded_instruction->next = NULL;

	return encoded_instruction;
}


// Parses the immediate operand to a J type instruction.
// See: https://stackoverflow.com/questions/7877407/jump-instruction-in-mips-assembly#7877528
// https://stackoverflow.com/questions/6950230/how-to-calculate-jump-target-address-and-branch-target-address
Encoding_Entity *encode_j_type(Symbol_Table *symtab,
	uint8_t opcode,
	Operand imm) {
	uint32_t encoding = opcode << 26;

	Encoding_Entity *encoded_instruction = malloc(sizeof(Encoding_Entity));
	encoded_instruction->n_reloc_entries = 0;
	encoded_instruction->reloc_entries = NULL;

	uint32_t immediate = 0;
	if(imm.type == OPERAND_TYPE_NUMERIC_LITERAL) {
		immediate = imm.value.numeric_literal;
	} else if(imm.type == OPERAND_TYPE_SYMBOL) {
		Symbol *symbol = symtab_find_symbol(symtab, imm.value.symbol);

		// We only ever need one reloc entry in a J type?
		// @TODO: Verify.
		encoded_instruction->n_reloc_entries = 1;
		encoded_instruction->reloc_entries = malloc(sizeof(Reloc_Entry));
		// Add RELOC info.

		immediate = symbol->offset;
	} else {
		printf("BAD REG TYPE: `%u`\n", imm.type);
		return NULL;
	}

	immediate = (immediate & 0x0FFFFFFF) >> 2;

	// Truncate to 26bits.
	encoding |= (immediate & 0x7FFFFFF);

	encoded_instruction->size = 4;
	encoded_instruction->data = malloc(4);
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
	Instruction instruction,
	size_t program_counter) {

	/** The resulting encoding entity. */
	Encoding_Entity *encoded_entity = NULL;
	uint8_t rd = 0;
	uint8_t rs = 0;
	uint8_t rt = 0;
	uint8_t sa = 0;

	switch(instruction.opcode) {
		case OPCODE_ADD:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x20);
			break;
		case OPCODE_ADDI:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[1].value.reg);
			encoded_entity = encode_i_type(symtab, 0x8, rs, rt, instruction.opseq.operands[2]);
			break;
		case OPCODE_ADDIU:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[1].value.reg);
			encoded_entity = encode_i_type(symtab, 0x9, rs, rt, instruction.opseq.operands[2]);
			break;
		case OPCODE_ADDU:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x21);
			break;
		case OPCODE_AND:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x24);
			break;
		case OPCODE_ANDI:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[1].value.reg);
			encoded_entity = encode_i_type(symtab, 0xC, rs, rt, instruction.opseq.operands[2]);
			break;
		case OPCODE_BAL:
			if(!instruction_check_operand_length(1, instruction)) {
				return NULL;
			}

			encoded_entity = encode_i_type(symtab, 1, 0, 0x11, instruction.opseq.operands[2]);
			break;
		case OPCODE_BEQ:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[1].value.reg);
			encoded_entity = encode_i_type(symtab, 0x4, rs, rt, instruction.opseq.operands[2]);
			break;
		case OPCODE_BGEZ:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[1].value.reg);
			encoded_entity = encode_i_type(symtab, 0x14, rs, rt, instruction.opseq.operands[2]);
			break;
		case OPCODE_BNE:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[1].value.reg);
			encoded_entity = encode_i_type(symtab, 0x5, rs, rt, instruction.opseq.operands[2]);
			break;
		case OPCODE_J:
			if(!instruction_check_operand_length(1, instruction)) {
				return NULL;
			}

			encoded_entity = encode_j_type(symtab, 0x2, instruction.opseq.operands[0]);
			break;
		case OPCODE_JAL:
			if(!instruction_check_operand_length(1, instruction)) {
				return NULL;
			}

			encoded_entity = encode_j_type(symtab, 0x3, instruction.opseq.operands[0]);
			break;
		case OPCODE_JALR:
			if(!instruction_check_operand_length(1, instruction)) {
				return NULL;
			}

			if(instruction.opseq.n_operands == 1) {
				rd = 0x1F;
			} else {
				rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
				rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			}

			encoded_entity = encode_r_type(0, rd, rs, 0, 0, 0x9);
			break;
		case OPCODE_JR:
			if(!instruction_check_operand_length(1, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_r_type(0, 0, rs, 0, 0, 0x9);
			break;
		case OPCODE_LB:
			if(!instruction_check_operand_length(2, instruction)) {
				return NULL;
			}

			rt = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_offset_type(0x20, rt, instruction.opseq.operands[1]);
			break;
		case OPCODE_LBU:
			if(!instruction_check_operand_length(2, instruction)) {
				return NULL;
			}

			rt = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_offset_type(0x24, rt, instruction.opseq.operands[1]);
			break;
		case OPCODE_LUI:
			if(!instruction_check_operand_length(2, instruction)) {
				return NULL;
			}

			rt = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_i_type(symtab, 0xF, 0, rt, instruction.opseq.operands[1]);
			break;
		case OPCODE_LW:
			if(!instruction_check_operand_length(2, instruction)) {
				return NULL;
			}

			rt = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_offset_type(0x23, rt, instruction.opseq.operands[1]);
			break;
		case OPCODE_MUH:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x3, 0x18);
			break;
		case OPCODE_MUHU:
			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x3, 0x19);
			break;
		case OPCODE_MUL:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x2, 0x18);
			break;
		case OPCODE_MULU:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0x2, 0x19);
			break;
		case OPCODE_NOP:
			if(!instruction_check_operand_length(0, instruction)) {
				return NULL;
			}

			encoded_entity = encode_r_type(0, 0, 0, 0, 0, 0);
			break;
		case OPCODE_OR:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x25);
			break;
		case OPCODE_ORI:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rs = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[1].value.reg);
			encoded_entity = encode_i_type(symtab, 0xD, rs, rt, instruction.opseq.operands[2]);
			break;
		case OPCODE_SB:
			if(!instruction_check_operand_length(2, instruction)) {
				return NULL;
			}

			rt = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_offset_type(0x28, rt, instruction.opseq.operands[1]);
			break;
		case OPCODE_SH:
			if(!instruction_check_operand_length(2, instruction)) {
				return NULL;
			}

			rt = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_offset_type(0x29, rt, instruction.opseq.operands[1]);
			break;
		case OPCODE_SLL:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			sa = instruction.opseq.operands[2].value.numeric_literal;
			encoded_entity = encode_r_type(0, rd, 0, rt, sa, 0x0);
			break;
		case OPCODE_SUB:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x22);
			break;
		case OPCODE_SUBU:
			if(!instruction_check_operand_length(3, instruction)) {
				return NULL;
			}

			rd = encode_operand_register(instruction.opseq.operands[0].value.reg);
			rs = encode_operand_register(instruction.opseq.operands[1].value.reg);
			rt = encode_operand_register(instruction.opseq.operands[2].value.reg);
			encoded_entity = encode_r_type(0, rd, rs, rt, 0, 0x23);
			break;
		case OPCODE_SW:
			if(!instruction_check_operand_length(2, instruction)) {
				return NULL;
			}

			rt = encode_operand_register(instruction.opseq.operands[0].value.reg);
			encoded_entity = encode_offset_type(0x2B, rt, instruction.opseq.operands[1]);
			break;
		case OPCODE_SYSCALL:
			// @TODO: Investigate use of `code` field.
			encoded_entity = encode_r_type(0, 0, 0, 0, 0, 0xC);
			break;
		case OPCODE_UNKNOWN:
		default:
			printf("Unrecognised Opcode: `%i`\n", instruction.opcode);
			break;
	}

#if DEBUG_CODEGEN == 1
	printf("Debug Codegen: Encoded instruction `");
	print_opcode(instruction.opcode);
	printf("` at `0x%lx` as `0x%x`\n", program_counter, *(uint32_t *)encoded_entity->data);
#endif

	return encoded_entity;
}


Encoding_Entity *encode_directive(Symbol_Table *symtab,
	Directive *directive,
	size_t program_counter) {

	Encoding_Entity *encoded_entity = malloc(sizeof(Encoding_Entity));
	encoded_entity->next = NULL;

	size_t total_len = 0;
	size_t count = 0;
	size_t fill_size = 0;
	size_t string_len;
	uint8_t *data = NULL;
	size_t curr_pos = 0;

#if DEBUG_CODEGEN == 1
	printf("Debug Codegen: Encoding directive type `%u`...", directive->type);
#endif

	switch(directive->type) {
		case DIRECTIVE_ASCII:
			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				// Iterate through each string operand.
				string_len = strlen(directive->opseq.operands[i].value.string_literal);
				curr_pos = total_len;
				total_len += string_len;
				data = realloc(data, total_len);
				memcpy(data + curr_pos, &directive->opseq.operands[i].value.string_literal, string_len);
			}

			encoded_entity->size = total_len;
			encoded_entity->data = data;
			break;
		case DIRECTIVE_STRING:
		case DIRECTIVE_ASCIZ:
			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				string_len = strlen(directive->opseq.operands[i].value.string_literal);
				curr_pos = total_len;
				total_len += (1 + string_len);    // Take NUL terminator into account.

				data = realloc(data, total_len);
				memcpy(data + curr_pos, directive->opseq.operands[i].value.string_literal, string_len);

				// Add NUL terminator.
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
			total_len = sizeof(uint32_t) * directive->opseq.n_operands;
			uint32_t *word_data = malloc(total_len);
			for(size_t i = 0; i < directive->opseq.n_operands; i++) {
				// Create an array of each of the word operands.
				if(directive->opseq.operands[i].type == OPERAND_TYPE_SYMBOL) {
					Symbol *symbol =
							symtab_find_symbol(symtab, directive->opseq.operands[i].value.string_literal);

					if(!symbol) {
						printf("DEAD\n");
					}

					word_data[i] = symbol->offset;
				} else if(directive->opseq.operands[i].type == OPERAND_TYPE_NUMERIC_LITERAL) {
					word_data[i] = directive->opseq.operands[i].value.numeric_literal;
				} else {
					printf("Invaliddddddd\n");
				}
			}


			data = malloc(total_len);
			memcpy(data, word_data, total_len);
			encoded_entity->size = total_len;
			encoded_entity->data = data;

			free(word_data);
			break;
		case DIRECTIVE_ALIGN:
		case DIRECTIVE_BSS:
		case DIRECTIVE_DATA:
		case DIRECTIVE_GLOBAL:
		case DIRECTIVE_TEXT:
		case DIRECTIVE_UNKNOWN:
		default:
			return NULL;
	}

#if DEBUG_CODEGEN == 1
	printf("Produced output with len: `%lu`.\n", encoded_entity->size);
#endif

	return encoded_entity;
}
