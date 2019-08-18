/**
 * @file preprocessor.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for preprocessing file input.
 * Contains functions for preprocessing file input prior to the parsing process.
 * Helps in normalising the source input file.
 * @version 0.1
 * @date 2019-03-09
 */


#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <as.h>
#include <input.h>


/**
 * @brief Preprocesses a line of input source.
 *
 * Preprocesses a line of input source, normalising it to ease parsing of the text.
 * All leading whitespace is removed, all mid-line whitespace is collapsed to a
 * single space character.
 * @param line_buffer The line to process.
 * @return The processed line.
 * @warning The returned line is allocated in this function, and must be freed
 * by the caller.
 */
Assembler_Status preprocess_line(const char* line_buffer,
	char** output) {

	Assembler_Status status = ASSEMBLER_STATUS_SUCCESS;

	// Copy the line into a new buffer that we can modify.
	// All operations from here are destructive.
	*output = strdup(line_buffer);

	char *scan = *output;
	char *last_char_pos = *output;

	const char *preprocessor_error = NULL;

	// Trim leading whitespace.
	// Advance to first non-whitespace char and shift all
	// chars from here back to the start of the line.
	while(*scan != '\0' && isblank(*scan)) {
		++scan;
	}

	scan = memmove(last_char_pos, scan, 1 + strlen(scan));

	// Collapse all whitespace.
	while(*scan != '\0') {
		if(isblank(*scan)) {
			scan = memmove(last_char_pos + 1, scan, 1 + strlen(scan));
			*scan = ' ';
		} else {
			if(*scan == '\"') {
				// Do not alter the contents of string literals.
				while(*++scan != '\"') {
					if(*scan == '\0') {
						preprocessor_error = "Preprocessor Error: Unterminated string literal.";
						status = ASSEMBLER_STATUS_BAD_INPUT;
						goto PREPROCESSOR_FAILURE;
					}
				}
			} else if(*scan == '#') {
				// Terminate the string at any comment char.
				*scan = '\0';
			}

			last_char_pos = scan;
		}

		scan++;
	}

	// Remove trailing whitespace.
	*last_char_pos = '\0';

#if DEBUG_PREPROCESSOR == 1
	if(strlen(*output)) {
		printf("Debug Preprocessor: Processed: `%s`\n", *output);
	} else {
		printf("Debug Preprocessor: Line truncated by preprocessor.\n");
	}
#endif

	return status;

PREPROCESSOR_FAILURE:
	return status;
}
