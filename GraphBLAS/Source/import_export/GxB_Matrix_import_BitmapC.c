//------------------------------------------------------------------------------
// GxB_Matrix_import_BitmapC: import a matrix in bitmap format, held by column
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "import_export/GB_export.h"

GrB_Info GxB_Matrix_import_BitmapC  // import a bitmap matrix, held by column
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    uint64_t nrows,     // number of rows of the matrix
    uint64_t ncols,     // number of columns of the matrix

    int8_t **Ab,        // bitmap
    void **Ax,          // values
    uint64_t Ab_size,   // size of Ab in bytes
    uint64_t Ax_size,   // size of Ax in bytes
    bool iso,           // if true, A is iso

    uint64_t nvals,     // # of entries in bitmap
    const GrB_Descriptor desc
)
{ 

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE0 ("GxB_Matrix_import_BitmapC (&A, type, nrows, ncols, "
        "&Ab, &Ax, Ab_size, Ax_size, iso, nvals, desc)") ;

    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;
    GB_GET_DESCRIPTOR_IMPORT (desc, fast_import) ;

    //--------------------------------------------------------------------------
    // import the matrix
    //--------------------------------------------------------------------------

    info = GB_import (false, A, type, nrows, ncols, false,
        NULL, 0,        // Ap
        NULL, 0,        // Ah
        Ab,   Ab_size,  // Ab
        NULL, 0,        // Ai
        Ax,   Ax_size,  // Ax
        nvals, false, 0,                    // nvals for bitmap
        GxB_BITMAP, true,                   // bitmap by col
        iso, fast_import, true, Werk) ;

    return (info) ;
}

