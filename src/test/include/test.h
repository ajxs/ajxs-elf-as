/**
 * Codegen test suite.
 */
int init_codegen_test_suite(void);
int teardown_codegen_test_suite(void);

void test_encode_i_type(void);
void test_encode_j_type(void);
void test_encode_r_type(void);

/**
 * Preprocessor test suite.
 */
int init_preprocessor_test_suite(void);
int teardown_preprocessor_test_suite(void);

void test_trim_leading_whitespace(void);
void test_trim_internal_whitespace(void);
void test_no_trailing_newline(void);
