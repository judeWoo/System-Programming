#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *x = sf_malloc(sizeof(double) * 8);
    sf_free(x);
    /* Backward coalesce */
    void *y = sf_malloc(sizeof(double));
    sf_free(y);

    sf_snapshot();
    sf_varprint(x);
    sf_varprint(y);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
