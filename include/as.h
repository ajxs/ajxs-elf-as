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
 * @brief The result of a macro expansion.
 *
 * Indicates the result of the expansion of a macro. The expansion functions
 * do not return an actual value, so this is used to track the success of
 * the expansion operation.
 */
typedef enum _expand_macro_status_result {
	EXPAND_MACRO_FAILURE,
	EXPAND_MACRO_SUCCESS,
} Expand_Macro_Result_Status;


typedef enum _codegen_status {
	CODEGEN_SUCCESS,
	CODEGEN_ERROR_BAD_ALLOC,
	CODEGEN_ERROR_BAD_OPCODE,
	CODEGEN_ERROR_INVALID_ARGS,
	CODEGEN_ERROR_OPERAND_COUNT_MISMATCH,
	CODEGEN_ERROR_MISSING_SECTION,
	CODEGEN_ERROR_MISSING_SYMBOL
} Codegen_Status_Result;


/**
 * @brief The result of an assembler process.
 *
 * Indicates the result of an assembler process. The assembler process functions
 * do not return an actual value, so this is used to track the success of
 * the operations.
 */
typedef enum _assemble_pass_status {
	ASSEMBLER_PROCESS_SUCCESS,
	ASSEMBLER_ERROR_BAD_ALLOC,
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
	ASSEMBLER_ERROR_SYMBOL_ENTITY_FAILURE
} Assembler_Process_Result;





Assembler_Process_Result initialise_sections(Section **sections);

Assembler_Process_Result populate_symtab(Section *sections,
	Symbol_Table *symbol_table);

Assembler_Process_Result assemble(const char *input_filename,
	const char *output_filename,
	bool verbose);

Assembler_Process_Result assemble_first_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements);

Assembler_Process_Result assemble_second_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements);

Assembler_Process_Result expand_macros(Statement *statements);

Elf32_Ehdr *create_elf_header(void);

Elf32_Shdr *encode_section_header(Section *section);

Codegen_Status_Result encode_instruction(Encoding_Entity **encoded_instruction,
	Symbol_Table *symbol_table,
	Instruction *instruction,
	size_t program_counter);

#endif
