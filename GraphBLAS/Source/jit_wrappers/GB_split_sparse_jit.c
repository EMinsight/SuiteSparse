//------------------------------------------------------------------------------
// GB_split_sparse_jit: split A into a sparse tile C
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "jitifyer/GB_stringify.h"

typedef GB_JIT_KERNEL_SPLIT_SPARSE_PROTO ((*GB_jit_dl_function)) ;

GrB_Info GB_split_sparse_jit    // split A into a sparse tile C
(
    // input/output
    GrB_Matrix C,
    // input:
    const GB_Operator op,
    const GrB_Matrix A,
    int64_t akstart,
    int64_t aistart,
    const void *Wp,             // 32/64 bit, depending on A->p_is_32
    const int64_t *restrict C_ek_slicing,
    const int C_ntasks,
    const int C_nthreads
)
{ 

    //--------------------------------------------------------------------------
    // encodify the problem
    //--------------------------------------------------------------------------

    GB_jit_encoding encoding ;
    char *suffix ;
    uint64_t hash = GB_encodify_apply (&encoding, &suffix,
        GB_JIT_KERNEL_SPLIT_SPARSE, GxB_SPARSE, true, C->type, C->p_is_32,
        C->i_is_32, false, op, false, GB_sparsity (A), true, A->type,
        A->p_is_32, A->j_is_32, A->i_is_32, A->iso, A->nzombies) ;

    //--------------------------------------------------------------------------
    // get the kernel function pointer, loading or compiling it if needed
    //--------------------------------------------------------------------------

    void *dl_function ;
    GrB_Info info = GB_jitifyer_load (&dl_function,
        GB_jit_apply_family, "split_sparse",
        hash, &encoding, suffix, NULL, NULL,
        op, C->type, A->type, NULL) ;
    if (info != GrB_SUCCESS) return (info) ;

    //--------------------------------------------------------------------------
    // call the jit kernel and return result
    //--------------------------------------------------------------------------

    #include "include/GB_pedantic_disable.h"
    GB_jit_dl_function GB_jit_kernel = (GB_jit_dl_function) dl_function ;
    return (GB_jit_kernel (C, A, akstart, aistart, Wp, C_ek_slicing, C_ntasks,
        C_nthreads, &GB_callback)) ;
}

