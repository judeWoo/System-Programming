#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "__helpers.h"
#include "const.h"

#define DIR "sample_files/polybius_files/"

void test_cmd(char *out_file, char *expected_out, char *cmd) {
	cr_assert_eq(system(cmd), EXIT_SUCCESS, "Program didn't exit successfully");
	memset(cmd, 0, 1000);
	sprintf(cmd, "diff %s %s", out_file, expected_out);
	int out = system(cmd);

	if(WIFEXITED(out)) {
		cr_assert_eq(WEXITSTATUS(out), EXIT_SUCCESS, "Program output was incorrect. Did not match contents of: %s", expected_out);
	}
	else {
		cr_assert_not(WIFSIGNALED(out), "Program exited with %d", WTERMSIG(out));
	}

	remove(out_file);
	free(cmd);
}

Test(polybius_suite, 01_polybius_simple_from_stdin) {
	char *in_text = "I LOVE CSE320!!";
	char *out_file = "01_polybius_out.txt";
	char *expected_out = DIR "simple_out.txt";
	char *cmd = calloc(1000, 1);
	sprintf(cmd, "echo %s | bin/hw1 -p -e > %s", in_text, out_file);
	test_cmd(out_file, expected_out, cmd);
}

Test(polybius_suite, 02_mixed_case) {
	char *in_file = DIR "mixedcase.txt";
	char *out_file = "02_polybius_out.txt";
	char *expected_out = DIR "mixedcase_out.txt";
	char *cmd = calloc(1000, 1);

	sprintf(cmd, "cat %s | bin/hw1 -p -e > %s", in_file, out_file);
	test_cmd(out_file, expected_out, cmd);
}

Test(polybius_suite, 03_whitespace) {
	char *in_file = DIR "whitespace.txt";
	char *out_file = "03_polybius_out.txt";
	char *expected_out = DIR "whitespace_out.txt";
	char *cmd = calloc(1000, 1);

	sprintf(cmd, "cat %s | bin/hw1 -p -e > %s", in_file, out_file);
	test_cmd(out_file, expected_out, cmd);
}

Test(polybius_suite, 04_key) {
	char *in_file = DIR "simple_in.txt";
	char *out_file = "04_polybius_out.txt";
	char *expected_out = DIR "simple_with_key_out.txt";
	char *key = "\"CSE#@)\"";
	char *cmd = calloc(1000, 1);

	sprintf(cmd, "cat %s | bin/hw1 -p -e -k %s > %s", in_file, key, out_file);
	test_cmd(out_file, expected_out, cmd);
}

Test(polybius_suite, 05_rows_cols) {
	char *in_file = DIR "simple_in.txt";
	char *out_file = "05_polybius_out.txt";
	char *expected_out = DIR "simple_rows_cols_out.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -e -r 12 -c 12 > %s", in_file, out_file);
	test_cmd(out_file, expected_out, cmd);
}

Test(polybius_suite, 06_full_encrypt) {
	char *in_file = DIR "gulliver.txt";
	char *out_file = "06_polybius_out.txt";
	char *expected = DIR "gulliver_enc.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -e -r 11 -c 9 -k GuLiveR > %s", in_file, out_file);
	test_cmd(out_file, expected, cmd);
}

Test(polybius_suite, 07_simple_decrypt) {
	char *in_file = DIR "simple_out.txt";
	char *out_file = "07_polybius_out.txt";
	char *expected = DIR "simple_in.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -d > %s", in_file, out_file);
	test_cmd(out_file, expected, cmd);
}

Test(polybius_suite, 08_mixedcase_decrypt) {
	char *in_file = DIR "mixedcase_out.txt";
	char *out_file = "08_polybius_out.txt";
	char *expected = DIR "mixedcase.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -d > %s", in_file, out_file);
	test_cmd(out_file, expected, cmd);
}

Test(polybius_suite, 09_whitespace_decrypt) {
	char *in_file = DIR "whitespace_out.txt";
	char *out_file = "09_polybius_out.txt";
	char *expected = DIR "whitespace.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -d > %s", in_file, out_file);
	test_cmd(out_file, expected, cmd);
}

Test(polybius_suite, 10_key_decrypt) {
	char *in_file = DIR "simple_with_key_out.txt";
	char *out_file = "10_polybius_out.txt";
	char *expected_out = DIR "simple_in.txt";
	char *key = "\"CSE#@)\"";
	char *cmd = calloc(1000, 1);

	sprintf(cmd, "cat %s | bin/hw1 -p -d -k %s > %s", in_file, key, out_file);
	test_cmd(out_file, expected_out, cmd);
}

Test(polybius_suite, 11_rows_cols_decrypt) {
	char *in_file = DIR "simple_rows_cols_out.txt";
	char *out_file = "11_polybius_out.txt";
	char *expected_out = DIR "simple_in.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -d -r 12 -c 12 > %s", in_file, out_file);
	test_cmd(out_file, expected_out, cmd);
}

Test(polybius_suite, 12_full_decrypt) {
	char *in_file = DIR "gulliver_enc.txt";
	char *out_file = "12_polybius_out.txt";
	char *expected = DIR "gulliver.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -d -r 11 -c 9 -k GuLiveR > %s", in_file, out_file);
	test_cmd(out_file, expected, cmd);
}

Test(polybius_suite, 13_enc_dec) {
	char *plain_file = DIR "gulliver.txt";
	char *out_file = "13_polybius_out.txt";

	char *cmd = calloc(1000, 1);
	sprintf(cmd, "cat %s | bin/hw1 -p -e -r 15 -c 15 -k CSE3@0 | bin/hw1 -p -d -r 15 -c 15 -k CSE3@0 > %s", plain_file, out_file);
	system(cmd);
	memset(cmd, 0, 1000);
	sprintf(cmd, "diff %s %s", plain_file, out_file);
	cr_assert_eq(system(cmd), EXIT_SUCCESS, "Program didn't exit successfully");
	remove(out_file);
	free(cmd);
}