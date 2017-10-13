#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *x = sf_malloc(1);

    sf_snapshot();
    sf_varprint(x);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
