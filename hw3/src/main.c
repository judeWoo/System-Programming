#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    for (int i = 0; i < (PAGE_SZ * 4 / 32) - 1; ++i)
    {
        double* ptr = sf_malloc(4.2);
        *ptr = 320320320e-10; // Ae-N: A * 10^(-N)

    }

    sf_snapshot();
    // sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
