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

char *preprocess_line(char *line_buffer);

Statement *scan_string(const char *str);

Assembler_Process_Result read_input(FILE *input_file,
	Statement **program_statements);

#endif
