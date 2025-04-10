//------------------------------------------------------------------------------
// GB_AxB_dot_generic: generic template for all dot-product methods
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This template serves the dot2 and dot3 methods, but not dot4, since dot4 is
// not implemented for generic kernels.  The #including file defines
// GB_DOT2_GENERIC or GB_DOT3_GENERIC.

// This file does not use GB_DECLARE_TERMINAL_CONST (zterminal).  Instead, it
// defines zterminal itself.

#include "mxm/include/GB_mxm_shared_definitions.h"
#include "generic/GB_generic.h"

{

    //--------------------------------------------------------------------------
    // get operators, functions, workspace, contents of A, B, C
    //--------------------------------------------------------------------------

    ASSERT (!C->iso) ;

    GxB_binary_function fmult = mult->binop_function ;    // NULL if positional
    GxB_index_binary_function fmult_idx = mult->idxbinop_function ;
    GxB_binary_function fadd  = add->op->binop_function ;
    GB_Opcode opcode = mult->opcode ;
//  bool op_is_builtin_positional =
//      GB_IS_BUILTIN_BINOP_CODE_POSITIONAL (opcode) ;

    ASSERT (C->type == add->op->ztype) ;
    size_t csize = C->type->size ;
    size_t asize = A_is_pattern ? 0 : A->type->size ;
    size_t bsize = B_is_pattern ? 0 : B->type->size ;

    size_t zsize = csize ;      // C->type always matches add->op->ztype
    size_t xsize = mult->xtype->size ;
    size_t ysize = mult->ytype->size ;

    // scalar workspace: because of typecasting, the x/y types need not
    // be the same as the size of the A and B types.
    // flipxy false: aki = (xtype) A(k,i) and bkj = (ytype) B(k,j)
    // flipxy true:  aki = (ytype) A(k,i) and bkj = (xtype) B(k,j)
    size_t aki_size = flipxy ? ysize : xsize ;
    size_t bkj_size = flipxy ? xsize : ysize ;

    bool is_terminal = (add->terminal != NULL) ;

    GB_cast_function cast_A, cast_B ;
    if (flipxy)
    { 
        // A is typecasted to y, and B is typecasted to x
        cast_A = A_is_pattern ? NULL : 
                 GB_cast_factory (mult->ytype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL : 
                 GB_cast_factory (mult->xtype->code, B->type->code) ;
    }
    else
    { 
        // A is typecasted to x, and B is typecasted to y
        cast_A = A_is_pattern ? NULL :
                 GB_cast_factory (mult->xtype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL :
                 GB_cast_factory (mult->ytype->code, B->type->code) ;
    }

    //--------------------------------------------------------------------------
    // C = A'*B via dot products, function pointers, and typecasting
    //--------------------------------------------------------------------------

    // aki = A(i,k), located in Ax [A_iso?0:(pA)]
    #undef  GB_A_IS_PATTERN
    #define GB_A_IS_PATTERN 0
    #undef  GB_DECLAREA
    #define GB_DECLAREA(aki)                                        \
        GB_void aki [GB_VLA(aki_size)] ;
    #undef  GB_GETA
    #define GB_GETA(aki,Ax,pA,A_iso)                                \
        if (!A_is_pattern) cast_A (aki, Ax +((A_iso) ? 0:(pA)*asize), asize)

    // bkj = B(k,j), located in Bx [B_iso?0:pB]
    #undef  GB_B_IS_PATTERN
    #define GB_B_IS_PATTERN 0
    #undef  GB_DECLAREB
    #define GB_DECLAREB(bkj)                                        \
        GB_void bkj [GB_VLA(bkj_size)] ;
    #undef  GB_GETB
    #define GB_GETB(bkj,Bx,pB,B_iso)                                \
        if (!B_is_pattern) cast_B (bkj, Bx +((B_iso) ? 0:(pB)*bsize), bsize)

    // instead of GB_DECLARE_TERMINAL_CONST (zterminal):
    GB_void *restrict zterminal = (GB_void *) add->terminal ;
    GB_void *restrict zidentity = (GB_void *) add->identity ;

    // define cij for each task
    #undef  GB_DECLARE_IDENTITY
    #define GB_DECLARE_IDENTITY(cij)            \
        GB_void cij [GB_VLA(zsize)] ;           \
        memcpy (cij, zidentity, zsize)

    // Cx [p] = cij (note csize == zsize)
    #undef  GB_PUTC
    #define GB_PUTC(cij,Cx,p) memcpy (Cx +((p)*csize), cij, csize)

    // break if cij reaches the terminal value
    #undef  GB_IF_TERMINAL_BREAK
    #define GB_IF_TERMINAL_BREAK(z,zterminal)                       \
        if (is_terminal && memcmp (z, zterminal, zsize) == 0)       \
        {                                                           \
            break ;                                                 \
        }
    #undef  GB_TERMINAL_CONDITION
    #define GB_TERMINAL_CONDITION(z,zterminal)                      \
        (is_terminal && memcmp (z, zterminal, zsize) == 0)

    // C(i,j) += (A')(i,k) * B(k,j)
    #undef  GB_MULTADD
    #define GB_MULTADD(cij, aki, bkj, i, k, j)                      \
        GB_void zwork [GB_VLA(zsize)] ;                             \
        GB_MULT (zwork, aki, bkj, i, k, j) ;                        \
        fadd (cij, cij, zwork)

    // generic types for C and Z
    #undef  GB_C_TYPE
    #define GB_C_TYPE GB_void

    #undef  GB_Z_TYPE
    #define GB_Z_TYPE GB_void

    if (opcode == GB_FIRST_binop_code)
    { 
        // t = A(i,k)
        // fmult is not used and can be NULL (for user-defined types)
        ASSERT (!flipxy) ;
        ASSERT (B_is_pattern) ;
        #undef  GB_MULT
        #define GB_MULT(t, aik, bkj, i, k, j) memcpy (t, aik, zsize)
        #if defined ( GB_DOT2_GENERIC )
        #include "mxm/template/GB_AxB_dot2_meta.c"
        #elif defined ( GB_DOT3_GENERIC )
        #include "mxm/template/GB_AxB_dot3_meta.c"
        #endif
    }
    else if (opcode == GB_SECOND_binop_code)
    { 
        // t = B(i,k)
        // fmult is not used and can be NULL (for user-defined types)
        ASSERT (!flipxy) ;
        ASSERT (A_is_pattern) ;
        #undef  GB_MULT
        #define GB_MULT(t, aik, bkj, i, k, j) memcpy (t, bkj, zsize)
        #if defined ( GB_DOT2_GENERIC )
        #include "mxm/template/GB_AxB_dot2_meta.c"
        #elif defined ( GB_DOT3_GENERIC )
        #include "mxm/template/GB_AxB_dot3_meta.c"
        #endif
    }
    else if (fmult != NULL)
    {
        // standard binary op
        if (flipxy)
        { 
            // t = B(k,j) * (A')(i,k)
            #undef  GB_MULT
            #define GB_MULT(t, aki, bkj, i, k, j) fmult (t, bkj, aki)
            #if defined ( GB_DOT2_GENERIC )
            #include "mxm/template/GB_AxB_dot2_meta.c"
            #elif defined ( GB_DOT3_GENERIC )
            #include "mxm/template/GB_AxB_dot3_meta.c"
            #endif
        }
        else
        { 
            // t = (A')(i,k) * B(k,j)
            #undef  GB_MULT
            #define GB_MULT(t, aki, bkj, i, k, j) fmult (t, aki, bkj)
            #if defined ( GB_DOT2_GENERIC )
            #include "mxm/template/GB_AxB_dot2_meta.c"
            #elif defined ( GB_DOT3_GENERIC )
            #include "mxm/template/GB_AxB_dot3_meta.c"
            #endif
        }
    }
    else
    {
        // index binary op
        ASSERT (fmult_idx != NULL) ;
        const void *theta = mult->theta ;
        if (flipxy)
        { 
            // t = B(k,j) * (A')(i,k)
            #undef  GB_MULT
            #define GB_MULT(t, aki, bkj, i, k, j) \
                fmult_idx (t, bkj, j, k, aki, k, i, theta)
            #if defined ( GB_DOT2_GENERIC )
            #include "mxm/template/GB_AxB_dot2_meta.c"
            #elif defined ( GB_DOT3_GENERIC )
            #include "mxm/template/GB_AxB_dot3_meta.c"
            #endif
        }
        else
        { 
            // t = (A')(i,k) * B(k,j)
            #undef  GB_MULT
            #define GB_MULT(t, aki, bkj, i, k, j) \
                fmult_idx (t, aki, i, k, bkj, k, j, theta)
            #if defined ( GB_DOT2_GENERIC )
            #include "mxm/template/GB_AxB_dot2_meta.c"
            #elif defined ( GB_DOT3_GENERIC )
            #include "mxm/template/GB_AxB_dot3_meta.c"
            #endif
        }
    }
}

