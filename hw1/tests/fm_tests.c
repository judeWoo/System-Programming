#include <criterion/criterion.h>
#include <stdio.h>
#include <stdlib.h>
#include "__helpers.h"

#define DIR "sample_files/fm_files/"
#define BUF_SIZE 10000


void test_out(const char *expected_out, const char *outfile)
{
    int out;
    char *cmd = calloc(10000, sizeof(char));
    sprintf(cmd, "diff %s %s", expected_out, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS, "Output of file was incorrect");
    remove(outfile);
}

// basic test with no other characters
Test(fm_suite, 01_fm_basic_e) {
    char *infile = DIR "01_fm_basic_e_in.txt";
    char *expected_out = DIR "01_fm_basic_e_out.txt";
    char *outfile = "fm_test_01_out.txt";
    int out;

    char *cmd = calloc(BUF_SIZE, 1);
    sprintf(cmd, "cat %s | bin/hw1 -f -e > %s",
	    infile, outfile);

    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);
    test_out(expected_out, outfile);
    free(cmd);
}

// basic test with no other characters
Test(fm_suite, 02_fm_multi_space_e) {
    char *infile = DIR "02_fm_multi_space_e_in.txt";
    char *expected_out = DIR "02_fm_multi_space_e_out.txt";
    char *outfile = "fm_test_02_out.txt";
    int out;

    char *cmd = calloc(BUF_SIZE, 1);
    sprintf(cmd, "cat %s | bin/hw1 -f -e > %s",
        infile, outfile);

    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
         "Program did not exit with EXIT_SUCCESS. Got: %d", out);
    test_out(expected_out, outfile);
    free(cmd);
}

// test that newlines handled correctly
Test(fm_suite, 03_fm_newlines_e) {
    char *infile = DIR "04_fm_newlines_e_in.txt";
    char *expected_out = DIR "04_fm_newlines_e_out.txt";
    char *outfile = "fm_test_04_out.txt";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -e > %s", infile, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);

    test_out(expected_out, outfile);
    free(cmd);
}

// test thta they use the key arg properly
Test(fm_suite, 04_fm_with_key_e) {
    char *infile = DIR "05_fm_with_key_e_in.txt";
    char *expected_out = DIR "05_fm_with_key_e_out.txt";
    char *outfile = "fm_test_05_out.txt";
    char *key = "DEKU";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -e -k %s > %s", infile, key, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);

    test_out(expected_out, outfile);
    free(cmd);
}

// test a long all inclusive file encryption
Test(fm_suite, 05_fm_long_e) {
    char *infile = DIR "06_fm_long_e_in.txt";
    char *expected_out = DIR "06_fm_long_e_out.txt";
    char *outfile = "fm_test_06_out.txt";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -e > %s", infile, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);

    test_out(expected_out, outfile);
    free(cmd);
}

// invalid input
Test(fm_suite, 06_fm_invalid_input_e) {
    char *infile = DIR "07_fm_invalid_input_e_in.txt";
    char *outfile = "fm_test_07_out_.txt";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -e > %s", infile, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_FAILURE,
		 "Program did not exit with EXIT_FAILURE. Got: %d", out);
    // no expected out
    remove(outfile);
    free(cmd);
}

// decryption of basic output
Test(fm_suite, 07_fm_basic_d) {
    char *infile = DIR "08_fm_basic_d_in.txt";
    char *expected_out = DIR "08_fm_basic_d_out.txt";
    char *outfile = "fm_test_08_out.txt";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -d > %s", infile, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);

    test_out(expected_out, outfile);
    free(cmd);
    remove(outfile);
}

// Test that newlines handled correctly
Test(fm_suite, 08_fm_newlines_d) {
    char *infile = DIR "11_fm_newlines_d_in.txt";
    char *expected_out = DIR "11_fm_newlines_d_out.txt";
    char *outfile = "fm_test_11_out.txt";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -d > %s", infile, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);

    test_out(expected_out, outfile);
    free(cmd);
}

// Test that they key arg is used properly
Test(fm_suite, 09_fm_with_key_d) {
    char *infile = DIR "12_fm_with_key_d_in.txt";
    char *expected_out = DIR "12_fm_with_key_d_out.txt";
    char *outfile = "fm_test_12_out.txt";
    char *key = "RYOU";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -d -k %s > %s", infile, key, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);

    test_out(expected_out, outfile);
    free(cmd);
}

// Test a large file is handled correctly
Test(fm_suite, 10_fm_long_d) {
    char *infile = DIR "13_fm_long_d_in.txt";
    char *expected_out = DIR "13_fm_long_d_out.txt";
    char *outfile = "fm_test_13_out.txt";
    int out;
    char *cmd = calloc(BUF_SIZE, 1);

    sprintf(cmd, "cat %s | bin/hw1 -f -d  > %s", infile, outfile);
    out = WEXITSTATUS(system(cmd));
    cr_assert_eq(out, EXIT_SUCCESS,
		 "Program did not exit with EXIT_SUCCESS. Got: %d", out);

    test_out(expected_out, outfile);
    free(cmd);
}

#undef BUF_SIZE
#undef DIR
