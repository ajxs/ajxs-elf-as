#include <as.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void free_program_statement(Parsed_Statement *parsed_statement) {
	if(parsed_statement->next) {
		free_program_statement(parsed_statement->next);
	}

	free_statement(parsed_statement->statement);

	free(parsed_statement);
};


void assemble_first_pass(Section *sections,
	size_t n_sections,
	Symbol_Table *symbol_table,
	Parsed_Statement *statements) {

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Begin first pass...\n");
#endif

	Section *section_text = find_section(sections, n_sections, ".text");
	Section *section_data = find_section(sections, n_sections, ".data");
	Section *section_bss = find_section(sections, n_sections, ".bss");
	// Start in the .text section by default.
	Section *section_current = section_text;

	Parsed_Statement *curr = statements;

	while(curr) {
		// All labels must be processed first.
		// Since a label _can_ precede a section directive, but not the other way around.
		if(curr->statement.labels) {
			for(size_t i = 0; i < curr->statement.n_labels; i++) {
				symtab_add_symbol(symbol_table, curr->statement.labels[i], section_current,
					section_current->program_counter);
			}
		}

		// Process section directives.
		// These are directives which specify which section to place the following
		// statements in. Adjust the current section accordingly.
		// These have a size of zero, as returned from `get_statement_size`.
		if(curr->statement.type == STATEMENT_TYPE_DIRECTIVE) {
			if(curr->statement.body.directive.type == DIRECTIVE_BSS) {
				section_current = section_bss;
			} else if(curr->statement.body.directive.type == DIRECTIVE_DATA) {
				section_current = section_data;
			} else if(curr->statement.body.directive.type == DIRECTIVE_TEXT) {
				section_current = section_text;
			}
		}

		ssize_t statement_size = get_statement_size(curr->statement);
		if(statement_size == -1) {
			printf("ERROR\n");
		}

#if DEBUG_ASSEMBLER == 1
		printf("Debug Assembler: Calculated size `0x%lx` for ", statement_size);
		if(curr->statement.type == STATEMENT_TYPE_DIRECTIVE) {
			printf("directive type `%i`.\n", curr->statement.body.directive.type);

		} else if(curr->statement.type == STATEMENT_TYPE_INSTRUCTION) {
			printf("instruction `");
			print_opcode(curr->statement.body.instruction.opcode);
			printf("`.\n");
		} else {
			// Empty statement
			printf("empty statement.\n");
		}
#endif

		// Increment the program counter by the size of the statement found.
		section_current->program_counter += (size_t)statement_size;
		curr = curr->next;
	}
}


void assemble_second_pass(Section *sections,
	size_t n_sections,
	Symbol_Table *symbol_table,
	Parsed_Statement *statements) {

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Begin second pass...\n");
#endif

	// Ensure all section program counters counters are reset.
	// These will have been set by the first assembly pass.
	for(uint8_t i=0; i < n_sections; i++) {
		sections[i].program_counter = 0;
	}

	Section *section_text = find_section(sections, n_sections, ".text");
	Section *section_data = find_section(sections, n_sections, ".data");
	Section *section_bss = find_section(sections, n_sections, ".bss");
	// Start in the .text section by default.
	Section *section_current = section_text;

	Parsed_Statement *curr = statements;

	Encoding_Entity *encoding = NULL;

	while(curr) {
		if(curr->statement.type == STATEMENT_TYPE_DIRECTIVE) {
			switch(curr->statement.body.directive.type) {
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
					encoding = encode_directive(symbol_table, &curr->statement.body.directive,
						section_current->program_counter);
					section_current->program_counter += encoding->size;
					section_add_encoding_entity(section_current, encoding);
			}
		} else if(curr->statement.type == STATEMENT_TYPE_INSTRUCTION) {
			encoding = encode_instruction(symbol_table, curr->statement.body.instruction,
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
	printf("Debug Assembler: Finished second pass.\n");
#endif
}


void assemble(FILE *input_file) {
#if DEBUG_INPUT == 1
	uint32_t line_number = 1;
#endif

	char *line_buffer = 0;
	size_t line_buffer_length = 0;
	ssize_t chars_read = 0;
	size_t line_num = 1;

	Parsed_Statement *program_statements = NULL;

	// Read all the lines in the file.
	while((chars_read = getline(&line_buffer, &line_buffer_length, input_file)) != -1) {
#if DEBUG_INPUT == 1
	printf("Input line #%u: `%s`", line_number, line_buffer);
#endif

		// Preprocess the line. Normalises the line to conform to a standard format.
		// If NULL is returned, assume error state and abort.
		char *line = preprocess_line(line_buffer);
		if(!line) {
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
		Parsed_Statement *parsed_statements = scan_string(line);
		Parsed_Statement *curr = parsed_statements;
		curr->line_num = line_num;
		while(curr->next) {
			curr = curr->next;
			curr->line_num = line_num;
		}

		if(!program_statements) {
			program_statements = parsed_statements;
		} else {
			Parsed_Statement *curr = program_statements;
			while(curr->next) {
				curr = curr->next;
			}

			curr->next = parsed_statements;
		}

		line_num++;
	}


#if DEBUG_PARSED_STATEMENTS == 1
	// Iterate over all parsed statements, printing each one.
	Parsed_Statement *curr = program_statements;

	while(curr) {
		print_statement(curr->statement);
		curr = curr->next;
	}
#endif


#define ENCODE_SYMBOLS 1

	/** The executable symbol table. */
	Symbol_Table symbol_table;
	symbol_table.n_entries = 0;
	symbol_table.symbols = NULL;

	/** The number of ELF sections in the executable. */
	size_t n_sections = 7;

	/** The ELF section data. */
	Section *sections = malloc(sizeof(Section) * n_sections);

	// The section header data will be filled as the sections are serialised.
	sections[0].name = "\0";
	sections[0].name_strtab_offset = 0;
	sections[0].program_counter = 0;
	sections[0].file_offset = 0;
	sections[0].size = 0;
	sections[0].flags = 0;
	sections[0].link = 0;
	sections[0].type = SHT_NULL;
	sections[0].encoding_entities = NULL;

	sections[1].name = ".text";
	sections[1].name_strtab_offset = 11;
	sections[1].program_counter = 0;
	sections[1].file_offset = 0;
	sections[1].size = 0;
	sections[1].flags = SHF_ALLOC | SHF_EXECINSTR;
	sections[1].link = 0;
	sections[1].type = SHT_PROGBITS;
	sections[1].encoding_entities = NULL;

	sections[2].name = ".data";
	sections[2].name_strtab_offset = 6;
	sections[2].program_counter = 0;
	sections[2].file_offset = 0;
	sections[2].size = 0;
	sections[2].flags = SHF_ALLOC | SHF_WRITE;
	sections[2].link = 0;
	sections[2].type = SHT_PROGBITS;
	sections[2].encoding_entities = NULL;

	sections[3].name = ".bss";
	sections[3].name_strtab_offset = 0;
	sections[3].program_counter = 0;
	sections[3].file_offset = 0;
	sections[3].size = 0;
	sections[3].link = 0;
	sections[3].flags = SHF_ALLOC | SHF_WRITE;
	sections[3].type = SHT_NOBITS;
	sections[3].encoding_entities = NULL;

	sections[4].name = ".symtab";
	sections[4].name_strtab_offset = 0;
	sections[4].program_counter = 0;
	sections[4].file_offset = 0;
	sections[4].size = 0;
	sections[4].flags = SHF_ALLOC;
	sections[4].link = 6;
	sections[4].type = SHT_SYMTAB;
	sections[4].encoding_entities = NULL;

	sections[5].name = ".shstrtab";
	sections[5].name_strtab_offset = 0;
	sections[5].program_counter = 0;
	sections[5].file_offset = 0;
	sections[5].size = 0;
	sections[5].flags = SHF_ALLOC;
	sections[5].link = 0;
	sections[5].type = SHT_STRTAB;
	sections[5].encoding_entities = NULL;

	sections[6].name = ".strtab";
	sections[6].name_strtab_offset = 0;
	sections[6].program_counter = 0;
	sections[6].file_offset = 0;
	sections[6].size = 0;
	sections[6].link = 0;
	sections[6].type = SHT_STRTAB;
	sections[6].encoding_entities = NULL;

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Beginning macro expansion...\n");
#endif

	// Loop through all statements, expanding all macros.
	expand_macros(program_statements);

	// Begin the first assembler pass. Populating the symbol table.
	assemble_first_pass(sections, n_sections, &symbol_table, program_statements);

#if DEBUG_SYMBOLS == 1
	// Print the symbol table.
	printf("Debug Assembler: Symbol Table:\n");
	for(size_t i = 0; i < symbol_table.n_entries; i++) {
		printf("  Symbol: `%s` in `%s` at `%#zx`\n", symbol_table.symbols[i].name,
			symbol_table.symbols[i].section->name, symbol_table.symbols[i].offset);
	}
#endif

	// Begin the second assembler pass, which handles code generation.
	assemble_second_pass(sections, n_sections, &symbol_table, program_statements);

#if DEBUG_OUTPUT == 1
	printf("Debug Output: Initialising output file...\n");
#endif

	/** The ELF header. */
	Elf32_Ehdr elf_header;

	// @TODO: Resolve the fact that this is currently encoding the executable
	// in LE format by default.
	// Ideally this would be configurable per target.
	char EI_IDENT[EI_NIDENT] = {
		0x7F, 'E', 'L', 'F',
		ELFCLASS32,
		ELFDATA2LSB,
		EV_CURRENT,
		ELFOSABI_SYSV,
		0,
		0, 0, 0, 0, 0, 0, 0
	};

	memcpy(&elf_header.e_ident, &EI_IDENT, EI_NIDENT);
	elf_header.e_type = ET_REL;
	elf_header.e_machine = EM_MIPS;
	elf_header.e_version = EV_CURRENT;
	elf_header.e_entry = 0;
	elf_header.e_phoff = 0;
	elf_header.e_shoff = 0;
	// @TODO - Temporarily hardcoding MIPS values.
	elf_header.e_flags = 0x90000400;
	elf_header.e_ehsize = sizeof(Elf32_Ehdr);
	elf_header.e_phentsize = 0;
	elf_header.e_phnum = 0;
	elf_header.e_shentsize = sizeof(Elf32_Shdr);
	elf_header.e_shnum = n_sections;
	elf_header.e_shstrndx = 5;


#if DEBUG_OUTPUT == 1
	printf("Debug Output: Populating .shstrtab...\n");
#endif

	Section *shstrtab = find_section(sections, n_sections, ".shstrtab");

	for(size_t i=0; i<n_sections; i++) {
		// Iterate through each section and add its name to the section header
		// string table, recording the index.
		size_t section_name_len = strlen(sections[i].name) + 1;
		// The current section size is the offset of each section name into SHSTRTAB.
		sections[i].name_strtab_offset = shstrtab->size;

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Adding section name: `%s` to .shstrtab at offset `0x%lx`...\n",
			sections[i].name, shstrtab->size);
#endif

		// Create an encoding entity for each section name, this will be encoded
		// during the writing of the section data.
		Encoding_Entity *string_entity = malloc(sizeof(Encoding_Entity));
		string_entity->n_reloc_entries = 0;
		string_entity->reloc_entries = NULL;

		string_entity->size = section_name_len;
		string_entity->data = malloc(section_name_len);
		string_entity->data = memcpy(string_entity->data, sections[i].name,
			section_name_len);
		string_entity->data[section_name_len] = '\0';

		section_add_encoding_entity(shstrtab, string_entity);
	}


#if ENCODE_SYMBOLS == 1
#if DEBUG_OUTPUT == 1
	printf("Debug Output: Populating .symtab...\n");
#endif

	Section *strtab = find_section(sections, n_sections, ".strtab");
	Section *symtab = find_section(sections, n_sections, ".symtab");

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

	for(size_t i = 0; i < symbol_table.n_entries; i++) {
		// Add each symbol name to the string table, and each symbol entry to the
		// symbol table section.

		Elf32_Sym symbol_entry;
		symbol_entry.st_name = strtab->size;
		symbol_entry.st_value = symbol_table.symbols[i].offset;
		symbol_entry.st_size = 0;
		symbol_entry.st_info = 0;
		symbol_entry.st_other = 0;

		// Iterate through each section to find the correct st_shndx
		symbol_entry.st_shndx = -1;
		for(size_t k=0; k<n_sections; k++) {
			if(strcmp(sections[k].name, symbol_table.symbols[i].section->name) == 0) {
				symbol_entry.st_shndx = k;
			}
		}

		// If we could not match the section index, abort.
		if(symbol_entry.st_shndx == -1) {
			printf("ERROR FINDING SYMBOL SECTION INDEX.\n");
		}

#if DEBUG_OUTPUT == 1
			printf("Debug Output: Matched section index to: `%s` for symbol name `%s`.\n",
				sections[symbol_entry.st_shndx].name, symbol_table.symbols[i].name);
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
			symbol_table.symbols[i].name, symtab->size);
#endif

		// Create an encoding entity for each symbol name, this will be encoded
		// in the string table during the writing of the section data.
		Encoding_Entity *symbol_name_entity = malloc(sizeof(Encoding_Entity));
		symbol_name_entity->n_reloc_entries = 0;
		symbol_name_entity->reloc_entries = NULL;

		size_t symbol_name_len = strlen(symbol_table.symbols[i].name) + 1;
		symbol_name_entity->size = symbol_name_len;
		symbol_name_entity->data = malloc(symbol_name_len);
		symbol_name_entity->data = memcpy(symbol_name_entity->data,
			symbol_table.symbols[i].name, symbol_name_len);
		symbol_name_entity->data[symbol_name_len] = '\0';

		section_add_encoding_entity(strtab, symbol_name_entity);

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Added symbol name: `%s` to .strtab at offset `0x%lx`.\n",
			symbol_table.symbols[i].name, strtab->size);
#endif
	}
#endif

	/** The total size of all section data. */
	size_t total_section_data_size = 0;

	for(size_t i=0; i<n_sections; i++) {
		total_section_data_size += sections[i].size;
	}

	// Set the section header offset to after the ELF header, and
	// after the section data.
	elf_header.e_shoff = elf_header.e_ehsize + total_section_data_size;

	FILE *out_file = fopen("./out.elf", "w");
	if(!out_file) {
		fprintf(stderr, "Error opening output file: `%u`\n", errno);
	}

	// Write header.
	size_t written = fwrite(&elf_header, sizeof(Elf32_Ehdr), 1, out_file);
	if(written != 1) {
		printf("ERROR\n");
	}

	// Write section data.
	for(size_t i=0; i<n_sections; i++) {
		// Store the current file offset as the offset of this section in the file.
		sections[i].file_offset = ftell(out_file);

#if DEBUG_OUTPUT == 1
		printf("Debug Output: Writing section: `%s` with size: `0x%lx`at `0x%lx`...\n",
			sections[i].name, sections[i].size, sections[i].file_offset);
#endif

		Encoding_Entity *curr_entity = sections[i].encoding_entities;
		while(curr_entity) {
			written = fwrite(curr_entity->data, 1, curr_entity->size, out_file);
			curr_entity = curr_entity->next;
		}
	}

	for(size_t i=0; i<n_sections; i++) {
#if DEBUG_OUTPUT == 1
		printf("Debug Output: Writing section header `%s` with offset `0x%lx` at `0x%lx`...\n",
			sections[i].name, sections[i].file_offset, ftell(out_file));
#endif

		Elf32_Shdr section_header;
		section_header.sh_name = sections[i].name_strtab_offset;
		section_header.sh_type = sections[i].type;
		section_header.sh_flags = sections[i].flags;
		section_header.sh_addr = 0;
		section_header.sh_offset = sections[i].file_offset;
		section_header.sh_size = sections[i].size;
		section_header.sh_link = sections[i].link;
		section_header.sh_info = 0;
		section_header.sh_addralign = 0;
		section_header.sh_entsize = 0;

		written = fwrite(&section_header, 1, sizeof(Elf32_Shdr), out_file);
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Cleaning up main program.\n");
#endif

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Freeing statements...\n");
#endif

	free_program_statement(program_statements);

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Closing output file...\n");
#endif

	fclose(out_file);

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

	for(size_t i=0; i<n_sections; i++) {
		free_section(&sections[i]);
	}

#if DEBUG_ASSEMBLER == 1
	printf("Debug Assembler: Finished.\n");
#endif
}
