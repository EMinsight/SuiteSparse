//------------------------------------------------------------------------------
// LAGraph_Vector_Print:  pretty-print a vector
//------------------------------------------------------------------------------

// LAGraph, (c) 2019-2022 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
//
// For additional details (including references to third party source code and
// other files) see the LICENSE file or contact permission@sei.cmu.edu. See
// Contributors.txt for a full list of contributors. Created, in part, with
// funding and support from the U.S. Government (see Acknowledgments.txt file).
// DM22-0790

// Contributed by Timothy A. Davis, Texas A&M University

//------------------------------------------------------------------------------

// LAGraph_Vector_Print:  pretty-print a vector.  The type is either derived
// from GxB_Vector_type (if available) or assumed to be GrB_FP64 otherwise,
// or passed in as a parameter.

#include "LG_internal.h"

#undef  LG_FREE_WORK
#define LG_FREE_WORK                    \
{                                       \
    LAGraph_Free ((void **) &I, NULL) ; \
    LAGraph_Free ((void **) &X, NULL) ; \
}

#undef  LG_FREE_ALL
#define LG_FREE_ALL LG_FREE_WORK

//------------------------------------------------------------------------------
// LG_Vector_Print_TYPE: print with the specified type
//------------------------------------------------------------------------------

#define LG_VECTOR_PRINT(suffix,ctype,gtype,fmt1,fmt2)                       \
int LG_Vector_Print_ ## suffix                                              \
(                                                                           \
    GrB_Vector v, LAGraph_PrintLevel pr, FILE *f, char *msg                 \
)                                                                           \
{                                                                           \
    LG_CLEAR_MSG ;                                                          \
    ctype *X = NULL ;                                                       \
    GrB_Index *I = NULL ;                                                   \
    LG_ASSERT (v != NULL, GrB_NULL_POINTER) ;                               \
    LG_ASSERT (f != NULL, GrB_NULL_POINTER) ;                               \
    int prl = (int) pr ;                                                    \
    if (prl <= 0) return (GrB_SUCCESS) ;                                    \
    /* get basic properties */                                              \
    GrB_Index n, nvals ;                                                    \
    GRB_TRY (GrB_Vector_size  (&n, v)) ;                                    \
    GRB_TRY (GrB_Vector_nvals (&nvals, v)) ;                                \
    /* print header line */                                                 \
    FPRINTF (f, "%s vector: n: %" PRIu64 " entries: %" PRIu64               \
        "\n", LG_XSTR (gtype), n, nvals) ;                                  \
    if (prl <= 1) return (GrB_SUCCESS) ;                                    \
    /* extract tuples */                                                    \
    LG_TRY (LAGraph_Malloc ((void **) &I, nvals, sizeof (GrB_Index), msg)) ;\
    LG_TRY (LAGraph_Malloc ((void **) &X, nvals, sizeof (ctype), msg)) ;    \
    GrB_Info info = GrB_Vector_extractTuples (I, X, &nvals, v) ;            \
    LG_ASSERT_MSG (info != GrB_DOMAIN_MISMATCH,                             \
        GrB_NOT_IMPLEMENTED, "type not supported") ;                        \
    GRB_TRY (info) ;                                                        \
    /* determine the format */                                              \
    char *format = (prl <= 3) ? fmt1 : fmt2 ;                               \
    bool summary = (prl == 2 || prl == 4) && (nvals > LG_SHORT_LEN) ;       \
    for (int64_t k = 0 ; k < nvals ; k++)                                   \
    {                                                                       \
        /* print the kth tuple */                                           \
        GrB_Index i = I [k] ;                                               \
        ctype     x = X [k] ;                                               \
        FPRINTF (f, "    (%" PRIu64 ")   ", i) ;                            \
        FPRINTF (f, format, x) ;                                            \
        FPRINTF (f, "\n") ;                                                 \
        if (summary && k > LG_SHORT_LEN)                                    \
        {                                                                   \
            /* quit early if a only a summary is requested */               \
            FPRINTF (f, "    ...\n") ;                                      \
            break ;                                                         \
        }                                                                   \
    }                                                                       \
    LG_FREE_WORK ;                                                          \
    return (GrB_SUCCESS) ;                                                  \
}

LG_VECTOR_PRINT (BOOL  , bool    , GrB_BOOL  , "%d"  , "%d"    )
LG_VECTOR_PRINT (INT8  , int8_t  , GrB_INT8  , "%d"  , "%d"    )
LG_VECTOR_PRINT (INT16 , int16_t , GrB_INT16 , "%d"  , "%d"    )
LG_VECTOR_PRINT (INT32 , int32_t , GrB_INT32 , "%" PRId32, "%" PRId32  )
LG_VECTOR_PRINT (INT64 , int64_t , GrB_INT64 , "%" PRId64, "%" PRId64  )
LG_VECTOR_PRINT (UINT8 , uint8_t , GrB_UINT8 , "%d"  , "%d"    )
LG_VECTOR_PRINT (UINT16, uint16_t, GrB_UINT16, "%d"  , "%d"    )
LG_VECTOR_PRINT (UINT32, uint32_t, GrB_UINT32, "%" PRIu32, "%" PRIu32  )
LG_VECTOR_PRINT (UINT64, uint64_t, GrB_UINT64, "%" PRIu64, "%" PRIu64  )
LG_VECTOR_PRINT (FP32  , float   , GrB_FP32  , "%g"  , "%0.7g" )
LG_VECTOR_PRINT (FP64  , double  , GrB_FP64  , "%g"  , "%0.15g")
#if 0
// would need to pass in an iscomplex flag to print creal(x) and cimag(x)
LG_VECTOR_PRINT (FC32  , GxB_FC32_t, GxB_FC32, ...)
LG_VECTOR_PRINT (FC64  , GxB_FC64_t, GxB_FC64, ...)
#endif

#undef  LG_FREE_WORK
#define LG_FREE_WORK ;
#undef  LG_FREE_ALL
#define LG_FREE_ALL ;

//------------------------------------------------------------------------------
// LAGraph_Vector_Print: automatically determine the type
//------------------------------------------------------------------------------

int LAGraph_Vector_Print
(
    // input:
    const GrB_Vector v,     // vector to pretty-print to the file
    LAGraph_PrintLevel pr,  // print level (0 to 5)
    FILE *f,            // file to write it to, must be already open; use
                        // stdout or stderr to print to those locations.
    char *msg
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    LG_CLEAR_MSG ;
    LG_ASSERT (v != NULL, GrB_NULL_POINTER) ;
    LG_ASSERT (f != NULL, GrB_NULL_POINTER) ;

    //--------------------------------------------------------------------------
    // determine the type
    //--------------------------------------------------------------------------

    GrB_Type type ;
    char typename [LAGRAPH_MAX_NAME_LEN] ;
    LG_TRY (LAGraph_Vector_TypeName (typename, v, msg)) ;
    LG_TRY (LAGraph_TypeFromName (&type, typename, msg)) ;

    //--------------------------------------------------------------------------
    // print the vector
    //--------------------------------------------------------------------------

    if (type == GrB_BOOL)
    {
        return (LG_Vector_Print_BOOL (v, pr, f, msg)) ;
    }
    else if (type == GrB_INT8)
    {
        return (LG_Vector_Print_INT8 (v, pr, f, msg)) ;
    }
    else if (type == GrB_INT16)
    {
        return (LG_Vector_Print_INT16 (v, pr, f, msg)) ;
    }
    else if (type == GrB_INT32)
    {
        return (LG_Vector_Print_INT32 (v, pr, f, msg)) ;
    }
    else if (type == GrB_INT64)
    {
        return (LG_Vector_Print_INT64 (v, pr, f, msg)) ;
    }
    else if (type == GrB_UINT8)
    {
        return (LG_Vector_Print_UINT8 (v, pr, f, msg)) ;
    }
    else if (type == GrB_UINT16)
    {
        return (LG_Vector_Print_UINT16 (v, pr, f, msg)) ;
    }
    else if (type == GrB_UINT32)
    {
        return (LG_Vector_Print_UINT32 (v, pr, f, msg)) ;
    }
    else if (type == GrB_UINT64)
    {
        return (LG_Vector_Print_UINT64 (v, pr, f, msg)) ;
    }
    else if (type == GrB_FP32)
    {
        return (LG_Vector_Print_FP32 (v, pr, f, msg)) ;
    }
    else if (type == GrB_FP64)
    {
        return (LG_Vector_Print_FP64 (v, pr, f, msg)) ;
    }
    #if 0
    else if (type == GxB_FC32)
    {
        return (LG_Vector_Print_FC32 (v, pr, f, msg)) ;
    }
    else if (type == GxB_FC32)
    {
        return (LG_Vector_Print_FC64 (v, pr, f, msg)) ;
    }
    #endif
    else
    {
        LG_ASSERT_MSG (false,
            GrB_NOT_IMPLEMENTED, "user-defined types not supported") ;
        return (GrB_SUCCESS) ;
    }
}
