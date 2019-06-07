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
#include <arch.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// These control whether or not debug information specific to certain processes
// is printed to STDOUT.
#define DEBUG_ASSEMBLER 1
#define DEBUG_CODEGEN 1
#define DEBUG_INPUT 0
#define DEBUG_MACRO 1
#define DEBUG_OUTPUT 1
#define DEBUG_PARSED_STATEMENTS 1
#define DEBUG_SYMBOLS 1


typedef enum { OPERAND_MASK_NONE, OPERAND_MASK_HIGH, OPERAND_MASK_LOW } Operand_Mask;


typedef struct {
	uint16_t shift;
	Operand_Mask mask;
} Operand_Flags;


static const Operand_Flags DEFAULT_OPERAND_FLAGS = {0, OPERAND_MASK_NONE};


typedef enum {
	OPERAND_TYPE_UNKNOWN,
	OPERAND_TYPE_SYMBOL,
	OPERAND_TYPE_NUMERIC_LITERAL,
	OPERAND_TYPE_STRING_LITERAL,
	OPERAND_TYPE_REGISTER,
} Operand_Type;


typedef struct {
	Operand_Flags flags;
	Operand_Type type;
	uint16_t offset;
	union _operand_value {
		char *string_literal;
		char *symbol;
		uint32_t numeric_literal;
		Register reg;
	} value;
} Operand;


typedef struct {
	size_t n_operands;
	Operand *operands;
} Operand_Sequence;


typedef struct {
	Instruction_Type type;
	Opcode opcode;
	Operand_Sequence opseq;
} Instruction;


typedef enum {
	DIRECTIVE_UNKNOWN,
	DIRECTIVE_ALIGN,
	DIRECTIVE_ASCII,
	DIRECTIVE_ASCIZ,
	DIRECTIVE_BSS,
	DIRECTIVE_BYTE,
	DIRECTIVE_DATA,
	DIRECTIVE_FILL,
	DIRECTIVE_GLOBAL,
	DIRECTIVE_LONG,
	DIRECTIVE_SHORT,
	DIRECTIVE_SIZE,
	DIRECTIVE_SKIP,
	DIRECTIVE_SPACE,
	DIRECTIVE_STRING,
	DIRECTIVE_TEXT,
	DIRECTIVE_WORD
} Directive_Type;


typedef struct {
	Directive_Type type;
	Operand_Sequence opseq;
} Directive;


typedef enum {
	STATEMENT_TYPE_EMPTY,
	STATEMENT_TYPE_DIRECTIVE,
	STATEMENT_TYPE_INSTRUCTION,
} Statement_Type;


typedef struct statement {
	size_t n_labels;
	char **labels;
	Statement_Type type;
	union _statement_body {
		Instruction instruction;
		Directive directive;
	} body;
	size_t line_num;
	struct statement *next;
} Statement;


typedef struct {
	char *section;
	size_t offset;
	uint32_t type;
} Reloc_Entry;


typedef struct _encoding_entity {
	size_t address;
	size_t size;
	uint8_t *data;
	size_t n_reloc_entries;
	Reloc_Entry *reloc_entries;
	struct _encoding_entity *next;
} Encoding_Entity;


typedef struct _section {
	char *name;
	size_t name_strtab_offset;
	size_t file_offset;
	size_t program_counter;
	uint32_t type;
	uint32_t flags;
	size_t size;
	size_t info;
	size_t link;
	Encoding_Entity *encoding_entities;
	struct _section *next;
} Section;


typedef struct {
	char *name;
	Section *section;
	size_t offset;
} Symbol;


typedef struct {
	size_t n_entries;
	Symbol *symbols;
} Symbol_Table;


Register parse_register_symbol(char *register_symbol);
Opcode parse_opcode_symbol(char *opcode_symbol);
Directive_Type parse_directive_symbol(char *directive_symbol);

void print_operand(Operand op);
void print_operand_sequence(Operand_Sequence opseq);
void print_instruction(Instruction inst);
void print_directive(Directive dir);
void print_directive_type(Directive dir);
void print_statement(Statement *statement);
void print_opcode(Opcode op);

bool instruction_check_operand_length(size_t expected_operand_length,
	Instruction instruction);


Statement *scan_string(const char *str);


char *preprocess_line(char *line_buffer);


void free_operand(Operand *op);
void free_operand_sequence(Operand_Sequence *opseq);
void free_directive(Directive *directive);
void free_instruction(Instruction *instruction);
void free_statement(Statement *statement);
void free_section(Section *section);
void free_encoding_entity(Encoding_Entity *encoding_entity);

Symbol *symtab_find_symbol(Symbol_Table *symtab,
	char *label);

void expand_macros(Statement *statements);

Encoding_Entity *encode_instruction(Symbol_Table *symbol_table,
	Instruction instruction,
	size_t program_counter);

Encoding_Entity *encode_directive(Symbol_Table *symtab,
	Directive *directive,
	size_t program_counter);

ssize_t get_statement_size(Statement *statement);

void assemble(const char *input_filename,
	const char *output_filename);

void assemble_first_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements);

void assemble_second_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements);

void symtab_add_symbol(Symbol_Table *symtab,
	char *name,
	Section *section,
	size_t offset);

Symbol *symtab_find_symbol(Symbol_Table *symtab,
	char *name);

Section *create_section(char *name,
	uint32_t type,
	uint32_t flags);

void add_section(Section **section_list,
	Section *section);

Section *find_section(Section *section_list,
	const char *name);

ssize_t find_section_index(Section *section_list,
	const char *name);

void section_add_encoding_entity(Section *section,
	Encoding_Entity *entity);

void free_encoding_entity(Encoding_Entity *entity);

Elf32_Ehdr *create_elf_header(void);
Elf32_Shdr *encode_section_header(Section *section);

#endif
