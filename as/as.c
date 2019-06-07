#include <as.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void populate_symtab(Section *sections,
	Symbol_Table *symbol_table);

void populate_relocation_entries(Section *sections);


Section *initialise_sections(void);

Statement *read_input(FILE *input_file);


/**
 * @brief Creates and initialises the executable sections.
 *
 * This function creates all of the sections required to generate a relocatable
 * ELF file. This will create all of the sections, as well as their relocation
 * entry sections.
 * Creates a linked list of the sections.
 * @return A pointer to the created section linked list.
 */
Section *initialise_sections(void) {
	Section *sections = NULL;

	// The section header data will be filled as the sections are serialised.
	Section *section_null = create_section("\0", SHT_NULL, 0);

	Section *section_text = create_section(".text", SHT_PROGBITS,
		SHF_ALLOC | SHF_EXECINSTR);

	// The ELF man page suggests that the flags for relocatable sections are
	// set to SHF_ALLOC, but from readelf we can see that gcc itself
	// seems to use `SHF_INFO_LINK`.
	// Refer to: 'http://www.sco.com/developers/gabi/2003-12-17/ch4.sheader.html'
	// for the undocumented flags.
	Section *section_text_rel = create_section(".rel.text", SHT_REL, SHF_INFO_LINK);

	Section *section_data = create_section(".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);

	Section *section_data_rel = create_section(".rel.data", SHT_REL, SHF_INFO_LINK);

	Section *section_bss = create_section(".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE);

	Section *section_symtab = create_section(".symtab", SHT_SYMTAB, SHF_ALLOC);

	Section *section_shstrtab = create_section(".shstrtab", SHT_STRTAB, SHF_ALLOC);

	Section *section_strtab = create_section(".strtab", SHT_STRTAB, 0);

	add_section(&sections, section_null);
	add_section(&sections, section_text);
	add_section(&sections, section_text_rel);
	add_section(&sections, section_data);
	add_section(&sections, section_data_rel);
	add_section(&sections, section_bss);
	add_section(&sections, section_symtab);
	add_section(&sections, section_shstrtab);
	add_section(&sections, section_strtab);

	// Populate the 'link' fields of the sections.
	ssize_t section_strtab_index = find_section_index(sections, ".strtab");
	section_strtab_index = find_section_index(sections, ".strtab");
	if(section_strtab_index == -1) {
		printf("Error linking .symtab to .strtab.");
	}

	section_symtab->link = section_strtab_index;


	ssize_t section_data_index = find_section_index(sections, ".data");
	if(section_data_index == -1) {
		printf("Error linking .rel.data to .data.");
	}

	section_data_rel->info = section_data_index;


	ssize_t section_text_index = find_section_index(sections, ".text");
	if(section_text_index == -1) {
		printf("Error linking .rel.text to .text.");
	}

	section_text_rel->info = section_text_index;


	ssize_t section_symtab_index = find_section_index(sections, ".symtab");
	section_symtab_index = find_section_index(sections, ".symtab");
	if(section_symtab_index == -1) {
		printf("Error linking .rel.data to .symtab.");
	}


	section_data_rel->link = section_symtab_index;
	section_text_rel->link = section_symtab_index;

	return sections;
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
 */
void assemble_first_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements) {

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Begin first pass...\n");
#endif

	Section *section_text = find_section(sections, ".text");
	if(!section_text) {
		printf("Error finding section: `.text`\n");
	}

	Section *section_data = find_section(sections, ".data");
	if(!section_text) {
		printf("Error finding section: `.data`\n");
	}

	Section *section_bss = find_section(sections, ".bss");
	if(!section_text) {
		printf("Error finding section: `.bss`\n");
	}

	// Start in the .text section by default.
	Section *section_current = section_text;

	Statement *curr = statements;

	while(curr) {
		// All labels must be processed first.
		// Since a label _can_ precede a section directive, but not the other way around.
		if(curr->labels) {
			for(size_t i = 0; i < curr->n_labels; i++) {
				symtab_add_symbol(symbol_table, curr->labels[i], section_current,
					section_current->program_counter);
			}
		}

		// Process section directives.
		// These are directives which specify which section to place the following
		// statements in. Adjust the current section accordingly.
		// These have a size of zero, as returned from `get_statement_size`.
		if(curr->type == STATEMENT_TYPE_DIRECTIVE) {
			if(curr->body.directive.type == DIRECTIVE_BSS) {
				section_current = section_bss;
			} else if(curr->body.directive.type == DIRECTIVE_DATA) {
				section_current = section_data;
			} else if(curr->body.directive.type == DIRECTIVE_TEXT) {
				section_current = section_text;
			}
		}

		/** The encoded size of the statement. */
		ssize_t statement_size = get_statement_size(curr);
		if(statement_size == -1) {
			printf("Error getting statement size for ");
			if(curr->type == STATEMENT_TYPE_DIRECTIVE) {
				printf("directive `");
				print_directive_type(curr->body.directive);
				printf("`.\n");
			} else if(curr->type == STATEMENT_TYPE_INSTRUCTION) {
				printf("instruction `");
				print_opcode(curr->body.instruction.opcode);
				printf("`.\n");
			} else {
				printf("empty statement.\n");
			}
		}

#if DEBUG_ASSEMBLER == 1
		printf("Debug Assembler: Calculated size `0x%lx` for ", statement_size);
		if(curr->type == STATEMENT_TYPE_DIRECTIVE) {
			printf("directive `");
			print_directive_type(curr->body.directive);
			printf("`.\n");
		} else if(curr->type == STATEMENT_TYPE_INSTRUCTION) {
			printf("instruction `");
			print_opcode(curr->body.instruction.opcode);
			printf("`.\n");
		} else {
			printf("empty statement.\n");
		}
#endif

		// Increment the program counter by the size of the statement found.
		section_current->program_counter += (size_t)statement_size;
		curr = curr->next;
	}


#if DEBUG_SYMBOLS == 1
	// Print the symbol table.
	printf("Debug Assembler: Symbol Table:\n");
	for(size_t i = 0; i < symbol_table->n_entries; i++) {
		printf("  Symbol: `%s` in `%s` at `%#zx`\n", symbol_table->symbols[i].name,
			symbol_table->symbols[i].section->name, symbol_table->symbols[i].offset);
	}
#endif
}


/**
 * @brief Populates the relocation entry sections.
 *
 * This function populates the sections specific to relocation entries. Each
 * statement for which code has been generated is parsed, and any relocation
 * entries generated are encoded in the correct ELF format and added to their
 * relevant relocation entry section.
 * @param sections A pointer to the section linked list.
 * @warning This function modifies the sections.
 */
void populate_relocation_entries(Section *sections) {
	Section *curr_section = sections;
	while(curr_section) {
		Encoding_Entity *curr_entity = curr_section->encoding_entities;
		while(curr_entity) {
			if(curr_entity->n_reloc_entries > 0) {
				size_t curr_section_name_len = strlen(curr_section->name);
				char *curr_section_rel_name = malloc(5 + curr_section_name_len);
				strcpy(curr_section_rel_name, ".rel");
				strcpy(curr_section_rel_name + 4, curr_section->name);
				curr_section_rel_name[curr_section_name_len + 4] = '\0';

				/** The section to add the reloc entry to. */
				Section *curr_section_rel = find_section(sections, curr_section_rel_name);
				if(!curr_section_rel) {
					printf("Error: Unable to find relocatable entry section: `%s`\n",
						curr_section_rel_name);
				}

				free(curr_section_rel_name);

				for(size_t r=0; r<curr_entity->n_reloc_entries; r++) {
					Elf32_Rel *rel = malloc(sizeof(Elf32_Rel));
					rel->r_info = curr_entity->reloc_entries[r].type;
					rel->r_offset = curr_section->file_offset;

					Encoding_Entity *reloc_entity = malloc(sizeof(Encoding_Entity));
					reloc_entity->n_reloc_entries = 0;
					reloc_entity->reloc_entries = NULL;

					reloc_entity->size = sizeof(Elf32_Rel);
					reloc_entity->data = (uint8_t*)rel;

					// Add the relocatable entry to the relevant section.
					section_add_encoding_entity(curr_section_rel, reloc_entity);
				}
			}

			curr_entity = curr_entity->next;
		}

		curr_section = curr_section->next;
	}
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
 */
void assemble_second_pass(Section *sections,
	Symbol_Table *symbol_table,
	Statement *statements) {

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
	Section *section_data = find_section(sections, ".data");
	Section *section_bss = find_section(sections, ".bss");
	// Start in the .text section by default.
	Section *section_current = section_text;

	Statement *curr = statements;

	Encoding_Entity *encoding = NULL;

	while(curr) {
		if(curr->type == STATEMENT_TYPE_DIRECTIVE) {
			switch(curr->body.directive.type) {
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
					// Non directly encoded entities.
					break;
				default:
					encoding = encode_directive(symbol_table, &curr->body.directive,
						section_current->program_counter);
					section_current->program_counter += encoding->size;
					section_add_encoding_entity(section_current, encoding);
			}
		} else if(curr->type == STATEMENT_TYPE_INSTRUCTION) {
			encoding = encode_instruction(symbol_table, curr->body.instruction,
				section_current->program_counter);

			if(!encoding) {
				// handle error.
			}

			section_current->program_counter += encoding->size;
			section_add_encoding_entity(section_current, encoding);
		}

		curr = curr->next;
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Populating relocation entries...\n");
#endif

	populate_relocation_entries(sections);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Finished second pass.\n");
#endif
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
 */
void populate_symtab(Section *sections,
	Symbol_Table *symbol_table) {

	Section *strtab = find_section(sections, ".strtab");
	Section *symtab = find_section(sections, ".symtab");

	// Add the initial null byte to strtab as per ELF documentation.
	Encoding_Entity *null_byte_entity = malloc(sizeof(Encoding_Entity));
	null_byte_entity->n_reloc_entries = 0;
	null_byte_entity->reloc_entries = NULL;

	null_byte_entity->size = 1;
	null_byte_entity->data = malloc(1);
	null_byte_entity->data[0] = '\0';

	section_add_encoding_entity(strtab, null_byte_entity);

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

		ssize_t shndx = find_section_index(sections,
			symbol_table->symbols[i].section->name);

		// If we could not match the section index, abort.
		if(shndx == -1) {
			printf("ERROR FINDING SYMBOL SECTION INDEX.\n");
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
		symbol_entry_entity->n_reloc_entries = 0;
		symbol_entry_entity->reloc_entries = NULL;

		symbol_entry_entity->size = symbol_entry_size;
		symbol_entry_entity->data = malloc(symbol_entry_size);
		symbol_entry_entity->data = memcpy(symbol_entry_entity->data,
			&symbol_entry, symbol_entry_size);

		section_add_encoding_entity(symtab, symbol_entry_entity);

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Adding symbol: `%s` to .symtab at offset `0x%lx`...\n",
			symbol_table->symbols[i].name, symtab->size);
#endif

		// Create an encoding entity for each symbol name, this will be encoded
		// in the string table during the writing of the section data.
		Encoding_Entity *symbol_name_entity = malloc(sizeof(Encoding_Entity));
		symbol_name_entity->n_reloc_entries = 0;
		symbol_name_entity->reloc_entries = NULL;

		size_t symbol_name_len = strlen(symbol_table->symbols[i].name) + 1;
		symbol_name_entity->size = symbol_name_len;
		symbol_name_entity->data = malloc(symbol_name_len);
		symbol_name_entity->data = memcpy(symbol_name_entity->data,
			symbol_table->symbols[i].name, symbol_name_len);
		symbol_name_entity->data[symbol_name_len] = '\0';

		section_add_encoding_entity(strtab, symbol_name_entity);

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Added symbol name: `%s` to .strtab at offset `0x%lx`.\n",
			symbol_table->symbols[i].name, strtab->size);
#endif
	}
}


/**
 * @brief Reads the source file input.
 *
 * This function reads the assembly source file, lexes and parses each individual
 * statement. After all of the individual statements in the file have been parsed
 * these are passed to the two stage assembler.
 * The file handle is closed in the main function.
 * @param input_file The file pointer for the input source file.
 */
Statement *read_input(FILE *input_file) {
	char *line_buffer = 0;
	size_t line_buffer_length = 0;
	ssize_t chars_read = 0;
	size_t line_num = 1;

	Statement *program_statements = NULL;

	// Read all the lines in the file.
	while((chars_read = getline(&line_buffer, &line_buffer_length, input_file)) != -1) {
#if DEBUG_INPUT == 1
	printf("Input line #%u: `%s`", line_num, line_buffer);
#endif

		// Preprocess the line. Normalises the line to conform to a standard format.
		// If NULL is returned, assume error state and abort.
		char *line = preprocess_line(line_buffer);
		if(!line) {
			printf("Error preprocessing line.");
			break;
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
		Statement *curr = parsed_statements;
		curr->line_num = line_num;
		while(curr->next) {
			curr = curr->next;
			curr->line_num = line_num;
		}

		if(!program_statements) {
			// Add to start of linked list.
			program_statements = parsed_statements;
		} else {
			// Add to tail of linked list.
			Statement *curr = program_statements;
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
	Statement *curr = program_statements;

	while(curr) {
		print_statement(curr);
		curr = curr->next;
	}
#endif

	return program_statements;
}


/**
 * @brief The main assembler entry point.
 *
 * This function begins the assembly process for an input source file.
 * All processing and assembly is initiated here.
 * @param input_file The file pointer for the input source file.
 */
void assemble(FILE *input_file) {
	/** The individual statements parsed from the source input file. */
	Statement *program_statements = read_input(input_file);

	/** The executable symbol table. */
	Symbol_Table symbol_table;
	symbol_table.n_entries = 0;
	symbol_table.symbols = NULL;

	/** The binary section data. */
	Section *sections = initialise_sections();

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Beginning macro expansion...\n");
#endif

	// Loop through all statements, expanding all macros.
	expand_macros(program_statements);

	// Begin the first assembler pass. Populating the symbol table.
	assemble_first_pass(sections, &symbol_table, program_statements);

	// Begin the second assembler pass, which handles code generation.
	assemble_second_pass(sections, &symbol_table, program_statements);


#if DEBUG_OUTPUT == 1
	printf("Debug Output: Initialising output file...\n");
#endif

	/** The ELF file header. */
	Elf32_Ehdr *elf_header = create_elf_header();

	// Find the index into the section header block of the section header
	// string table. This is needed by the ELF header.
	ssize_t section_shstrtab_index = find_section_index(sections, ".shstrtab");
	if(section_shstrtab_index == -1) {
		printf("Error finding `.shstrtab` index.\n");
	}

	elf_header->e_shstrndx = section_shstrtab_index;


#if DEBUG_OUTPUT == 1
	printf("Debug Output: Populating .shstrtab...\n");
#endif

	Section *shstrtab = find_section(sections, ".shstrtab");
	if(!shstrtab) {
		printf("Error finding `.shstrtab` section.\n");
	}

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
		string_entity->n_reloc_entries = 0;
		string_entity->reloc_entries = NULL;

		string_entity->size = section_name_len;
		string_entity->data = malloc(section_name_len);
		string_entity->data = memcpy(string_entity->data, curr_section->name,
			section_name_len);
		string_entity->data[section_name_len] = '\0';

		// Add the encoded string to the `shstrtab` section.
		section_add_encoding_entity(shstrtab, string_entity);

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
		printf("Debug Output: Opening output file `%s`...\n", "FFFFFFF");
#endif

	// Open the output file.
	FILE *out_file = fopen("./out.elf", "w");
	if(!out_file) {
		fprintf(stderr, "Error opening output file: `%u`\n", errno);
	}

	// Write the ELF file header.
	size_t written = fwrite(elf_header, sizeof(Elf32_Ehdr), 1, out_file);
	if(written != 1) {
		if(ferror(out_file)) {
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
			written = fwrite(curr_entity->data, curr_entity->size, 1, out_file);
			if(written != 1) {
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

		// Write each ELF header to the output file.
		written = fwrite(section_header, sizeof(Elf32_Shdr), 1, out_file);
		if(written != 1) {
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

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Closing output file...\n");
#endif

	fclose(out_file);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing statements...\n");
#endif

	free(elf_header);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing Symbol Table...\n");
#endif

	for(size_t i = 0; i < symbol_table.n_entries; i++) {
		free(symbol_table.symbols[i].name);
	}

	free(symbol_table.symbols);


#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing sections...\n");
#endif

	free_section(sections);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Finished.\n");
#endif
}
