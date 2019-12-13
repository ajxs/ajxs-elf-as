#include <CUnit/CUError.h>
#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdint.h>
#include <as.h>
#include <arch.h>
#include <codegen.h>
#include <operand.h>
#include <section.h>
#include <stdlib.h>
#include <symtab.h>
#include <test.h>

int init_preprocessor_test_suite(void) {
	return 0;
}


int teardown_preprocessor_test_suite(void) {
	return 0;
}


/**
 * Tests that the preprocessor will trim leading whitespace.
 */
void test_trim_leading_whitespace(void)
{
	char* test_line = "    ADDI $t0, $t1, 0x50\n";
	char* output = NULL;
	char* expected_output = "ADDI $t0, $t1, 0x50";

	Assembler_Status status = preprocess_line(test_line, &output);

	CU_ASSERT(strcmp(output, expected_output) == 0);
}


/**
 * Tests that the preprocessor will trim internal whitespace.
 */
void test_trim_internal_whitespace(void)
{
	char* test_line = "    message: .asciiz    \"just    testing\n\"\n";
	char* output = NULL;
	char* expected_output = "message: .asciiz \"just    testing\n\"";

	Assembler_Status status = preprocess_line(test_line, &output);

	CU_ASSERT(strcmp(output, expected_output) == 0);
}


/**
 * Tests that the preprocessor will deal with there being no trailing endline.
 */
void test_no_trailing_newline(void)
{
	char* test_line = "    ADDI $t0, $t1, 0x50";
	char* output = NULL;
	char* expected_output = "ADDI $t0, $t1, 0x50";

	Assembler_Status status = preprocess_line(test_line, &output);

	CU_ASSERT(strcmp(output, expected_output) == 0);
}
