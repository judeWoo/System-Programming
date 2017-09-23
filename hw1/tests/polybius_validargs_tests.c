#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "__helpers.h"
#include "const.h"

#define HELP 0x8000
#define POLY 0x4000
#define FM 0x4000
#define ENCR 0x2000
#define DECR 0x2000
#define ROW 0x00F0
#define COL 0x000F


// no program arguments besides name (error)
Test(polybius_validargs_suite, 01_no_args) {
	char* args[] = {"bin/hw1", NULL};
	unsigned short ret = validargs(1, args);
	cr_assert_eq(ret, 0, "Expected 0. Got: %s", binary_string(&ret, sizeof(ret)));
}

// -h flag
Test(polybius_validargs_suite, 02_help_menu) {
	char* args[] = {"bin/hw1", "-h", NULL};
	unsigned short ret = validargs(2, args);
	cr_assert_eq((ret & HELP), HELP, "Help menu bit wasn't set. Got: %s", binary_string(&ret, sizeof(ret)));
}

Test(polybius_validargs_suite, 03_poly_encr) {
	char* args[] = {"bin/hw1", "-p", "-e", NULL};
	unsigned short ret = validargs(3, args);
	cr_expect_eq((ret & POLY), 0, "Polybius cipher bit wasn't 0. Got: %s", binary_string(&ret, sizeof(ret)));
	cr_expect_eq((ret & ENCR), 0, "Encryption bit wasn't 0. Got: %s", binary_string(&ret, sizeof(ret)));
}

Test(polybius_validargs_suite, 04_poly_dec_rows_cols) {
	char* args[] = {"bin/hw1", "-p", "-d", "-r", "14", "-c", "9", NULL};
	unsigned short ret = validargs(7, args);
	cr_expect_eq((ret & DECR), DECR, "Decryption bit wasn't set. Got: %s", binary_string(&ret, sizeof(ret)));
	cr_expect_eq((ret & ROW), 0x00E0, "Row bits were not 14. Got: %s", binary_string(&ret, sizeof(ret)));
	cr_expect_eq((ret & COL), 0x0009, "Column bits were not 9. Got: %s", binary_string(&ret, sizeof(ret)));
}

Test(polybius_validargs_suite, 05_poly_with_valid_key) {
	char* args[] = {"bin/hw1", "-p", "-e", "-k", "HELO", NULL};
	unsigned short ret = validargs(5, args);
	cr_expect_eq((ret & POLY), 0, "Polybius cipher bit wasn't 0. Got: %s", binary_string(&ret, sizeof(ret)));
	cr_expect_eq((ret & ENCR) , 0, "Encryption bit wasn't 0. Got: %s", binary_string(&ret, sizeof(ret)));
	cr_assert_not_null(key, "Key was NULL");
	cr_expect_str_eq(key, "HELO", "Key was not set. Expected: HELO. Got: %s", key);
}

Test(polybius_validargs_suite, 06_poly_invalid_rows) {
	char* args[] = {"bin/hw1", "-p", "-e", "-r", "20", NULL};
	unsigned short ret = validargs(4, args);
	cr_assert_eq(ret, 0, "Mode was not 0. Got: %s", binary_string(&ret, sizeof(ret)));
}

Test(polybius_validargs_suite, 07_poly_invalid_key) {
	char* args[] = {"bin/hw1", "-p", "-d", "-k", "HELLO", NULL};
	unsigned short ret = validargs(5, args);
	cr_assert_eq(ret, 0, "Mode was not 0. Got: %s", binary_string(&ret, sizeof(ret)));
}
