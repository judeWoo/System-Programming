#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *x = sf_malloc(sizeof(double) * 8);
    void *y = sf_realloc(x, sizeof(int));

    // int *ptr1 = sf_malloc(48);
    // int *ptr2 = sf_malloc(sizeof(double));
    // int *ptr3 = sf_malloc(sizeof(double)*2000);

    sf_snapshot();
    sf_varprint(x);
    sf_varprint(y);
    // sf_varprint(ptr3);

    // printf("NOW LET'S FREE\n");
    // sf_free(ptr1);
    // sf_free(ptr2);
    // sf_free(ptr3);
    // sf_snapshot();

    // printf("NOW LET'S FREE AGAIN\n");
    // sf_free(ptr1);
    // sf_free(ptr2);
    // sf_free(ptr3);
    // sf_snapshot();

    // printf("NOW REALLOC\n");
    // int *ptr2 = sf_malloc(48);
    // sf_realloc(ptr2, 16);
    // sf_snapshot();
    // sf_varprint(ptr2);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
