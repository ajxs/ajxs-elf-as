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


Assembler_Status preprocess_line(const char* line_buffer,
	char **output);

Statement* scan_string(const char* str);

Assembler_Status read_input(FILE* input_file,
	Statement** program_statements);

#endif
