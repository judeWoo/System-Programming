#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    double* ptr1 = sf_malloc(259);
    double* ptr2 = sf_malloc(sizeof(double) * 100);
    double* ptr3 = sf_malloc(sizeof(double) * 400);
    double* ptr4 = sf_malloc(sizeof(double) * 10);
    double* ptr5 = sf_malloc(sizeof(double));
    // int* ptr = sf_malloc(sizeof(int));
    *ptr1 = 320320320e-10; // Ae-N: A * 10^(-N)
    *ptr2 = 320.320320;
    *ptr3 = 320.320320;
    *ptr4 = 320.320320;
    *ptr5 = 320.320320;

    sf_snapshot();
    sf_varprint(ptr1);
    sf_varprint(ptr2);
    sf_varprint(ptr3);
    sf_varprint(ptr4);
    sf_varprint(ptr5);
    // sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
