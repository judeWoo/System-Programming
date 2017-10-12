#ifndef MYSFMM_H
#define MYSFMM_H
#include "sfmm.h"

#define WSIZE 2
#define MEMROW 8

#define SF_HEADER_SIZE_BYTE 8

void *sf_malloc_helper(size_t size, size_t asize, char *bp, sf_header *hdrp, sf_footer *ftrp, int f);
void *sf_place(size_t size, size_t asize, char *bp, sf_header *hdrp, sf_footer *ftrp, int s);
void *sf_search(size_t size, size_t asize, sf_footer *ftrp, int f);
void *sf_remainder(size_t size, size_t asize, int block_size, char *bp, sf_footer *ftrp, int s);
void ascending_sort(int number[]);

extern int sf_sbrk_counter;
extern int sort[8]; /* For sorting */

#endif