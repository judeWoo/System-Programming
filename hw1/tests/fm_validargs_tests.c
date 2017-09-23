#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <string.h>
#include "__helpers.h"
#include "const.h"

#define HELP 0x8000
#define POLY 0x4000
#define FM 0x4000
#define ENCR 0x2000
#define DECR 0x2000
#define ROW 0x00F0
#define COL 0x000F


Test(fm_validargs_suite, 01_fm_encr) {
    char *args[] = {"bin/hw1", "-f", "-e", NULL};
    unsigned short ret = validargs(3, args);
    cr_assert_eq((ret & FM), FM, "Fractionated morse cipher bit wasn't set. Got: %s", binary_string(&ret, sizeof(ret)));
    cr_assert_eq((ret & ENCR), 0, "Encryption bit wasn't 0. Got: %s", binary_string(&ret, sizeof(ret)));
}

Test(fm_validargs_suite, 02_fm_decr) {
    char *args[] = {"bin/hw1", "-f", "-d", NULL};
    unsigned short ret = validargs(3, args);
    cr_assert_eq((ret & DECR), DECR, "Decryption bit wasn't set. Got: %s", binary_string(&ret, sizeof(ret)));
}

Test(fm_validargs_suite, 03_fm_valid_key) {
    char *args[] = {"bin/hw1", "-f", "-e", "-k", "CSE", NULL};
    unsigned short ret = validargs(5, args);
    cr_assert_eq((ret & FM), FM, "Fractionated morse cipher bit wasn't set. Got: %s", binary_string(&ret, sizeof(ret)));
    cr_assert_eq((ret & ENCR), 0, "Encryption bit wasn't 0. Got: %s", binary_string(&ret, sizeof(ret)));
    cr_assert_not_null(key, "Key was NULL");
    cr_assert_eq(key, "CSE", "Key was not set. Expected \'CSE\'. Got: %s", key);
}

Test(fm_validargs_suite, 04_fm_invalid_key) {
    char *args[] = {"bin/hw1", "-f", "-d", "-k", "ABCDEFGHIJJ", NULL};
    unsigned short ret = validargs(5, args);
    cr_assert_eq(ret, 0, "Mode was not 0. Got: %s", binary_string(&ret, sizeof(ret)));
}
