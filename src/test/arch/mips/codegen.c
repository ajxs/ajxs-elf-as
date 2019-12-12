#include <CUnit/CUnit.h>
#include <CUnit/CUError.h>
#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdint.h>
#include <as.h>
#include <arch.h>
#include <codegen.h>
#include <operand.h>
#include <section.h>
#include <stdlib.h>
#include <symtab.h>
#include <test.h>

int init_codegen_test_suite(void) {
	return 0;
}


int teardown_codegen_test_suite(void) {
	return 0;
}


void test_encode_i_type(void) {
	/** The executable symbol table. */
	Symbol_Table symbol_table;
	uint8_t opcode = 0;
	uint8_t rs = 0;
	uint8_t rt = 0;
	Operand imm;
	size_t program_counter = 0;
	Encoding_Entity* encoded_instruction = NULL;
	Assembler_Status status;

	// Initialise with room for the null symbol entry.
	symbol_table.n_entries = 1;
	symbol_table.symbols = malloc(sizeof(Symbol));

	// Create the null symbol entry.
	// This is required as per ELF specification.
	symbol_table.symbols[0].section = NULL;
	symbol_table.symbols[0].offset = 0;

	// Create an empty name entry, so as to not disrupt other processes that
	// require handling of this string.
	symbol_table.symbols[0].name = malloc(1);
	symbol_table.symbols[0].name[0] = '\0';

	// ADDI $t1, $t0, 0x50
	opcode = 0x8;
	rs = encode_operand_register(REGISTER_$T0);
	rt = encode_operand_register(REGISTER_$T1);
	imm.type = OPERAND_TYPE_NUMERIC_LITERAL;
	imm.numeric_literal = 0x50;

	status = encode_i_type(&encoded_instruction,
		&symbol_table,
		opcode,
		rs,
		rt,
		imm,
		program_counter);

	CU_ASSERT(*(uint32_t*)encoded_instruction->data == 0x21090050);
	CU_ASSERT(status == ASSEMBLER_STATUS_SUCCESS);

	// ORI $t3, $a0, 0x50
	opcode = 13;
	rs = encode_operand_register(REGISTER_$A0);
	rt = encode_operand_register(REGISTER_$T3);
	imm.type = OPERAND_TYPE_NUMERIC_LITERAL;
	imm.numeric_literal = 0x50;

	status = encode_i_type(&encoded_instruction,
		&symbol_table,
		opcode,
		rs,
		rt,
		imm,
		program_counter);

	CU_ASSERT(*(uint32_t*)encoded_instruction->data == 0x348B0050);
	CU_ASSERT(status == ASSEMBLER_STATUS_SUCCESS);

	free_encoding_entity(encoded_instruction);

	free_symbol_table(&symbol_table);
}


void test_encode_j_type(void) {

}


void test_encode_r_type(void) {
	Encoding_Entity* encoded_instruction = NULL;
	Assembler_Status status;
	uint8_t opcode = 0;
	uint8_t func = 0;
	uint8_t rd = 0;
	uint8_t rs = 0;
	uint8_t rt = 0;

	// ADD, $t0, $t0, $t1
	opcode = 0;
	func = 0x20;
	rd = encode_operand_register(REGISTER_$T0);
	rs = encode_operand_register(REGISTER_$T0);
	rt = encode_operand_register(REGISTER_$T1);

	status = encode_r_type(&encoded_instruction,
		opcode, rd, rs, rt, 0, func);

	CU_ASSERT(*(uint32_t*)encoded_instruction->data == 0x1094020);
	CU_ASSERT(status == ASSEMBLER_STATUS_SUCCESS);

	free_encoding_entity(encoded_instruction);

	// SUB, $t1, $t1, $t0
	opcode = 0;
	func = 0x22;
	rd = encode_operand_register(REGISTER_$T1);
	rs = encode_operand_register(REGISTER_$T1);
	rt = encode_operand_register(REGISTER_$T0);

	status = encode_r_type(&encoded_instruction,
		opcode, rd, rs, rt, 0, func);

	CU_ASSERT(*(uint32_t*)encoded_instruction->data == 0x1284822);
	CU_ASSERT(status == ASSEMBLER_STATUS_SUCCESS);

	free_encoding_entity(encoded_instruction);
}
