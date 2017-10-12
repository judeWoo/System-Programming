/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include "mysfmm.h"
#include "debug.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;
int sf_sbrk_counter = 0;
int sort[8];

void *sf_malloc(size_t size) {

    size_t asize = 0; /* Adjusted block size */

    char *bp = NULL;
    sf_header *hdrp = NULL;
    sf_footer *ftrp = NULL;

    /* Invalid size */
    if (size == 0 || size > PAGE_SZ * FREE_LIST_COUNT)
    {
        sf_errno = EINVAL;
        debug("(ERROR) The size is %zu", size);
        return NULL;
    }

    /* Set macros in ascending order, just once */ /* TODO */
    if (sf_sbrk_counter == 0)
    {
        ascending_sort(sort);
    }

    /* Adjust block size to include overhead and alignment requests */
    if (size <= 16)
    {
        asize = sort[0];
    }
    else
    {
        asize = 16 * ((size + (2 * 8) + (16 - 1)) / 16);
    }
    debug("(Start up) Requested size is %zu, Adjusted size is %zu\n", size, asize);

    /* Search the free list for a fit */
    if (asize >= sort[0] || asize <= sort[1])
    {
        if ((bp = (char *) sf_malloc_helper(size, asize, bp, hdrp, ftrp, 0)) != (char *) -1)
        {
            debug("(LIST_1_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }
        goto case1;
    }
    else if (asize >= sort[2] || asize <= sort[3])
    {
        case1:
        if ((bp = (char *) sf_malloc_helper(size, asize, bp, hdrp, ftrp, 1)) != (char *) -1)
        {
            debug("(LIST_2_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }
        goto case2;
    }
    else if (asize >= sort[4] || asize <= sort[5])
    {
        case2:
        if ((bp = (char *) sf_malloc_helper(size, asize, bp, hdrp, ftrp, 2)) != (char *) -1)
        {
            debug("(LIST_3_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }
        goto case3;
    }
    else
    {
        case3:
        if ((bp = (char *) sf_malloc_helper(size, asize, bp, hdrp, ftrp, 3)) != (char *) -1)
        {
            debug("(LIST_4_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }

        /* No fit found. Get more memory and place the block */
        else
        {
            debug("(Request More Memory) (Outer Request %d)\n", sf_sbrk_counter);
            return sf_place(size, asize, bp, hdrp, ftrp, 3);
        }
    }
}

void *sf_malloc_helper(size_t size, size_t asize, char *bp, sf_header *hdrp, sf_footer *ftrp, int f){

    /* If list is NULL */
    if (seg_free_list[f].head == NULL)
    {
        return (void *) -1;
    }
    else
    {
        /* Find a free block to allocate */
        return sf_search(size, asize, ftrp, f);
    }
}

void *sf_place(size_t size, size_t asize, char *bp, sf_header *hdrp, sf_footer *ftrp, int s){

    size_t hsize = sf_sbrk_counter * PAGE_SZ; /* heap size */

    /* Check if sf_sbrk() called more than 4 times*/
    if (sf_sbrk_counter > 5)
    {
        sf_errno = ENOMEM;
        return NULL;
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            /* Request memory & check invalid */
            if((bp = (char *) sf_sbrk()) == (char *) -1)
            {
                sf_errno = ENOMEM;
                debug("(ERROR) (Internal Request %d) (Outer Request %d)\n", i, sf_sbrk_counter);
                return NULL;
            }

            /* Set counter */
            sf_sbrk_counter++;
            debug("(Outer Request %d) End of heap is at %p\n", sf_sbrk_counter, get_heap_end());

            /* Update heap size */
            hsize = hsize + PAGE_SZ;

            /* Coalesce */
            if (((bp - 8) >= (char *) get_heap_start()) && ((ftrp = (sf_footer *) (bp - 8))->allocated == 0x0))
            {
                bp = (bp - (16 * (ftrp->block_size)));
                hsize = hsize + (16 * (ftrp->block_size));
            }
            debug("(Outer Request %d) The block pointer is at %p\n", sf_sbrk_counter, bp);

            /* Check size */
            if(asize <= hsize)
            {
                break;
            }
        }
    }

    /* Ask Too Many */
    if (asize > hsize)
    {
        sf_errno = ENOMEM;
        return NULL;
    }

    /* If remainder too small */
    if ((hsize - asize) < sort[0])
    {
        asize = hsize;
    }

    /* Locate Free Block */
    if (sf_sbrk_counter > 1)
    {
        bp = ((char *) get_heap_start() + ((sf_sbrk_counter - 1) * PAGE_SZ));
    }

    /* Put Header to an allocated block */
    hdrp = (sf_header *) bp;
    (*hdrp).allocated = 1;
    (*hdrp).padded = 0;
    (*hdrp).two_zeroes = 0;
    (*hdrp).block_size = asize / 16;
    debug("(Outer Request %d) The block's head is now at %p\n", sf_sbrk_counter, hdrp);

    /* Put Footer to an allocated block */
    ftrp = (sf_footer *) (bp + asize - 8);
    (*ftrp).allocated = 1;
    (*ftrp).padded = 0;
    (*ftrp).two_zeroes = 0;
    (*ftrp).block_size = asize / 16;
    (*ftrp).requested_size = size;
    debug("(Outer Request %d) The block's foot is now at %p\n", sf_sbrk_counter, ftrp);

    /* Check Padding of an allocated block */
    if ((((*ftrp).requested_size) + 16) != ((*hdrp).block_size * 16))
    {
        (*hdrp).padded = 1;
        (*ftrp).padded = 1;
    }

    /* If no remainder */
    if (((hsize - asize) == 0))
    {
        debug("(No remainder) The block pointer is now at %p\n", bp);
        return (void *) (bp + 8);
    }

    /* If remainder */
    return sf_remainder(size, asize, PAGE_SZ, bp, ftrp, 3);
}

void *sf_search(size_t size, size_t asize, sf_footer *ftrp, int f){

    sf_free_header *tmp = NULL; /* Searcher */
    char *bp = NULL;

    /* First-fit search */
    for (tmp = seg_free_list[f].head; tmp != NULL; tmp = tmp->next)
    {
        if ((tmp->header.allocated == 0) && ((asize / 16) <= tmp->header.block_size))
        {
            /* No splinters */
            if (((16 * tmp->header.block_size) - asize) < sort[0])
            {
                asize = (16 * tmp->header.block_size);
            }

            /* Put Header to a selected free block */
            tmp->header.allocated = 1;
            tmp->header.padded = 0;
            tmp->header.two_zeroes = 0;

            /* Put Footer to a selected free block */
            ftrp = (sf_footer *) ((char *) tmp + asize - 8);
            (*ftrp).allocated = 1;
            (*ftrp).padded = 0;
            (*ftrp).two_zeroes = 0;
            (*ftrp).requested_size = size;
            debug("(Found) Allocated block header is now at %p\n", tmp);
            debug("(Found) Allocated block footer is now at %p\n", ftrp);
            debug("(Found) End of heap is at %p\n", get_heap_end());

            /* Place Remainder */
            if (((tmp->header.block_size * 16) - asize) != 0)
            {
                sf_remainder(size, asize, (size_t) ((tmp->header.block_size) * 16), ((char *) tmp), ftrp, 3);
            }

            /* Update block size */
            tmp->header.block_size = asize / 16;
            (*ftrp).block_size = asize / 16;
            /* Check Padding of an allocated block */
            if ((((*ftrp).requested_size) + 16) != (tmp->header.block_size * 16))
            {
                tmp->header.padded = 1;
                (*ftrp).padded = 1;
            }

            /* Update Block Pointer */
            bp = (char *) tmp;

            /* Remove from the list */
            if ((tmp->next != NULL) && (tmp->prev != NULL))
            {
                tmp->next->prev = tmp->prev;
                tmp->prev->next = tmp->next;
            }
            else if (((tmp->prev) == NULL) && ((tmp->next) == NULL))
            {
                seg_free_list[f].head = NULL;
            }
            else if (((tmp->prev) == NULL) && ((tmp->next) != NULL))
            {
                seg_free_list[f].head->next->prev = NULL;
                seg_free_list[f].head = seg_free_list[f].head->next;
            }
            else
            {
                tmp->prev->next = NULL;
                tmp->prev = NULL;
            }
            /* Return the address of payload */
            return (void *) ((char *) bp + 8);
        }
    }
    /* No Fit */
    return (void *) -1;
}

void *sf_remainder(size_t size, size_t asize, int block_size, char *bp, sf_footer *ftrp, int s) {

    sf_free_header *tmp; /* Indexer */

    /* Determine the right size for remainder */
    if (((block_size - asize) >= seg_free_list[s].min))
    {
        s = s;
    }
    if (((block_size - asize) < seg_free_list[s].min) && ((block_size - asize) >= seg_free_list[s - 1].min))
    {
        s = s - 1;
    }
    if (((block_size - asize) < seg_free_list[s - 1].min) && ((block_size - asize) >= seg_free_list[s - 2].min))
    {
        s = s - 2;
    }
    if (((block_size - asize) < seg_free_list[s - 2].min) && ((block_size - asize) >= seg_free_list[s - 3].min))
    {
        s = s - 3;
    }

    /* Place remainder */
    if (seg_free_list[s].head == NULL)
    {
        seg_free_list[s].head = (sf_free_header *) (bp + asize);
        seg_free_list[s].head->header.allocated = 0;
        seg_free_list[s].head->header.padded = 0;
        seg_free_list[s].head->header.two_zeroes = 0;
        seg_free_list[s].head->header.block_size = (block_size - asize) / 16;
        seg_free_list[s].head->prev = NULL;
        seg_free_list[s].head->next = NULL;
        ftrp = (sf_footer *) (bp + block_size - 8);
        (*ftrp).allocated = 0;
        (*ftrp).padded = 0;
        (*ftrp).two_zeroes = 0;
        (*ftrp).block_size = (block_size - asize) / 16;
        (*ftrp).requested_size = 0;
        /* Return the address of payload */
        debug("(Place list %d) The second free block header is %p\n", s + 1, seg_free_list[s].head);
        debug("(Place list %d) The second free block footer is %p\n", s + 1, ftrp);
        debug("(Place list %d) End of heap is at %p\n", s + 1, get_heap_end());
        return (void *) (bp + 8);
    }
    else
    {
        /* Append the remainder in the very front */
        tmp = seg_free_list[s].head;
        seg_free_list[s].head->prev = (sf_free_header *) (bp + asize);
        seg_free_list[s].head = (sf_free_header *) (bp + asize);
        seg_free_list[s].head->next = tmp;
        seg_free_list[s].head->prev = NULL;
        seg_free_list[s].head->header.allocated = 0;
        seg_free_list[s].head->header.padded = 0;
        seg_free_list[s].head->header.two_zeroes = 0;
        seg_free_list[s].head->header.block_size = (block_size - asize) / 16;
        ftrp = (sf_footer *) (bp + block_size - 8);
        (*ftrp).allocated = 0;
        (*ftrp).padded = 0;
        (*ftrp).two_zeroes = 0;
        (*ftrp).block_size = (block_size - asize) / 16;
        (*ftrp).requested_size = 0;
        /* Return the address of payload */
        debug("(Append list %d) The first free block header is %p\n", s + 1, seg_free_list[s].head);
        debug("(Append list %d) The first free block footer is %p\n", s + 1, ftrp);
        debug("(Append list %d) End of heap is at %p\n", s + 1, get_heap_end());
        return (void *) (bp + 8);
    }
}

void ascending_sort(int number[]) {

    int a;

    number[0] = LIST_1_MIN;
    number[1] = LIST_1_MAX;
    number[2] = LIST_2_MIN;
    number[3] = LIST_2_MAX;
    number[4] = LIST_3_MIN;
    number[5] = LIST_3_MAX;
    number[6] = LIST_4_MIN;
    number[7] = LIST_4_MAX;

    for (int i = 0; i < 8; ++i)
    {
        for (int j = i + 1; j < 8; ++j)
        {
            if (number[i] > number[j])
            {
                a =  number[i];
                number[i] = number[j];
                number[j] = a;
            }
        }
    }

    a = number[0];

    for (int i = 0; i < 7; ++i)
    {
        number[i] = number[i+1];
    }

    number[7] = a;
    return;
}

void *sf_realloc(void *ptr, size_t size) {
    return NULL;
}

void sf_free(void *ptr) {
    return;
}