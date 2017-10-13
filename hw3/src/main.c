#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    // for (int i = 0; i < (PAGE_SZ * 4 / 32) - 1; ++i)
    // {
    //     double* ptr = sf_malloc(4.2);
    //     *ptr = 320320320e-10; // Ae-N: A * 10^(-N)

    // }

    int *ptr1 = sf_malloc(48);
    // int *ptr2 = sf_malloc(sizeof(double));
    // int *ptr3 = sf_malloc(sizeof(double)*2000);

    sf_snapshot();
    sf_varprint(ptr1);
    // sf_varprint(ptr2);
    // sf_varprint(ptr3);

    printf("NOW LET'S FREE\n");
    sf_free(ptr1);
    // sf_free(ptr2);
    // sf_free(ptr3);
    sf_snapshot();

    printf("NOW REALLOC\n");
    int *ptr2 = sf_malloc(48);
    sf_realloc(ptr2, 16);
    sf_snapshot();
    sf_varprint(ptr2);



    sf_mem_fini();

    return EXIT_SUCCESS;
}
