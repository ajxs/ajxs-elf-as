/**
 * @file as.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Main program header. Architecture specific information is included from here.
 * Contains non-architecture specific type definitions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef AS_H
#define AS_H 1

#include <elf.h>
#include <statement.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <section.h>
#include <symtab.h>


// These control whether or not debug information specific to certain processes
// is printed to STDOUT.
#define DEBUG_ASSEMBLER 1
#define DEBUG_CODEGEN 1
#define DEBUG_INPUT 0
#define DEBUG_MACRO 1
#define DEBUG_OUTPUT 1
#define DEBUG_PARSED_STATEMENTS 1
#define DEBUG_SYMBOLS 1

#define ERROR_MSG_MAX_LEN 512


/**
 * @brief The result of an assembler process.
 *
 * Indicates the result of an assembler process. The assembler process functions
 * do not return an actual value, so this is used to track the success of
 * the operations.
 */
typedef enum e_assembler_status {
	ASSEMBLER_STATUS_SUCCESS,
	ASSEMBLER_STATUS_BAD_INPUT,
	ASSEMBLER_ERROR_BAD_ALLOC,
	ASSEMBLER_ERROR_BAD_OPERAND_TYPE,
	ASSEMBLER_ERROR_BAD_FUNCTION_ARGS,
	ASSEMBLER_ERROR_BAD_SECTION_DATA,
	ASSEMBLER_ERROR_CODEGEN_FAILURE,
	ASSEMBLER_ERROR_FILE_FAILURE,
	ASSEMBLER_ERROR_MACRO_EXPANSION,
	ASSEMBLER_ERROR_MISSING_SECTION,
	ASSEMBLER_ERROR_MISSING_SYMBOL,
	ASSEMBLER_ERROR_PREPROCESSING_FAILURE,
	ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE,
	ASSEMBLER_ERROR_STATEMENT_SIZE,
	ASSEMBLER_ERROR_SYMBOL_ENTITY_FAILURE,
	CODEGEN_ERROR_BAD_ALLOC,
	CODEGEN_ERROR_BAD_OPCODE,
	CODEGEN_ERROR_BAD_OPERAND_TYPE,
	CODEGEN_ERROR_INVALID_ARGS,
	CODEGEN_ERROR_OPERAND_COUNT_MISMATCH,
	CODEGEN_ERROR_MISSING_SECTION,
	CODEGEN_ERROR_MISSING_SYMBOL,
	CODEGEN_ERROR_DEPRECATED_OPCODE
} Assembler_Status;

Assembler_Status assemble(const char* input_filename,
	const char* output_filename,
	const bool verbose);

Assembler_Status assemble_first_pass(Section* sections,
	Symbol_Table* symbol_table,
	Statement* statements);

Assembler_Status assemble_second_pass(Section* sections,
	Symbol_Table* symbol_table,
	Statement* statements);

Elf32_Ehdr* create_elf_header(void);

Elf32_Shdr* encode_section_header(Section* section);

Assembler_Status encode_directive(Encoding_Entity** encoded_directive,
	Symbol_Table* const symtab,
	Directive* const directive,
	const size_t program_counter);

Assembler_Status encode_instruction(Encoding_Entity** encoded_instruction,
	Symbol_Table* const symbol_table,
	Instruction* const instruction,
	const size_t program_counter);

Assembler_Status expand_macros(Statement* statements);

char* get_encoding_as_string(Encoding_Entity* encoded_instruction);

Assembler_Status initialise_sections(Section** sections);

Assembler_Status populate_symtab(Section* sections,
	Symbol_Table* symbol_table);

#endif
