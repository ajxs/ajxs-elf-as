#include <CUnit/CUnit.h>
#include <CUnit/CUError.h>
#include <CUnit/Basic.h>
#include <stdbool.h>
#include <test.h>


int main(int argc, char **argv) {
	CU_ErrorCode status = CU_initialize_registry();
	if(status != CUE_SUCCESS) {
		return CU_get_error();
	}

	CU_pSuite codegen_test_suite = CU_add_suite("Codegen",
		init_codegen_test_suite, teardown_codegen_test_suite);
	if(!codegen_test_suite) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if(!CU_add_test(codegen_test_suite,
		"Encode R type instruction", test_encode_r_type)) {
		return CU_get_error();
	}

	if(!CU_add_test(codegen_test_suite,
		"Encode I type instruction", test_encode_i_type)) {
		return CU_get_error();
	}

	if(!CU_add_test(codegen_test_suite,
		"Encode J type instruction", test_encode_j_type)) {
		return CU_get_error();
	}

	CU_pSuite preprocessor_test_suite = CU_add_suite("Preprocessor",
		init_preprocessor_test_suite, teardown_preprocessor_test_suite);
	if(!preprocessor_test_suite) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if(!CU_add_test(preprocessor_test_suite,
		"Trim leading whitespace", test_trim_leading_whitespace)) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if(!CU_add_test(preprocessor_test_suite,
		"Trim internal whitespace", test_trim_internal_whitespace)) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if(!CU_add_test(preprocessor_test_suite,
		"Process line without trailing newline", test_no_trailing_newline)) {
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
