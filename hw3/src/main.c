#include <stdio.h>
#include "sfmm.h"
#include "mysfmm.h"


typedef struct test{
    uint64_t      allocated;
    uint64_t         padded;
} Test;

int main(int argc, char const *argv[]) {

    Test test = {0x1234123412341234, 0x5678567856785678};
    // Test *test_ptr = test;

    sf_mem_init();

    void *x = sf_malloc(32);
    ((Test *)x)->allocated = test.allocated;
    ((Test *)x)->padded = test.padded;
    void *y = sf_realloc(x, 48);

    sf_snapshot();
    sf_varprint(y);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
