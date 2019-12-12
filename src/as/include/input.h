/**
 * @file input.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Input function header.
 * Contains functions for reading and processing the input file.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef INPUT_H
#define INPUT_H 1


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
	char** output);

/**
 * @brief Entry point to parsing an input line.
 *
 * This is the entry point to lexing and parsing an individual line of input. This
 * function invokes the code created from Flex/Bison.
 * @param str The string to lex/parse
 * @return A linked list of parsed statements.
 */
Statement* scan_string(const char* str);

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
Assembler_Status read_input(FILE* input_file,
	Statement** program_statements);

#endif
