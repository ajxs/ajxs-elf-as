/**
 * @file as.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Main program.
 * Contains the main assembler process functions. The actual assembly logic is
 * contained within this file.
 * The `assemble` function is where the process begins.
 * @version 0.1
 * @date 2019-03-09
 */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <directive.h>
#include <input.h>
#include <instruction.h>
#include <section.h>
#include <statement.h>
#include <symtab.h>

/**
 * @brief Populates the relocation entry sections.
 *
 * This function populates the sections specific to relocation entries. Each
 * statement for which code has been generated is parsed, and any relocation
 * entries generated are encoded in the correct ELF format and added to their
 * relevant relocation entry section.
 * For more information on relocation entries, refer to:
 * https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-54839.html
 * @param sections A pointer to the section linked list.
 * @warning This function modifies the sections.
 */
static Assembler_Status populate_relocation_entries(Symbol_Table* symtab,
	Section* sections);


/**
 * @brief Runs the first pass of the assembler.
 *
 * This function runs the first assembly pass. This pass calculates the size of
 * each instruction, and populates the symbol table with all of the labels.
 * Creates a linked list of the sections.
 * @param sections A pointer to the section linked list.
 * @param symbol_table A pointer to the symbol table.
 * @param statements A pointer to the parsed statement linked list.
 * @warning This function modifies the symbol table.
 * @return A status entity indicating whether or not the pass was successful.
 */
static Assembler_Status assemble_first_pass(Section* sections,
	Symbol_Table* symbol_table,
	Statement* statements);

/**
 * @brief Runs the second pass of the assembler.
 *
 * This function runs the second assembly pass. This pass generates the code for
 * each parsed instruction and populates the section data.
 * @param sections A pointer to the section linked list.
 * @param symbol_table A pointer to the symbol table.
 * @param statements A pointer to the parsed statement linked list.
 * @warning This function modifies the sections.
 * @return A status entity indicating whether or not the pass was successful.
 */
static Assembler_Status assemble_second_pass(Section* sections,
	Symbol_Table* symbol_table,
	Statement* statements);

/**
 * assemble_first_pass
 */
static Assembler_Status assemble_first_pass(Section* sections,
	Symbol_Table* symbol_table,
	Statement* statements)
{
	/** The status of internal assembler function calls. */
	Assembler_Status status = ASSEMBLER_STATUS_SUCCESS;
	/** The text section. */
	Section* section_text = NULL;
	/** The program data section. */
	Section* section_data = NULL;
	/** The program bss section. */
	Section* section_bss = NULL;
	/** Pointer to the current section being parsed. */
	Section* curr_section = NULL;
	/** Pointer to the current statement being parsed. */
	Statement* curr = NULL;
	/** The encoded size of the statement. */
	size_t statement_size = 0;


#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Begin first pass\n");
#endif

	section_text = find_section(sections, ".text");
	if(!section_text) {
		fprintf(stderr, "Unable to locate .text section\n");

		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_data = find_section(sections, ".data");
	if(!section_data) {
		fprintf(stderr, "Unable to locate .data section\n");

		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_bss = find_section(sections, ".bss");
	if(!section_bss) {
		fprintf(stderr, "Unable to locate .bss section\n");

		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	// Start in the .text section by default.
	curr_section = section_text;

	curr = statements;
	while(curr) {
		// All labels must be processed first.
		// Since a label _can_ precede a section directive, but not the other way around.
		if(curr->labels) {
			for(size_t i = 0; i < curr->n_labels; i++) {
				Symbol* added_sybmol = symtab_add_symbol(symbol_table, curr->labels[i], curr_section,
					curr_section->program_counter);
				if(!added_sybmol) {
					// Error should already have been set.
					return ASSEMBLER_ERROR_SYMBOL_ENTITY_FAILURE;
				}
			}
		}

		// Process section directives.
		// These are directives which specify which section to place the following
		// statements in. Adjust the current section accordingly.
		// These have a size of zero, as returned from `get_statement_size`.
		if(curr->type == STATEMENT_TYPE_DIRECTIVE) {
			if(curr->directive.type == DIRECTIVE_BSS) {
				curr_section = section_bss;
			} else if(curr->directive.type == DIRECTIVE_DATA) {
				curr_section = section_data;
			} else if(curr->directive.type == DIRECTIVE_TEXT) {
				curr_section = section_text;
			}
		}

		// Get the size of the statement.
		get_statement_size(curr, &statement_size);
		if(!get_status(status)) {
			// Error will already have been printed.
			return ASSEMBLER_ERROR_STATEMENT_SIZE;
		}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Calculated size `0x%lx` for statement.\n", statement_size);
#endif

		// Increment the current section's program counter by the size of the
		// statement that has been computed.
		curr_section->program_counter += (size_t)statement_size;
		curr = curr->next;
	}


#if DEBUG_SYMBOLS == 1
	// Print the symbol table.
	printf("Debug Assembler: Symbol Table:\n");
	print_symbol_table(symbol_table);
#endif

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * assemble_second_pass
 */
static Assembler_Status assemble_second_pass(Section* sections,
	Symbol_Table* symbol_table,
	Statement* statements)
{
	/** The status of the encoding pass. */
	Assembler_Status status = ASSEMBLER_STATUS_SUCCESS;
	/** Pointer to the current section being encoded. */
	Section* curr_section = NULL;
	/** Pointer to the program text section. */
	Section* section_text = NULL;
	/** Pointer to the program data section. */
	Section* section_data = NULL;
	/** Pointer to the program bss section. */
	Section* section_bss = NULL;
	/** Pointer to the current statement being encoded. */
	Statement* curr = NULL;
	/** Pointer to the current encoding entity being encoded. */
	Encoding_Entity* encoding = NULL;
	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity* added_entity = NULL;

	if(!sections) {
		fprintf(stderr, "Invalid section data\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	if(!symbol_table) {
		fprintf(stderr, "Invalid symbol table data\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	if(!statements) {
		fprintf(stderr, "Invalid statement data\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	// Ensure all section program counters counters are reset.
	// These will have been set by the first assembly pass.
	curr_section = sections;
	while(curr_section) {
		curr_section->program_counter = 0;
		curr_section = curr_section->next;
	}

	section_text = find_section(sections, ".text");
	if(!section_text) {
		fprintf(stderr, "Unable to locate .text section\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_data = find_section(sections, ".data");
	if(!section_data) {
		fprintf(stderr, "Unable to locate .data section\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_bss = find_section(sections, ".bss");
	if(!section_bss) {
		fprintf(stderr, "Unable to locate .bss section\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	// Start in the .text section by default.
	curr_section = section_text;

	// Iterate over all statements.
	curr = statements;
	while(curr) {
		if(curr->type == STATEMENT_TYPE_DIRECTIVE) {
			const char* directive_name = get_directive_string(&curr->directive);
			if(!directive_name) {
				fprintf(stderr, "Error: Unable to get directive type for `%i`\n",
					curr->directive.type);
				return CODEGEN_ERROR_BAD_OPCODE;
			}

			switch(curr->directive.type) {
				case DIRECTIVE_BSS:
#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Setting current section to `.bss`\n");
#endif
					curr_section = section_bss;
					break;
				case DIRECTIVE_DATA:
#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Setting current section to `.data`\n");
#endif
					curr_section = section_data;
					break;
				case DIRECTIVE_TEXT:
#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Setting current section to `.text`\n");
#endif
					curr_section = section_text;
					break;
				case DIRECTIVE_ALIGN:
				case DIRECTIVE_SIZE:
				case DIRECTIVE_GLOBAL:
					// These entities are not directly encoded.
					// They represent instructions to the assembler which do not result
					// in encoded binary entities.
					break;
				default:
					status = encode_directive(&encoding, symbol_table, &curr->directive,
						curr_section->program_counter);
					if(!get_status(status)) {
						if(status == CODEGEN_ERROR_OPERAND_COUNT_MISMATCH) {
							fprintf(stderr, "Error: Operand count mismatch for `%s` directive\n",
								directive_name);

							return status;
						} else if(status == CODEGEN_ERROR_BAD_OPERAND_TYPE) {
							fprintf(stderr, "Error: Invalid operand type for `%s` directive\n",
								directive_name);
						}

						// Error message should already be set in the encode function.
						return ASSEMBLER_ERROR_CODEGEN_FAILURE;
					}

#if DEBUG_CODEGEN == 1
					printf("Debug Codegen: Encoded directive `%s`\n", directive_name);
#endif

					curr_section->program_counter += encoding->size;
					added_entity = section_add_encoding_entity(curr_section, encoding);
					if(!added_entity) {
						// Error message should already be set.
						return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
					}
			}
		} else if(curr->type == STATEMENT_TYPE_INSTRUCTION) {
			/** A string representing the opcode type being encoded. */
			const char* opcode_name = get_opcode_string(curr->instruction.opcode);
			if(!opcode_name) {
				fprintf(stderr, "Error: Unable to get opcode name for `%i`\n",
					curr->instruction.opcode);
				return CODEGEN_ERROR_BAD_OPCODE;
			}

			status = encode_instruction(&encoding, symbol_table, &curr->instruction,
				curr_section->program_counter);
			if(!get_status(status)) {
				if(status == CODEGEN_ERROR_OPERAND_COUNT_MISMATCH) {
					fprintf(stderr, "Error: Operand count mismatch for instruction `%s`\n",
						opcode_name);

					return status;
				}

				// Error message should already be set in the encode function.
				return ASSEMBLER_ERROR_CODEGEN_FAILURE;
			}

#if DEBUG_CODEGEN == 1
			/** String representation of the encoded instruction. */
			char* string_representation = get_encoding_as_string(encoding);
			printf("Debug Codegen: Encoded instruction `%s` at `0x%zx` as `%s`\n",
				opcode_name, curr_section->program_counter, string_representation);

			free(string_representation);
#endif

			curr_section->program_counter += encoding->size;
			added_entity = section_add_encoding_entity(curr_section, encoding);
			if(!added_entity) {
				// Error message should already be set.
				return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
			}
		}

		curr = curr->next;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Populating relocation entries\n");
#endif

	populate_relocation_entries(symbol_table, sections);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Finished second pass\n");
#endif

	return ASSEMBLER_STATUS_SUCCESS;
}


/**
 * assemble
 */
Assembler_Status assemble(const char* input_filename,
	const char* output_filename,
	const bool verbose)
{
#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Beginning main assembler process.\n");
	printf("  Using input file `%s`.\n", input_filename);
	printf("  Using output file `%s`.\n", output_filename);
	if(verbose) {
		printf("  Verbose output enabled.\n");
	}
#endif

	/**
	 * @brief The main process result status.
	 * This variable is used to track the success of all main assembly
	 * procedures.
	 */
	Assembler_Status process_status = ASSEMBLER_STATUS_SUCCESS;
	/** The input file. */
	FILE* input_file = NULL;
	/** The ELF file header. */
	Elf32_Ehdr* elf_header = NULL;
	/** The binary section data. */
	Section* sections = NULL;
	/** The individual statements parsed from the source input file. */
	Statement* program_statements = NULL;
	/** The executable symbol table. */
	Symbol_Table symbol_table;


	input_file = fopen(input_filename, "r");
	if(!input_file) {
		fprintf(stderr, "Error opening input file `%s`: `%i`.\n",
			input_filename, errno);

		// Return here, no cleanup necessary.
		return ASSEMBLER_ERROR_FILE_FAILURE;
	}

	// Read in all the statements from the source file.
	process_status = read_input(input_file, &program_statements);
	if(!get_status(process_status)) {
		goto FAIL_FREE_STATEMENTS;
	}

	const int close_status = fclose(input_file);
	if(close_status) {
		fprintf(stderr, "Error closing file handler: `%u`.\n", errno);
		process_status = ASSEMBLER_ERROR_FILE_FAILURE;

		goto FAIL_FREE_STATEMENTS;
	}

	// Initialise with room for the null symbol entry.
	symbol_table.n_entries = 1;
	symbol_table.symbols = malloc(sizeof(Symbol));
	if(!symbol_table.symbols) {
		fprintf(stderr, "Error allocating symbol table.\n");
		process_status = ASSEMBLER_ERROR_BAD_ALLOC;

		goto FAIL_FREE_STATEMENTS;
	}

	// Create the null symbol entry.
	// This is required as per ELF specification.
	symbol_table.symbols[0].section = NULL;
	symbol_table.symbols[0].offset = 0;

	// Create an empty name entry, so as to not disrupt other processes that
	// require handling of this string.
	symbol_table.symbols[0].name = malloc(1);
	if(!symbol_table.symbols[0].name) {
		fprintf(stderr, "Error: Error allocating null symbol entry\n");
		process_status = ASSEMBLER_ERROR_BAD_ALLOC;

		goto FAIL_FREE_SYMBOL_TABLE;
	}

	symbol_table.symbols[0].name[0] = '\0';

	// Initialise the section list.
	process_status = initialise_sections(&sections);
	if(!get_status(process_status)) {
		// Error message set in callee.
		goto FAIL_FREE_SYMBOL_TABLE;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Beginning macro expansion\n");
#endif

	// Loop through all statements, expanding all macros.
	process_status = expand_macros(program_statements);
	if(!get_status(process_status)) {
		// Error message set in callee.
		goto FAIL_FREE_SYMBOL_TABLE;
	}

	// Begin the first assembler pass. Populating the symbol table.
	process_status = assemble_first_pass(sections,
		&symbol_table, program_statements);
	if(!get_status(process_status)) {
		// Error message set in callee.
		goto FAIL_FREE_SECTIONS;
	}

	// Begin the second assembler pass, which handles code generation.
	process_status = assemble_second_pass(sections,
		&symbol_table, program_statements);
	if(!get_status(process_status)) {
		// Error message set in callee.
		goto FAIL_FREE_SECTIONS;
	}

#if DEBUG_OUTPUT == 1
	printf("Debug Output: Initialising output file\n");
#endif

	process_status = create_elf_header(&elf_header);
	if(!get_status(process_status)) {
		// Error message set in callee.
		goto FAIL_FREE_SYMBOL_TABLE;
	}

	// Find the index into the section header block of the section header
	// string table. This is needed by the ELF header.
	const ssize_t section_shstrtab_index = find_section_index(sections, ".shstrtab");
	if(section_shstrtab_index == -1) {
		fprintf(stderr, "Error: Error finding `.shstrtab` index\n");
		process_status = ASSEMBLER_ERROR_MISSING_SECTION;

		goto FAIL_FREE_SYMBOL_TABLE;
	}

	elf_header->e_shstrndx = section_shstrtab_index;


#if DEBUG_OUTPUT == 1
	printf("Debug Output: Populating `.shstrtab`\n");
#endif

	Section* shstrtab = find_section(sections, ".shstrtab");
	if(!shstrtab) {
		fprintf(stderr, "Error finding `.shstrtab` index.");
		process_status = ASSEMBLER_ERROR_MISSING_SECTION;

		goto FAIL_FREE_SYMBOL_TABLE;
	}

	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity* added_entity = NULL;

	Section* curr_section = sections;
	while(curr_section) {
		// Iterate through each section and add its name to the section header
		// string table. Increment the total sections in the header as we go to
		// populate the total section header count.
		elf_header->e_shnum++;

		// Rrecord the index into the section header string table of each section name.
		// The current section size is the offset of each section name into SHSTRTAB.
		curr_section->name_strtab_offset = shstrtab->size;

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Adding section name: `%s` to .shstrtab at offset `0x%lx`...\n",
			curr_section->name, curr_section->name_strtab_offset);
#endif

		size_t section_name_len = strlen(curr_section->name) + 1;

		// Create an encoding entity for each section name.
		// This raw data will be added to the section header string table binary
		// data and encoded into the final encoded file.
		Encoding_Entity* string_entity = malloc(sizeof(Encoding_Entity));
		if(!string_entity) {
			fprintf(stderr, "Error allocating section name entity.");
			process_status = ASSEMBLER_ERROR_BAD_ALLOC;

			goto FAIL_FREE_SYMBOL_TABLE;
		}

		string_entity->n_reloc_entries = 0;
		string_entity->reloc_entries = NULL;

		string_entity->size = section_name_len;
		string_entity->data = malloc(section_name_len);
		if(!string_entity->data) {
			fprintf(stderr, "Error allocating section name entity data.");
			process_status = ASSEMBLER_ERROR_BAD_ALLOC;

			goto FAIL_FREE_SYMBOL_TABLE;
		}

		string_entity->data = memcpy(string_entity->data, curr_section->name,
			section_name_len);
		string_entity->data[section_name_len-1] = '\0';

		string_entity->next = NULL;

		// Add the encoded string to the `shstrtab` section.
		added_entity = section_add_encoding_entity(shstrtab, string_entity);
		if(!added_entity) {
			fprintf(stderr, "Error adding entity to section header string table.");
			process_status = ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;

			goto FAIL_FREE_SYMBOL_TABLE;
		}

		curr_section = curr_section->next;
	}


#if DEBUG_OUTPUT == 1
	printf("Debug Output: Populating .symtab...\n");
#endif

	populate_symtab(sections, &symbol_table);

	/** The total size of all section data. */
	size_t total_section_data_size = 0;

	curr_section = sections;
	while(curr_section) {
		// Calculate the total section data size.
		// The section headers will be placed after all of the binary section data in
		// the file, so we need to compute the total binary data size to predict the
		// offset of the section headers in the file.
		total_section_data_size += curr_section->size;
		curr_section = curr_section->next;
	}

	// Set the section header offset in the ELF file header.
	elf_header->e_shoff = elf_header->e_ehsize + total_section_data_size;

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Opening output file `%s`...\n", output_filename);
#endif

	// Open the output file.
	FILE* out_file = fopen(output_filename, "w");
	if(!out_file) {
		fprintf(stderr, "Error opening output file: `%u`\n", errno);
		process_status = ASSEMBLER_ERROR_FILE_FAILURE;

		goto FAIL_FREE_SECTIONS;

	}

	// Write the ELF file header.
	size_t entity_write_count = fwrite(elf_header, sizeof(Elf32_Ehdr), 1, out_file);
	if(entity_write_count != 1) {
		if(ferror(out_file)) {
			fprintf(stderr, "Error writing ELF header: `%u`.\n", errno);
		} else {
			fprintf(stderr, "Error writing ELF header.\n");
		}

		process_status = ASSEMBLER_ERROR_FILE_FAILURE;

		goto FAIL_CLOSE_OUTPUT_FILE;
	}

	// Write the binary section data to the file.
	curr_section = sections;
	while(curr_section) {
		// Update the section information to store the current file location as the
		// offset of this section's binary data in the file.
		curr_section->file_offset = ftell(out_file);

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Writing section: `%s` with size: `0x%lx` at `0x%lx`...\n",
			curr_section->name, curr_section->size, curr_section->file_offset);
#endif

		Encoding_Entity* curr_entity = curr_section->encoding_entities;
		while(curr_entity) {
			// Write each encoding entity contained in each section.
			entity_write_count = fwrite(curr_entity->data, curr_entity->size, 1, out_file);
			if(entity_write_count != 1) {
				if(ferror(out_file)) {
					fprintf(stderr, "Error writing section data: `%u`.\n", errno);
				} else {
					fprintf(stderr, "Error writing section data.\n");
				}

				process_status = ASSEMBLER_ERROR_FILE_FAILURE;

				goto FAIL_CLOSE_OUTPUT_FILE;
			}

			curr_entity = curr_entity->next;
		}

		curr_section = curr_section->next;
	}

	curr_section = sections;
	while(curr_section) {
#if DEBUG_OUTPUT == 1
		printf("Debug Output: Writing section header `%s` with offset `0x%lx` at `0x%lx`...\n",
			curr_section->name, curr_section->file_offset, ftell(out_file));
#endif

		/** The header for the current section. */
		Elf32_Shdr* section_header = NULL;

		// Encode the section header in the ELF format.
		process_status = encode_section_header(curr_section, &section_header);
		if(!get_status(process_status)) {
			goto FAIL_CLOSE_OUTPUT_FILE;
		}

		// Write each ELF header to the output file.
		entity_write_count = fwrite(section_header, sizeof(Elf32_Shdr), 1, out_file);
		if(entity_write_count != 1) {
			if(ferror(out_file)) {
				fprintf(stderr, "Error writing section header data: `%u`.\n", errno);
			} else {
				fprintf(stderr, "Error writing section header data.\n");
			}

			process_status = ASSEMBLER_ERROR_FILE_FAILURE;

			goto FAIL_CLOSE_OUTPUT_FILE;

		}

		free(section_header);

		curr_section = curr_section->next;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Cleaning up main program.\n");
#endif

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing statements...\n");
#endif

	free_statement(program_statements);

	free(elf_header);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Closing output file...\n");
#endif

	fclose(out_file);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing Symbol Table...\n");
#endif

	free_symbol_table(&symbol_table);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing sections...\n");
#endif

	free_section(sections);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Finished.\n");
#endif

	return ASSEMBLER_STATUS_SUCCESS;

FAIL_CLOSE_OUTPUT_FILE:
	free(elf_header);
	fclose(out_file);
FAIL_FREE_SECTIONS:
	free_section(sections);
FAIL_FREE_SYMBOL_TABLE:
	free_symbol_table(&symbol_table);
FAIL_FREE_STATEMENTS:
	free_statement(program_statements);

	return process_status;
}


/**
 * populate_relocation_entries
 */
static Assembler_Status populate_relocation_entries(Symbol_Table* symtab,
	Section* sections)
{
	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity* added_entity = NULL;
	/** Pointer to the current section being parsed. */
	Section *curr_section = sections;

	while(curr_section) {
		Encoding_Entity* curr_entity = curr_section->encoding_entities;
		while(curr_entity) {
			if(curr_entity->n_reloc_entries > 0) {
				// If the current entity has relocation entries.
				// First we find the relocation section relevant to the section
				// that contains this entity.
				// Search for the section by concatenating `.rel` with the section name.
				size_t curr_section_name_len = strlen(curr_section->name);
				char* curr_section_rel_name = malloc(5 + curr_section_name_len);
				if(!curr_section_rel_name) {
					fprintf(stderr, "Unable to allocate space for reloc section name.\n");
					return ASSEMBLER_ERROR_BAD_ALLOC;
				}

				strncpy(curr_section_rel_name, ".rel", 4);
				strncpy(curr_section_rel_name + 4, curr_section->name, curr_section_name_len);
				curr_section_rel_name[curr_section_name_len + 4] = '\0';

				/** The section to add the reloc entry to. */
				Section* curr_section_rel = find_section(sections, curr_section_rel_name);
				if(!curr_section_rel) {
					fprintf(stderr, "Unable to find relocatable entry section: `%s`.\n",
						curr_section_rel_name);
					return ASSEMBLER_ERROR_MISSING_SECTION;
				}

				// Free the created string we used for searching.
				free(curr_section_rel_name);

				for(size_t r=0; r<curr_entity->n_reloc_entries; r++) {
					// Create the ELF relocatione entry to encode in the file.
					Elf32_Rel* rel = malloc(sizeof(Elf32_Rel));
					if(!rel) {
						fprintf(stderr, "Unable to allocate space for reloc entry.\n");
						return ASSEMBLER_ERROR_BAD_ALLOC;
					}

					/** The index of the relevant symbol into the symbol table. */
					ssize_t symbol_index = symtab_find_symbol_index(symtab,
						curr_entity->reloc_entries[r].symbol_name);
					if(symbol_index == -1) {
						// cleanup.
						free(rel);

						fprintf(stderr, "Unable to find symbol index for: `%s`.\n",
							curr_entity->reloc_entries[r].symbol_name);
						return ASSEMBLER_ERROR_MISSING_SYMBOL;
					}

					// The `info` field is encoded as the symbol index shifted right 8
					// bits, OR'd with the symbol `type`.
					rel->r_info = (symbol_index << 8) | curr_entity->reloc_entries[r].type;
					rel->r_offset = curr_entity->reloc_entries[r].offset;

					/** The encoding entity that encodes the relocation entry. */
					Encoding_Entity* reloc_entity = malloc(sizeof(Encoding_Entity));
					if(!reloc_entity) {
						// cleanup.
						free(rel);

						fprintf(stderr, "Unable to allocate space for reloc entry encoding entity.\n");
						return ASSEMBLER_ERROR_BAD_ALLOC;
					}

					reloc_entity->n_reloc_entries = 0;
					reloc_entity->reloc_entries = NULL;

					reloc_entity->size = sizeof(Elf32_Rel);
					reloc_entity->data = (uint8_t*)rel;
					reloc_entity->next = NULL;

					// Add the relocatable entry to the relevant section.
					added_entity = section_add_encoding_entity(curr_section_rel, reloc_entity);
					if(!added_entity) {
						// Error message should already have been set.
						return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
					}
				}
			}

			curr_entity = curr_entity->next;
		}

		curr_section = curr_section->next;
	}

	return ASSEMBLER_STATUS_SUCCESS;
}
