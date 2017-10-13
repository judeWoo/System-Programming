#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *x = sf_malloc(1);
    void *y = sf_realloc(x, 40);
    sf_free(y);

    sf_snapshot();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
