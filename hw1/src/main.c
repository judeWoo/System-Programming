#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;

    mode = validargs(argc, argv);

    debug("Mode: 0x%X", mode);

    if((((mode >> 14) & 1) == 0) && ((mode & 0x8000) == 0) && mode != 0x0000) {
        if(poly_cipher(mode, polybius_alphabet) == 0){
            return EXIT_FAILURE;
        }
    }

    if((((mode >> 14) & 1) == 1) && ((mode & 0x8000) == 0) && mode != 0x0000) {
        if(fm_cipher(mode, fm_alphabet) == 0){
            return EXIT_FAILURE;
        }
    }

    if(mode & 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */