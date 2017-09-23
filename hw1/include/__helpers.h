#ifndef __HELPERS_H
#define __HELPERS_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

char *binary_string(void* number, size_t size);

// returns the contents of a file in a buf of size 10000
char *contents_of(const char *file_name);
#endif /* __HELPERS_H */
