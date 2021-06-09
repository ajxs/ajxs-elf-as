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


#include <elf.h>
#include <encoding_entity.h>
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
#define DEBUG_INPUT 1
#define DEBUG_MACRO 1
#define DEBUG_OUTPUT 1
#define DEBUG_PARSED_STATEMENTS 1
#define DEBUG_PREPROCESSOR 0
#define DEBUG_SYMBOLS 1


/**
 * @brief The main assembler entry point.
 *
 * This function begins the assembly process for an input source file.
 * All processing and assembly is initiated here.
 * @param input_filename The file path for the input source file.
 * @param output_filename The file path for the output source file.
 * @return A status code indicating the success status of the operation
*/
Assembler_Status assemble(const char* input_filename,
	const char* output_filename,
	const bool verbose);

/**
 * @brief Creates the ELF file header.
 *
 * This function creates an ELF executable file header specific for this
 * particular architecture.
 * @param elf_header A pointer-to-pointer to the header to be created.
 * @return A status code indicating the result of the operation.
 */
Assembler_Status create_elf_header(Elf32_Ehdr** elf_header);

/**
 * @brief Encodes an ELF section header.
 *
 * This function encodes an ELF section header from an application
 * section entity.
 * @param section A pointer to the application section entity.
 * @param section_header A pointer-to-pointer to the section header to be
 * created.
 * @return The status of process.
 */
Assembler_Status encode_section_header(const Section* section,
	Elf32_Shdr** section_header);

/**
 * @brief Encodes a Directive entity.
 *
 * Encodes an Directive entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param encoded_directive A pointer-to-pointer to the resulting encoded deirective.
 * @param symtab The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param instruction The parsed instruction entity to encode.
 * @param program_counter The current program counter. This represents the current
 * place of the instruction within the current encoding context, which is the
 * current program section.
 * @return The status of the operation.
 */
Assembler_Status encode_directive(Encoding_Entity** encoded_directive,
	const Symbol_Table* symtab,
	const Directive* directive,
	const size_t program_counter);

/**
 * @brief Encodes an Instruction entity.
 *
 * Encodes an instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param encoded_directive A pointer-to-pointer to the resulting encoded deirective.
 * @param symtab The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param instruction The parsed instruction entity to encode.
 * @param program_counter The current program counter. This represents the current
 * place of the instruction within the current encoding context, which is the
 * current program section.
 * @return The status of the operation.
 */
Assembler_Status encode_instruction(Encoding_Entity** encoded_instruction,
	const Symbol_Table* symbol_table,
	const Instruction* instruction,
	const size_t program_counter);

/**
 * @brief Expands all of the macro statements in the program.
 *
 * This function iterates through all of the program statements, checking whether
 * each one is a macro. Any macros encountered will be 'expanded'. This process
 * typically involves the modification of the macro statement itself, as well as
 * potentially appending further statements to it. This is accomplished by adding
 * a new link to the `statements` linked list.
 * @param statements The linked list of parsed statements.
 * @returns The result of the operation.
 * @warning @p statements is modified by this function.
 */
Assembler_Status expand_macros(Statement* statements);

/**
 * @brief Gets a string representation of an encoded instruction.
 * 
 * This function creates a string representation of an encoded instruction. Used for
 * debugging purposes.
 * @param encoded_instruction The instruction to get the representation of.
 * @returns A string literal containing the representation.
 */
char* get_encoding_as_string(const Encoding_Entity* encoded_instruction);

/**
 * @brief tests a result status for success.
 *
 * This functions tests an assembler result status for success. Returns true on
 * success, false in any other state.
 * @param status The status to test.
 * @return True if the status is successful, false in any other state.
 */
bool get_status(const Assembler_Status status);

/**
 * @brief Creates and initialises the executable sections.
 *
 * This function creates all of the sections required to generate a relocatable
 * ELF file. This will create all of the sections, as well as their relocation
 * entry sections.
 * Creates a linked list of the sections.
 * @param sections A pointer-to-pointer to the section data.
 * @return A status result object showing the result of the process.
 */
Assembler_Status initialise_sections(Section** sections);

/**
 * @brief Populates the ELF symbol table.
 *
 * This function parses through the program symbol table and encodes the necessary
 * ELF entities to write to the final assembled ELF file.
 * This function will add all of the necessary encoded entities to the symbol
 * table and string table sections.
 * @param sections A pointer to the section linked list.
 * @param symbol_table A pointer to the symbol table.
 * @warning This function modifies the sections.
 * @return A status entity indicating whether or not the pass was successful.
 */
Assembler_Status populate_symtab(const Section* sections,
	const Symbol_Table* symbol_table);

#endif
