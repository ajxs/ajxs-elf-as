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

#include <as.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Assembler_Process_Result populate_symtab(Section *sections,
	Symbol_Table *symbol_table);

Assembler_Process_Result populate_relocation_entries(Symbol_Table *symtab,
	Section *sections);

Assembler_Process_Result initialise_sections(Section **sections);

Assembler_Process_Result read_input(FILE *input_file, Statement **program_statements);


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
Assembler_Process_Result initialise_sections(Section **sections) {
	// The section header data will be filled as the sections are serialised.
	Section *section_null = create_section("\0", SHT_NULL, 0);
	if(!section_null) {
		set_error_message("Error creating `NULL` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	Section *section_text = create_section(".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
	if(!section_text) {
		set_error_message("Error creating `.text` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	// The ELF man page suggests that the flags for relocatable sections are
	// set to SHF_ALLOC, but from readelf we can see that gcc itself
	// seems to use `SHF_INFO_LINK`.
	// Refer to: 'http://www.sco.com/developers/gabi/2003-12-17/ch4.sheader.html'
	// for the undocumented flags.
	Section *section_text_rel = create_section(".rel.text", SHT_REL, SHF_INFO_LINK);
	if(!section_text_rel) {
		set_error_message("Error creating `.rel.text` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	Section *section_data = create_section(".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
	if(!section_data) {
		set_error_message("Error creating `.data` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	Section *section_data_rel = create_section(".rel.data", SHT_REL, SHF_INFO_LINK);
	if(!section_data_rel) {
		set_error_message("Error creating `.rel.data` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	Section *section_bss = create_section(".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE);
	if(!section_bss) {
		set_error_message("Error creating `.bss` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	Section *section_symtab = create_section(".symtab", SHT_SYMTAB, SHF_ALLOC);
	if(!section_symtab) {
		set_error_message("Error creating `.symtab` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	Section *section_shstrtab = create_section(".shstrtab", SHT_STRTAB, SHF_ALLOC);
	if(!section_shstrtab) {
		set_error_message("Error creating `.shstrtab` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	Section *section_strtab = create_section(".strtab", SHT_STRTAB, 0);
	if(!section_strtab) {
		set_error_message("Error creating `.strtab` section.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}


	/** Value to track the results of adding the newly created sections. */
	Section *added_section = NULL;

	added_section = add_section(sections, section_null);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_text);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_text_rel);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_data);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_data_rel);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_bss);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_symtab);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_shstrtab);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	added_section = add_section(sections, section_strtab);
	if(!added_section) {
		// Error message set in callee.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

	// Find the index of the string table section, so we can link the symbol
	// table section to the string table section.
	ssize_t section_strtab_index = find_section_index(*sections, ".strtab");
	if(section_strtab_index == -1) {
		set_error_message("Unable to find `.strtab` section index.");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_symtab->link = section_strtab_index;

	// Find the index of the data section, so we can link its relevant relocation
	// entry section to it.
	ssize_t section_data_index = find_section_index(*sections, ".data");
	if(section_data_index == -1) {
		set_error_message("Unable to find `.data` section index.");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_data_rel->info = section_data_index;


	// Find the index of the text section, so we can link its relevant relocation
	// entry section to it.
	ssize_t section_text_index = find_section_index(*sections, ".text");
	if(section_text_index == -1) {
		set_error_message("Unable to find `.text` section index.");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_text_rel->info = section_text_index;


	// Find the index of the symbol table section, so we can link the program data
	// sections to it.
	ssize_t section_symtab_index = find_section_index(*sections, ".symtab");
	if(section_symtab_index == -1) {
		set_error_message("Unable to find `.symtab` section index.");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	section_data_rel->link = section_symtab_index;
	section_text_rel->link = section_symtab_index;

	return ASSEMBLER_PROCESS_SUCCESS;
}


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
Assembler_Process_Result assemble_first_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements) {

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Begin first pass...\n");
#endif

	Section *section_text = find_section(sections, ".text");
	if(!section_text) {
		set_error_message("Unable to locate .text section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	Section *section_data = find_section(sections, ".data");
	if(!section_text) {
		set_error_message("Unable to locate .data section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	Section *section_bss = find_section(sections, ".bss");
	if(!section_text) {
		set_error_message("Unable to locate .bss section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	// Start in the .text section by default.
	Section *section_current = section_text;

	Statement *curr = statements;

	while(curr) {
		// All labels must be processed first.
		// Since a label _can_ precede a section directive, but not the other way around.
		if(curr->labels) {
			for(size_t i = 0; i < curr->n_labels; i++) {
				Symbol *added_sybmol = symtab_add_symbol(symbol_table, curr->labels[i], section_current,
					section_current->program_counter);
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
				section_current = section_bss;
			} else if(curr->directive.type == DIRECTIVE_DATA) {
				section_current = section_data;
			} else if(curr->directive.type == DIRECTIVE_TEXT) {
				section_current = section_text;
			}
		}

		/** The encoded size of the statement. */
		ssize_t statement_size = get_statement_size(curr);
		if(statement_size == -1) {
			// Error should already have been set.
			return ASSEMBLER_ERROR_STATEMENT_SIZE;
		}

#if DEBUG_ASSEMBLER == 1
		printf("Debug Assembler: Calculated size `0x%lx` for statement.\n", statement_size);
#endif

		// Increment the current section's program counter by the size of the
		// statement that has been computed.
		section_current->program_counter += (size_t)statement_size;
		curr = curr->next;
	}


#if DEBUG_SYMBOLS == 1
	// Print the symbol table.
	printf("Debug Assembler: Symbol Table:\n");
	print_symbol_table(symbol_table);
#endif

	return ASSEMBLER_PROCESS_SUCCESS;
}


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
Assembler_Process_Result populate_relocation_entries(Symbol_Table *symtab,
	Section *sections) {

	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity *added_entity = NULL;

	Section *curr_section = sections;
	while(curr_section) {
		Encoding_Entity *curr_entity = curr_section->encoding_entities;
		while(curr_entity) {
			if(curr_entity->n_reloc_entries > 0) {
				// If the current entity has relocation entries.
				// First we find the relocation section relevant to the section
				// that contains this entity.
				// Search for the section by concatenating `.rel` with the section name.
				size_t curr_section_name_len = strlen(curr_section->name);
				char *curr_section_rel_name = malloc(5 + curr_section_name_len);
				if(!curr_section_rel_name) {
					set_error_message("Unable to allocate space for reloc section name.\n");
					return ASSEMBLER_ERROR_BAD_ALLOC;
				}

				strcpy(curr_section_rel_name, ".rel");
				strcpy(curr_section_rel_name + 4, curr_section->name);
				curr_section_rel_name[curr_section_name_len + 4] = '\0';

				/** The section to add the reloc entry to. */
				Section *curr_section_rel = find_section(sections, curr_section_rel_name);
				if(!curr_section_rel) {
					char error_message[ERROR_MSG_MAX_LEN];
					sprintf(error_message, "Unable to find relocatable entry section: `%s`.",
						curr_section_rel_name);
					return ASSEMBLER_ERROR_MISSING_SECTION;
				}

				// Free the created string we used for searching.
				free(curr_section_rel_name);

				for(size_t r=0; r<curr_entity->n_reloc_entries; r++) {
					// Create the ELF relocatione entry to encode in the file.
					Elf32_Rel *rel = malloc(sizeof(Elf32_Rel));
					if(!rel) {
						set_error_message("Unable to allocate space for reloc entry.\n");
						return ASSEMBLER_ERROR_BAD_ALLOC;
					}

					/** The index of the relevant symbol into the symbol table. */
					ssize_t symbol_index = symtab_find_symbol_index(symtab,
						curr_entity->reloc_entries[r].symbol->name);
					if(symbol_index == -1) {
						char error_message[ERROR_MSG_MAX_LEN];
						sprintf(error_message, "Unable to find symbol index for: `%s`.",
							curr_entity->reloc_entries[r].symbol->name);
						return ASSEMBLER_ERROR_MISSING_SYMBOL;
					}

					// The `info` field is encoded as the symbol index shifted right 8
					// bits, OR'd with the symbol `type`.
					rel->r_info = (symbol_index << 8) | curr_entity->reloc_entries[r].type;
					rel->r_offset = curr_entity->reloc_entries[r].offset;

					/** The encoding entity that encodes the relocation entry. */
					Encoding_Entity *reloc_entity = malloc(sizeof(Encoding_Entity));
					if(!reloc_entity) {
						set_error_message("Unable to allocate space for reloc entry encoding entity.\n");
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

	return ASSEMBLER_PROCESS_SUCCESS;
}


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
Assembler_Process_Result assemble_second_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements) {

	if(!sections) {
		set_error_message("Invalid section data.\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	if(!symbol_table) {
		set_error_message("Invalid symbol table data.\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	if(!statements) {
		set_error_message("Invalid statement data.\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Begin second pass...\n");
#endif

	// Ensure all section program counters counters are reset.
	// These will have been set by the first assembly pass.
	Section *curr_section = sections;
	while(curr_section) {
		curr_section->program_counter = 0;
		curr_section = curr_section->next;
	}

	Section *section_text = find_section(sections, ".text");
	if(!section_text) {
		set_error_message("Unable to locate .text section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	Section *section_data = find_section(sections, ".data");
	if(!section_data) {
		set_error_message("Unable to locate .data section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	Section *section_bss = find_section(sections, ".bss");
	if(!section_bss) {
		set_error_message("Unable to locate .bss section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	// Start in the .text section by default.
	Section *section_current = section_text;

	Statement *curr = statements;

	Encoding_Entity *encoding = NULL;
	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity *added_entity = NULL;

	while(curr) {
		if(curr->type == STATEMENT_TYPE_DIRECTIVE) {
			switch(curr->directive.type) {
				case DIRECTIVE_BSS:
#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Setting current section to `.bss`...\n");
#endif
					section_current = section_bss;
					break;
				case DIRECTIVE_DATA:
#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Setting current section to `.data`...\n");
#endif
					section_current = section_data;
					break;
				case DIRECTIVE_TEXT:
#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Setting current section to `.text`...\n");
#endif
					section_current = section_text;
					break;
				case DIRECTIVE_ALIGN:
				case DIRECTIVE_SIZE:
				case DIRECTIVE_GLOBAL:
					// These entities are not directly encoded.
					// They represent instructions to the assembler which do not result
					// in encoded binary entities.
					break;
				default:
					encoding = encode_directive(symbol_table, &curr->directive,
						section_current->program_counter);
					if(!encoding) {
						// Error message should already be set in the encode function.
						return ASSEMBLER_ERROR_CODEGEN_FAILURE;
					}

					section_current->program_counter += encoding->size;
					added_entity = section_add_encoding_entity(section_current, encoding);
					if(!added_entity) {
						// Error message should already be set.
						return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
					}

			}
		} else if(curr->type == STATEMENT_TYPE_INSTRUCTION) {
			encoding = encode_instruction(symbol_table, &curr->instruction,
				section_current->program_counter);
			if(!encoding) {
				// Error message should already be set in the encode function.
				return ASSEMBLER_ERROR_CODEGEN_FAILURE;
			}

			section_current->program_counter += encoding->size;
			added_entity = section_add_encoding_entity(section_current, encoding);
			if(!added_entity) {
				// Error message should already be set.
				return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
			}
		}

		curr = curr->next;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Populating relocation entries...\n");
#endif

	populate_relocation_entries(symbol_table, sections);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Finished second pass.\n");
#endif

	return ASSEMBLER_PROCESS_SUCCESS;
}


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
Assembler_Process_Result populate_symtab(Section *sections,
	Symbol_Table *symbol_table) {

	if(!sections) {
		set_error_message("Invalid section data.\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	if(!symbol_table) {
		set_error_message("Invalid symbol table data.\n");
		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}


	Section *strtab = find_section(sections, ".strtab");
	if(!strtab) {
		set_error_message("Unable to locate .strtab section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	Section *symtab = find_section(sections, ".symtab");
	if(!symtab) {
		set_error_message("Unable to locate .symtab section.\n");
		return ASSEMBLER_ERROR_MISSING_SECTION;
	}

	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity *added_entity = NULL;

	// Add the initial null byte to strtab as per ELF specification.
	Encoding_Entity *null_byte_entity = malloc(sizeof(Encoding_Entity));
	if(!null_byte_entity) {
		set_error_message("Error allocating null byte symbol entity.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	null_byte_entity->n_reloc_entries = 0;
	null_byte_entity->reloc_entries = NULL;

	null_byte_entity->size = 1;
	null_byte_entity->data = malloc(1);
	if(!null_byte_entity->data) {
		set_error_message("Error allocating null byte symbol entity data.");
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	null_byte_entity->data[0] = '\0';

	null_byte_entity->next = NULL;

	added_entity = section_add_encoding_entity(strtab, null_byte_entity);
	if(!added_entity) {
		// Error message should already be set.
		return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
	}

#if DEBUG_OUTPUT == 1
	printf("Debug Output: Added null byte to .strtab.\n");
#endif

	for(size_t i = 0; i < symbol_table->n_entries; i++) {
		// Add each symbol name to the string table, and each symbol entry to the
		// symbol table section.

		Elf32_Sym symbol_entry;
		symbol_entry.st_name = strtab->size;
		symbol_entry.st_value = symbol_table->symbols[i].offset;
		symbol_entry.st_size = 0;
		symbol_entry.st_info = 0;
		symbol_entry.st_other = 0;

		ssize_t shndx = 0;
		if(symbol_table->symbols[i].section) {
			// Take into account that we need to successfully parse the null symbol
			// entry. The null entry has zero for the section header index.
			shndx = find_section_index(sections,
				symbol_table->symbols[i].section->name);
		}

		// If we could not match the section index, abort.
		if(shndx == -1) {
			char error_message[ERROR_MSG_MAX_LEN];
			sprintf(error_message, "Unable to find section index for: `%s`.",
				symbol_table->symbols[i].section->name);
			return ASSEMBLER_ERROR_MISSING_SECTION;
		}

		// Get the section index.
		symbol_entry.st_shndx = shndx;

#if DEBUG_OUTPUT == 1
			printf("Debug Output: Matched section index: `%i` for symbol name `%s`.\n",
				symbol_entry.st_shndx, symbol_table->symbols[i].name);
#endif

		size_t symbol_entry_size = sizeof(Elf32_Sym);

		// Create an encoding entity for each symbol entry, this will be encoded
		// in the symbol table section during the writing of the section data.
		Encoding_Entity *symbol_entry_entity = malloc(sizeof(Encoding_Entity));
		if(!symbol_entry_entity) {
			set_error_message("Error allocating symbol entity.");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_entry_entity->n_reloc_entries = 0;
		symbol_entry_entity->reloc_entries = NULL;

		symbol_entry_entity->size = symbol_entry_size;
		symbol_entry_entity->data = malloc(symbol_entry_size);
		if(!symbol_entry_entity->data) {
			set_error_message("Error allocating symbol entity data.");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_entry_entity->data = memcpy(symbol_entry_entity->data,
			&symbol_entry, symbol_entry_size);

		symbol_entry_entity->next = NULL;

		added_entity = section_add_encoding_entity(symtab, symbol_entry_entity);
		if(!added_entity) {
			// Error message should already be set.
			return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
		}

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Adding symbol: `%s` to .symtab at offset `0x%lx`...\n",
			symbol_table->symbols[i].name, symtab->size);
#endif

		// Create an encoding entity for each symbol name, this will be encoded
		// in the string table during the writing of the section data.
		Encoding_Entity *symbol_name_entity = malloc(sizeof(Encoding_Entity));
		if(!symbol_name_entity) {
			set_error_message("Error allocating symbol name entity.");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_name_entity->n_reloc_entries = 0;
		symbol_name_entity->reloc_entries = NULL;

		size_t symbol_name_len = strlen(symbol_table->symbols[i].name) + 1;
		symbol_name_entity->size = symbol_name_len;
		symbol_name_entity->data = malloc(symbol_name_len);
		if(!symbol_name_entity->data) {
			set_error_message("Error allocating symbol name entity data.");
			return ASSEMBLER_ERROR_BAD_ALLOC;
		}

		symbol_name_entity->data = memcpy(symbol_name_entity->data,
			symbol_table->symbols[i].name, symbol_name_len);
		symbol_name_entity->data[symbol_name_len-1] = '\0';

		symbol_name_entity->next = NULL;

		added_entity = section_add_encoding_entity(strtab, symbol_name_entity);
		if(!added_entity) {
			// Error message should already be set.
			return ASSEMBLER_ERROR_SECTION_ENTITY_FAILURE;
		}

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Added symbol name: `%s` to .strtab at offset `0x%lx`.\n",
			symbol_table->symbols[i].name, strtab->size);
#endif
	}

	return ASSEMBLER_PROCESS_SUCCESS;
}


/**
 * @brief Reads the source file input.
 *
 * This function reads the assembly source file, lexes and parses each individual
 * statement. After all of the individual statements in the file have been parsed
 * these are passed to the two stage assembler.
 * The file handle is closed in the main function.
 * @param input_file The file pointer for the input source file.
 * @param program_statements A pointer-to-pointer to the statement list.
 * @return A status entity indicating whether or not the pass was successful.
 */
Assembler_Process_Result read_input(FILE *input_file,
	Statement **program_statements) {

	char *line_buffer = NULL;
	size_t line_buffer_length = 0;
	ssize_t chars_read = 0;
	size_t line_num = 1;

	// Read all the lines in the file.
	while((chars_read = getline(&line_buffer, &line_buffer_length, input_file)) != -1) {
#if DEBUG_INPUT == 1
	printf("Input line #%u: `%s`", line_num, line_buffer);
#endif

		// Preprocess the line. Normalises the line to conform to a standard format.
		// If NULL is returned, assume error state and abort.
		char *line = preprocess_line(line_buffer);
		if(!line) {
			// Error message should have been set in callee.
			// Free the line buffer.
			free(line_buffer);
			return ASSEMBLER_ERROR_PREPROCESSING_FAILURE;
		}

		// If the resulting line has no length, do not parse any further.
		if(strlen(line) == 0) {
			free(line);
			continue;
		}

		// This is where each line from the source file is lexed and parsed.
		// This returns a linked-list entity, since architecture-depending, a single
		// line may contain multiple `statement`s.
		Statement *parsed_statements = scan_string(line);

		// Iterate through each processed statement and set its line number.
		Statement *curr = parsed_statements;
		curr->line_num = line_num;
		while(curr->next) {
			curr = curr->next;
			curr->line_num = line_num;
		}

		if(!*program_statements) {
			// Add to start of linked list.
			*program_statements = parsed_statements;
		} else {
			// Add to tail of linked list.
			curr = *program_statements;
			while(curr->next) {
				curr = curr->next;
			}

			curr->next = parsed_statements;
		}

		// Free the preprocessed line.
		free(line);
		line_num++;
	}

#if DEBUG_PARSED_STATEMENTS == 1
	// Iterate over all parsed statements, printing each one.
	Statement *curr = *program_statements;

	while(curr) {
		print_statement(curr);
		curr = curr->next;
	}
#endif

	// Prevent memory leak. Refer to:
	// https://stackoverflow.com/questions/55731141/memory-leak-when-reading-file-line-by-line-using-getline
	free(line_buffer);

	return ASSEMBLER_PROCESS_SUCCESS;
}


/**
 * @brief The main assembler entry point.
 *
 * This function begins the assembly process for an input source file.
 * All processing and assembly is initiated here.
 * @param input_filename The file path for the input source file.
 * @param output_filename The file path for the output source file.
 */
void assemble(const char *input_filename,
	const char *output_filename,
	bool verbose) {

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Beginning main assembler process.\n");
	printf("  Using input file `%s`.\n", input_filename);
	printf("  Using output file `%s`.\n", output_filename);
	if(verbose) {
		printf("  Verbose output enabled.\n");
	}
#endif

	FILE *input_file = fopen(input_filename, "r");
	if(!input_file) {
		// @ERROR
		fprintf(stderr, "Error opening file: %i\n", errno);
		return;
	}

	/** The individual statements parsed from the source input file. */
	Statement *program_statements = NULL;

	Assembler_Process_Result read_input_status = read_input(input_file,
		&program_statements);
	if(read_input_status != ASSEMBLER_PROCESS_SUCCESS) {
		// @ERROR.
	}

	int close_status = fclose(input_file);
	if(close_status) {
		// @ERROR.
		fprintf(stderr, "Error closing file handler: %u! Exiting.\n", errno);
	}

	/** The executable symbol table. */
	Symbol_Table symbol_table;

	// Initialise with room for the null symbol entry.
	symbol_table.n_entries = 1;
	symbol_table.symbols = malloc(sizeof(Symbol));
	if(!symbol_table.symbols) {
		// @ERROR
		return;
	}

	// Create the null symbol entry.
	// This is required as per ELF specification.
	symbol_table.symbols[0].section = NULL;
	symbol_table.symbols[0].offset = 0;

	// Create an empty name entry, so as to not disrupt other processes that
	// require handling of this string.
	symbol_table.symbols[0].name = malloc(1);
	if(!symbol_table.symbols[0].name) {
		// @ERROR
		return;
	}

	symbol_table.symbols[0].name[0] = '\0';

	/** The binary section data. */
	Section *sections = NULL;

	// Initialise the section list.
	Assembler_Process_Result init_section_status = initialise_sections(&sections);
	if(init_section_status != ASSEMBLER_PROCESS_SUCCESS) {
		// @ERROR.
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Beginning macro expansion...\n");
#endif

	// Loop through all statements, expanding all macros.
	expand_macros(program_statements);

	// Begin the first assembler pass. Populating the symbol table.
	Assembler_Process_Result first_pass_status = assemble_first_pass(sections,
		&symbol_table, program_statements);
	if(first_pass_status != ASSEMBLER_PROCESS_SUCCESS) {
		// @ERROR.
	}

	// Begin the second assembler pass, which handles code generation.
	Assembler_Process_Result second_pass_status = assemble_second_pass(sections,
		&symbol_table, program_statements);
	if(second_pass_status != ASSEMBLER_PROCESS_SUCCESS) {
		// @ERROR.
	}

#if DEBUG_OUTPUT == 1
	printf("Debug Assembler: Freeing parsed statements...\n");
#endif

	free_statement(program_statements);


#if DEBUG_OUTPUT == 1
	printf("Debug Output: Initialising output file...\n");
#endif

	/** The ELF file header. */
	Elf32_Ehdr *elf_header = create_elf_header();
	if(!elf_header) {
		// @ERROR
		return;
	}

	// Find the index into the section header block of the section header
	// string table. This is needed by the ELF header.
	ssize_t section_shstrtab_index = find_section_index(sections, ".shstrtab");
	if(section_shstrtab_index == -1) {
		// @ERROR.
		printf("Error finding `.shstrtab` index.\n");
	}

	elf_header->e_shstrndx = section_shstrtab_index;


#if DEBUG_OUTPUT == 1
	printf("Debug Output: Populating .shstrtab...\n");
#endif

	Section *shstrtab = find_section(sections, ".shstrtab");
	if(!shstrtab) {
		// @ERROR
		printf("Error finding `.shstrtab` section.\n");
		return;
	}

	/** Used for tracking the result of adding the entity to a section. */
	Encoding_Entity *added_entity = NULL;

	Section *curr_section = sections;
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
		Encoding_Entity *string_entity = malloc(sizeof(Encoding_Entity));
		if(!string_entity) {
			// @ERROR
			return;
		}

		string_entity->n_reloc_entries = 0;
		string_entity->reloc_entries = NULL;

		string_entity->size = section_name_len;
		string_entity->data = malloc(section_name_len);
		if(!string_entity->data) {
			// @ERROR
			return;
		}

		string_entity->data = memcpy(string_entity->data, curr_section->name,
			section_name_len);
		string_entity->data[section_name_len-1] = '\0';

		string_entity->next = NULL;

		// Add the encoded string to the `shstrtab` section.
		added_entity = section_add_encoding_entity(shstrtab, string_entity);
		if(!added_entity) {
			// @ERROR
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
	FILE *out_file = fopen(output_filename, "w");
	if(!out_file) {
		// @ERROR
		fprintf(stderr, "Error opening output file: `%u`\n", errno);
	}

	// Write the ELF file header.
	size_t entity_write_count = fwrite(elf_header, sizeof(Elf32_Ehdr), 1, out_file);
	if(entity_write_count != 1) {
		if(ferror(out_file)) {
			// @ERROR
			perror("Error writing ELF header.\n");
		}
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

		Encoding_Entity *curr_entity = curr_section->encoding_entities;
		while(curr_entity) {
			// Write each encoding entity contained in each section.
			entity_write_count = fwrite(curr_entity->data, curr_entity->size, 1, out_file);
			if(entity_write_count != 1) {
				// @ERROR
				if(ferror(out_file)) {
					perror("Error writing section data.\n");
				}
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

		// Encode the section header in the ELF format.
		Elf32_Shdr *section_header = encode_section_header(curr_section);
		if(!section_header) {
			// @ERROR
			return;
		}

		// Write each ELF header to the output file.
		entity_write_count = fwrite(section_header, sizeof(Elf32_Shdr), 1, out_file);
		if(entity_write_count != 1) {
			// @ERROR
			if(ferror(out_file)) {
				perror("Error writing section header data.\n");
			}
		}

		free(section_header);

		curr_section = curr_section->next;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Cleaning up main program.\n");
#endif

FAIL_CLOSE_FILE:

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing statements...\n");
#endif

	free(elf_header);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Closing output file...\n");
#endif
	fclose(out_file);

FAIL_FREE_SYMBOL_TABLE:

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing Symbol Table...\n");
#endif

	free_symbol_table(&symbol_table);

FAIL_FREE_SECTIONS:

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing sections...\n");
#endif

	free_section(sections);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Finished.\n");
#endif
}
