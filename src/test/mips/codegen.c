#include <CUnit/CUnit.h>
#include <CUnit/CUError.h>
#include <CUnit/Basic.h>
#include <stdbool.h>


int init_test_suite(void);
int teardown_test_suite(void);
void test_encode_r_type(void);


int init_test_suite(void) {
	return 0;
}


int teardown_test_suite(void) {
	return 0;
}


int main(int argc, char **argv) {
	CU_pSuite pSuite = NULL;

	CU_ErrorCode status;

	status = CU_initialize_registry();
	if(status != CUE_SUCCESS) {
		return CU_get_error();
	}


	/* add a suite to the registry */
	pSuite = CU_add_suite("Codegen test", init_test_suite, teardown_test_suite);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}


	/* add the tests to the suite */
	if(NULL == CU_add_test(pSuite, "test encoding R type", test_encode_r_type)) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();

	return 0;
}


void test_encode_r_type(void) {
	printf("FFFFF");
	CU_ASSERT(true);
}
