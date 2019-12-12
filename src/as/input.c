/**
 * @file input.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Handles fle input.
 * Contains the functionality for reading and initial parsing of the input source
 * file.
 * @version 0.1
 * @date 2019-03-09
 */

#include <string.h>
#include <stdlib.h>
#include <as.h>
#include <input.h>
#include <statement.h>


/**
 * read_input
 */
Assembler_Status read_input(FILE* input_file,
	Statement** program_statements)
{
	/** The buffer holding the raw, unprocessed input line. */
	char* line_buffer = NULL;
	/** The length of the input line buffer, used by 'getline'. */
	size_t line_buffer_length = 0;
	/** The number of chars read from the input file. */
	ssize_t chars_read = 0;
	/** The number of the line being processed. */
	size_t line_num = 1;
	/** Buffer holding the preprocessed lines. */
	char* line = NULL;
	/** The program status. */
	Assembler_Status status = ASSEMBLER_STATUS_SUCCESS;

	// Read all the lines in the file.
	while((chars_read = getline(&line_buffer, &line_buffer_length, input_file)) != -1) {
#if DEBUG_INPUT == 1
	printf("Input line #%zu: `%s`", line_num, line_buffer);
#endif

		// Preprocess the line. Normalises the line to conform to a standard format.
		status = preprocess_line(line_buffer, &line);
		if(!get_status(status)) {
			// Free the line buffer.
			// The processed line buffer will have been freed by the callee.
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
		Statement* parsed_statements = scan_string(line);

		// Iterate through each processed statement and set its line number.
		Statement* curr = parsed_statements;
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
		line = NULL;

		line_num++;
	}

#if DEBUG_PARSED_STATEMENTS == 1
	// Iterate over all parsed statements, printing each one.
	Statement* curr = *program_statements;

	while(curr) {
		print_statement(curr);
		curr = curr->next;
	}
#endif

	// Prevent memory leak. Refer to:
	// https://stackoverflow.com/questions/55731141/memory-leak-when-reading-file-line-by-line-using-getline
	free(line_buffer);

	return ASSEMBLER_STATUS_SUCCESS;
}
