//------------------------------------------------------------------------------
// UMFPACK/Source/umf_free: free a block of memory
//------------------------------------------------------------------------------

// UMFPACK, Copyright (c) 2005-2023, Timothy A. Davis, All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

/*
    Free a block previously allocated by UMF_malloc and return NULL.
    Usage is p = UMF_free (p), to ensure that we don't free it twice.
    Also maintains the UMFPACK malloc count.
*/

#include "umf_internal.h"
#include "umf_free.h"

#if defined (UMF_MALLOC_COUNT) || !defined (NDEBUG)
#include "umf_malloc.h"
#endif

void *UMF_free
(
    void *p
)
{
    DEBUG0 (("UMF_free: %p\n", p)) ;
    if (p)
    {

	SuiteSparse_free (p) ;

#if defined (UMF_MALLOC_COUNT) || !defined (NDEBUG)
	/* One more object has been free'd.  Keep track of the count. */
	/* (purely for sanity checks). */
	UMF_malloc_count-- ;
	DEBUG0 (("     new malloc count: "ID"\n", UMF_malloc_count)) ;
#endif

    }

    return ((void *) NULL) ;
}
