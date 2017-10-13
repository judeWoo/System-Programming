#ifndef MYSFMM_H
#define MYSFMM_H
#include "sfmm.h"
#include "debug.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WSIZE 2
#define MEMROW 8

#define MINIMUM 32

void *sf_malloc_helper(size_t size, size_t asize, int list_index);
void *sf_place(size_t size, size_t asize);
void *sf_search(size_t size, size_t asize, int list_index);
void *sf_append(uint64_t block_size, char *bp);
void sf_remove(sf_free_header *tmp, int list_index);
int sf_list_index(uint64_t block_size);
void ascending_sort(int number[], int n);
void sf_invalid(sf_header * hdrp, sf_footer *ftrp);

extern int sf_sbrk_counter; /* For sf_sbrk() count */
extern int sort[8]; /* For sorting */

#endif