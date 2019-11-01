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
#include <stdlib.h>
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
	char** output)
{
	// All operations performed on the string are destructive.
	// Copy the line into a new buffer that we can modify.
	*output = strdup(line_buffer);
	if(!output) {
		return ASSEMBLER_ERROR_BAD_ALLOC;
	}

	/** A pointer to the current char being scanned. */
	char* scan = *output;
	/** A pointer to the last scanned character. */
	char* last_scanned_char = *output;

	// Trim leading whitespace.
	// Advancing the scan pointer here will cause the remaining string
	// contents to be moved into the starting position below.
	while(*scan != '\0' && isblank(*scan)) {
		++scan;
	}

	scan = memmove(last_scanned_char, scan, 1 + strlen(scan));

	// All whitespace is collapsed into a single space character.
	while(*scan != '\0') {
		if(isblank(*scan)) {
			// Move the remaining string contents to the next position from the last
			// scanned character and replace the whitespace char with a space.
			scan = memmove(last_scanned_char + 1, scan, 1 + strlen(scan));
			*scan = ' ';
		} else {
			if(*scan == '\"') {
				// The contents of string literals are not processed in any way.
				// This loop skips over the contents of the string literal by iterating the
				// contents until a matching closing quote is found.
				while(*++scan != '\"') {
					if(*scan == '\0') {
						fprintf(stderr, "Preprocessor Error: Unterminated string literal\n");

						// Clean up output buffer.
						free(output);
						output = NULL;

						return ASSEMBLER_STATUS_BAD_INPUT;
					}
				}
			} else if(*scan == '#') {
				// Terminate the string at any trailing comment char.
				*scan = '\0';
			}

			last_scanned_char = scan;
		}

		scan++;
	}

	// Remove any trailing whitespace.
	*last_scanned_char = '\0';

#if DEBUG_PREPROCESSOR == 1
	if(strlen(*output)) {
		printf("Debug Preprocessor: Processed: `%s`\n", *output);
	} else {
		printf("Debug Preprocessor: Line truncated by preprocessor.\n");
	}
#endif

	return ASSEMBLER_STATUS_SUCCESS;
}
