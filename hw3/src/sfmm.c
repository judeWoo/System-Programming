/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include "mysfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    char *bp = NULL; /* Block pointer */

    /* Invalid size */
    if (size == 0 || size > PAGE_SZ * FREE_LIST_COUNT)
    {
        sf_errno = EINVAL;
        debug("(ERROR) The requested size is %zu\n", size);
        return NULL;
    }

    /* Set macros in ascending order, just once */
    if (sf_sbrk_counter == 0)
    {

        sort[0] = LIST_1_MIN;
        sort[1] = LIST_1_MAX;
        sort[2] = LIST_2_MIN;
        sort[3] = LIST_2_MAX;
        sort[4] = LIST_3_MIN;
        sort[5] = LIST_3_MAX;
        sort[6] = LIST_4_MIN;
        sort[7] = LIST_4_MAX;
        ascending_sort(sort, 8);
    }

    /* Adjust block size to include overhead and alignment requests */
    if (size <= 16)
    {
        asize = MINIMUM;
    }
    else
    {
        asize = 16 * ((size + (2 * 8) + (16 - 1)) / 16);
    }
    debug("(Start up) Requested size is %zu, Adjusted size is %zu\n", size, asize);

    /* Search the free list for a fit */
    if (asize >= sort[0] || asize <= sort[1])
    {
        if ((bp = (char *) sf_malloc_helper(size, asize, 0)) != (char *) -1)
        {
            debug("(LIST_1_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }
        goto case1;
    }
    else if (asize >= sort[2] || asize <= sort[3])
    {
        case1:
        if ((bp = (char *) sf_malloc_helper(size, asize, 1)) != (char *) -1)
        {
            debug("(LIST_2_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }
        goto case2;
    }
    else if (asize >= sort[4] || asize <= sort[5])
    {
        case2:
        if ((bp = (char *) sf_malloc_helper(size, asize, 2)) != (char *) -1)
        {
            debug("(LIST_3_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }
        goto case3;
    }
    else
    {
        case3:
        if ((bp = (char *) sf_malloc_helper(size, asize, 3)) != (char *) -1)
        {
            debug("(LIST_4_MIN) The block pointer is now at %p\n", bp);
            return (void *) bp;
        }

        /* No fit found. Get more memory and place the block */
        else
        {
            debug("(Request More Memory) (Outer Request %d)\n", sf_sbrk_counter);
            return sf_place(size, asize);
        }
    }
}

void *sf_malloc_helper(size_t size, size_t asize, int list_index){

    /* If list is NULL */
    if (seg_free_list[list_index].head == NULL)
    {
        return (void *) -1;
    }
    else
    {
        /* Find a free block to allocate */
        return sf_search(size, asize, list_index);
    }
}

void *sf_place(size_t size, size_t asize){

    uint64_t free = 0; /* Prev free block size */
    char *bp = NULL; /* Block pointer */
    sf_footer *ftrp = NULL; /* Footer */

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

            /* Coalesce prev block */
            if (((bp - 8) >= (char *) get_heap_start()) && ((ftrp = (sf_footer *) (bp - 8))->allocated == 0))
            {
                debug("(Coalesce) (Outer Request %d) bp is at %p\n", sf_sbrk_counter, bp);
                /* Prev free block size */
                free = ((ftrp->block_size) << 4);
                debug("(Outer Request %d) free is %zu\n", sf_sbrk_counter, free);
                debug("(Outer Request %d) The prev free block size is %d\n", sf_sbrk_counter, (ftrp->block_size) * 16);
                debug("(Outer Request %d) The prev free block header is at %p\n", sf_sbrk_counter, bp - (ftrp->block_size) * 16);
                debug("(Outer Request %d) The prev free block footer is at %p\n", sf_sbrk_counter, bp - 8);
                /* Remove from free list */
                sf_remove((sf_free_header *) (bp - ((ftrp->block_size) << 4)), sf_list_index((ftrp->block_size) << 4));

                /* Update Block pointer */
                bp = bp - ((ftrp->block_size) << 4);
            }
            debug("(Outer Request %d) The free block header is at %p\n", sf_sbrk_counter, bp);
            /* Append */
            bp = sf_append(PAGE_SZ + free, bp);

            /* Check size */
            if(asize <= (PAGE_SZ + free))
            {
                break;
            }
        }
    }
    /* Ask Too Many */
    if (asize > (PAGE_SZ + free))
    {
        sf_errno = ENOMEM;
        return NULL;
    }
    /* No splinter */
    if ((PAGE_SZ + free) - asize < MINIMUM)
    {
        asize = (PAGE_SZ + free);
    }

    /* Search & return */
    return sf_search(size, asize, sf_list_index(PAGE_SZ + free));
}

void *sf_search(size_t size, size_t asize, int list_index){

    sf_free_header *tmp = NULL; /* Searcher */
    char *bp = NULL; /* Block pointer */
    sf_footer *ftrp = NULL; /* Footer */

    /* First-fit search */
    debug("(Search) The free block list is at %d\n", list_index);
    for (tmp = seg_free_list[list_index].head; tmp != NULL; tmp = tmp->next)
    {
        if ((tmp->header.allocated == 0) && ((asize / 16) <= tmp->header.block_size))
        {
            /* No splinters */
            if (((tmp->header.block_size << 4) - asize) < MINIMUM)
            {
                asize = (tmp->header.block_size << 4);
            }

            /* Put Header to selected free block */
            tmp->header.allocated = 1;
            tmp->header.padded = 0;
            tmp->header.two_zeroes = 0;
            tmp->header.unused = 0;

            /* Put Footer to selected free block */
            ftrp = (sf_footer *) ((char *) tmp + asize - 8);
            (*ftrp).allocated = 1;
            (*ftrp).padded = 0;
            (*ftrp).two_zeroes = 0;
            (*ftrp).requested_size = size;
            debug("(Found) Allocated block header is now at %p\n", tmp);
            debug("(Found) Allocated block footer is now at %p\n", ftrp);
            debug("(Found) End of heap is at %p\n", get_heap_end());

            /* Place remainder */
            if (((tmp->header.block_size << 4) - asize) != 0)
            {
                sf_append(((tmp->header.block_size) << 4) - asize, ((char *) tmp + asize));
            }

            /* Update selected block size */
            tmp->header.block_size = asize / 16;
            (*ftrp).block_size = asize / 16;

            /* Check padding of selected block */
            if ((((*ftrp).requested_size) + 16) != (tmp->header.block_size << 4))
            {
                tmp->header.padded = 1;
                (*ftrp).padded = 1;
            }

            /* Update Block Pointer */
            bp = (char *) tmp;

            /* Remove selected from the list */
            sf_remove(tmp, list_index);

            /* Return the address of payload */
            return (void *) ((char *) bp + 8);
        }
    }
    /* No Fit */
    return (void *) -1;
}

void *sf_append(uint64_t block_size, char *bp) {

    int s; /* Free list chooser */
    sf_free_header *tmp; /* Free list indexer */
    sf_footer *ftrp; /* Footer */

    /* Determine the list */
    s = sf_list_index(block_size);
    debug("(Append) The free block list is at %d\n", s);

    /* Place free block */
    if (seg_free_list[s].head == NULL)
    {
        seg_free_list[s].head = (sf_free_header *) bp;
        seg_free_list[s].head->header.allocated = 0;
        seg_free_list[s].head->header.padded = 0;
        seg_free_list[s].head->header.two_zeroes = 0;
        seg_free_list[s].head->header.block_size = block_size / 16;
        seg_free_list[s].head->header.unused = 0;
        seg_free_list[s].head->prev = NULL;
        seg_free_list[s].head->next = NULL;
        ftrp = (sf_footer *) (bp + block_size - 8);
        (*ftrp).allocated = 0;
        (*ftrp).padded = 0;
        (*ftrp).two_zeroes = 0;
        (*ftrp).block_size = block_size / 16;
        (*ftrp).requested_size = 0;
        /* Return the address of payload */
        debug("(Place list %d) The free block header is %p\n", s + 1, seg_free_list[s].head);
        debug("(Place list %d) The free block footer is %p\n", s + 1, ftrp);
        debug("(Place list %d) End of heap is at %p\n", s + 1, get_heap_end());
        debug("(Place list %d) Block size is %zu\n", s + 1, block_size);
        return (void *) (bp + 8);
    }
    else
    {
        /* Append free block in the very front */
        tmp = seg_free_list[s].head;
        seg_free_list[s].head->prev = (sf_free_header *) bp;
        seg_free_list[s].head = (sf_free_header *) bp;
        seg_free_list[s].head->next = tmp;
        seg_free_list[s].head->prev = NULL;
        seg_free_list[s].head->header.allocated = 0;
        seg_free_list[s].head->header.padded = 0;
        seg_free_list[s].head->header.two_zeroes = 0;
        seg_free_list[s].head->header.block_size = block_size / 16;
        seg_free_list[s].head->header.unused = 0;
        ftrp = (sf_footer *) (bp + block_size - 8);
        (*ftrp).allocated = 0;
        (*ftrp).padded = 0;
        (*ftrp).two_zeroes = 0;
        (*ftrp).block_size = block_size / 16;
        (*ftrp).requested_size = 0;
        /* Return the address of payload */
        debug("(Append list %d) The free block header is %p\n", s + 1, seg_free_list[s].head);
        debug("(Append list %d) The free block footer is %p\n", s + 1, ftrp);
        debug("(Append list %d) End of heap is at %p\n", s + 1, get_heap_end());
        return (void *) (bp + 8);
    }
}

void sf_remove(sf_free_header *tmp, int list_index) {

    if ((tmp->next != NULL) && (tmp->prev != NULL))
    {
        tmp->next->prev = tmp->prev;
        tmp->prev->next = tmp->next;
    }
    else if (((tmp->prev) == NULL) && ((tmp->next) == NULL))
    {
        seg_free_list[list_index].head = NULL;
    }
    else if (((tmp->prev) == NULL) && ((tmp->next) != NULL))
    {
        seg_free_list[list_index].head->next->prev = NULL;
        seg_free_list[list_index].head = seg_free_list[list_index].head->next;
    }
    else
    {
        tmp->prev->next = NULL;
        tmp->prev = NULL;
    }

    return;
}

int sf_list_index(uint64_t block_size) {

    int sort[4]; /* Sorter */

    /* Ascending sort */
    sort[0] = seg_free_list[3].min;
    sort[1] = seg_free_list[2].min;
    sort[2] = seg_free_list[1].min;
    sort[3] = seg_free_list[0].min;
    ascending_sort(sort, 4);

    if (block_size >= sort[3])
    {
        return 3;
    }
    if ((block_size < sort[3]) && (block_size >= sort[2]))
    {
        return 2;
    }
    if ((block_size < sort[2]) && (block_size >= sort[1]))
    {
        return 1;
    }
    if ((block_size < sort[1]) && (block_size >= sort[0]))
    {
        return 0;
    }

    return -1;
}

void ascending_sort(int number[], int n) {

    int a;

    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            if (number[i] > number[j])
            {
                a =  number[i];
                number[i] = number[j];
                number[j] = a;
            }
        }
    }

    if (number[0] == -1)
    {
        a = number[0];

        for (int i = 0; i < 7; ++i)
        {
            number[i] = number[i+1];
        }

        number[7] = a;

    }

    return;
}

void *sf_realloc(void *ptr, size_t size) {

    uint64_t selected = 0; /* Ptr's block size */
    size_t asize = 0; /* Adjusted block size */
    sf_header *hdrp = NULL; /* Header */
    sf_footer *ftrp = NULL; /* Footer */
    char *bp = NULL; /* Block pointer */

    /* Check null & address of ptr */
    if ((ptr == NULL) || (ptr < (void *) 16))
    {
        abort();
    }

    /* Init */
    hdrp = (sf_header *) ((char *) ptr - 8);
    ftrp = (sf_footer *) ((char *) ptr + (hdrp->block_size << 4) - 16);

    /* Invalid size */
    if (size > PAGE_SZ * FREE_LIST_COUNT)
    {
        sf_errno = EINVAL;
        debug("(ERROR) The requested size is %zu\n", size);
        return NULL;
    }

    /* Check invalid */
    sf_invalid(hdrp, ftrp);

    /* Free */
    if (size == 0)
    {
        sf_append((hdrp->block_size << 4), (char *) hdrp);
        return NULL;
    }

    /* Realloc to larger size */
    if (size > (hdrp->block_size << 4))
    {
        if ((bp = sf_malloc(size)) != NULL)
        {
            memcpy(bp, ptr, ((hdrp->block_size << 4) - 16)); /* GDB */
            sf_free(ptr);
        }

        return (void *) bp;
    }

    /* Realloc to smaller size */
    else if (size < (hdrp->block_size << 4))
    {
        /* No split */
        if (((ftrp->requested_size) - size) < MINIMUM)
        {
            /* No splinter */

            /* Change payload */
            ftrp->requested_size = size;

            /* Change padding */
            hdrp->padded = 1;
            ftrp->padded = 1;

            return (void *) ptr;
        }
        /* Split */
        else
        {
            /* Save ptr's block size */
            selected = hdrp->block_size << 4;

            /* Adjust block size to include overhead and alignment requests */
            asize = 16 * ((size + (2 * 8) + (16 - 1)) / 16);

            /* Update header & footer */
            ftrp = (sf_footer *) ((char *) hdrp + asize - 8);
            ftrp->allocated = hdrp->allocated;
            ftrp->two_zeroes = hdrp->two_zeroes;

            /* Change block size */
            hdrp->block_size = asize / 16;
            ftrp->block_size = asize / 16;

            /* Change payload */
            ftrp->requested_size = size;

            /* Change padding */
            if ((ftrp->requested_size + 16) != (hdrp->block_size << 4))
            {
                hdrp->padded = 1;
                ftrp->padded = 1;
            }
            else
            {
                hdrp->padded = 0;
                ftrp->padded = 0;
            }

            /* Update block pointer for remainder */
            bp = ((char *)ftrp + 8);

            /* Free */
            sf_append(selected - (hdrp->block_size << 4), bp);

            return (void *) ptr;
        }
    }

    /* Realloc to same size */
    else
    {
        /* When request diff */
        if (size != (ftrp->requested_size))
        {
            ftrp->requested_size = size;
        }

        return (void *) ptr;
    }

    return NULL;
}

void sf_free(void *ptr) {

    uint64_t selected = 0; /* ptr's block size */
    sf_header *hdrp = NULL; /* Header */
    sf_footer *ftrp = NULL; /* Footer */

    /* Check null & address of ptr */
    if ((ptr == NULL) || (ptr < (void *) 16))
    {
        abort();
    }

    /* Init */
    hdrp = (sf_header *) ((char *) ptr - 8);
    ftrp = (sf_footer *) ((char *) ptr + (hdrp->block_size << 4) - 16);
    selected = hdrp->block_size << 4;

    /* Check invalid */
    sf_invalid(hdrp, ftrp);

    /* Coalesce next block + update header to next block */
    if ((hdrp = (sf_header *) (ptr - 8 + (hdrp->block_size << 4)))->allocated == 0)
    {
        /* Remove from list */
        sf_remove((sf_free_header *) hdrp, sf_list_index(hdrp->block_size << 4));

        /* Append */
        sf_append((hdrp->block_size << 4) + selected, (char *) hdrp);
        goto end;
    }

    /* Revert header */
    hdrp = (sf_header *) ((char *) ptr - 8);

    /* Append */
    sf_append(selected, (char *) hdrp);

    end:
    return;
}

void sf_invalid(sf_header *hdrp, sf_footer *ftrp) {

    if (hdrp < (sf_header *) get_heap_start())
    {
        abort(); /* Header before heap */
    }
    if (ftrp > (sf_footer *) get_heap_end())
    {
        abort(); /* Footer after heap */
    }
    if ((hdrp->allocated == 0) || (ftrp->allocated == 0))
    {
        abort(); /* Not free */
    }
    if (hdrp->allocated != ftrp->allocated)
    {
        abort(); /* Different allocating */
    }
    if (hdrp->padded != ftrp->padded)
    {
        abort(); /* Different padding */
    }
    if (hdrp->block_size != ftrp->block_size)
    {
        abort(); /* Different block size */
    }
    if ((hdrp->block_size << 4 < MINIMUM) || (hdrp->block_size << 4 > PAGE_SZ * 4) )
    {
        abort(); /* Too small or Too big block size */
    }
    if (hdrp->two_zeroes != ftrp->two_zeroes)
    {
        abort(); /* Different two zeroes */
    }
    if (hdrp->two_zeroes != 0)
    {
        abort(); /* Two zeroes not zeroes */
    }
    if (hdrp->unused != 0)
    {
        abort(); /* Unused bits not zero */
    }
    if ((ftrp->requested_size <= 0) || (ftrp->requested_size > PAGE_SZ * 4 - 16))
    {
        abort();
    }
    if (hdrp->padded == 1)
    {
        if ((ftrp->requested_size + 16) == (hdrp->block_size << 4))
        {
            abort(); /* Does not make sense */
        }
    }
    if (hdrp->padded == 0)
    {
        if ((ftrp->requested_size + 16) != (hdrp->block_size << 4))
        {
            abort(); /* Does not make sense */
        }
    }

    return;
}