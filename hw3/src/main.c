#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *x = sf_malloc(sizeof(double) * 8);
    void *y = sf_realloc(x, sizeof(int));

    sf_snapshot();
    sf_varprint(x);
    sf_varprint(y);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
