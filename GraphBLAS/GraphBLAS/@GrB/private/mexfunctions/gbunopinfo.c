//------------------------------------------------------------------------------
// gbunopinfo : print a GraphBLAS unary op (for illustration only)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Usage:

// gbunopinfo (unop)
// gbunopinfo (unop, type)
// ok = gbunopinfo (unop)

#include "gb_interface.h"

#define USAGE "usage: GrB.unopinfo (unop) or GrB.unopinfo (unop,type)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin >= 1 && nargin <= 2 && nargout <= 1, USAGE) ;

    //--------------------------------------------------------------------------
    // construct the GraphBLAS unary operator and print it
    //--------------------------------------------------------------------------

    #define LEN 256
    char opstring [LEN+2] ;
    gb_mxstring_to_string (opstring, LEN, pargin [0], "unary operator") ;

    GrB_Type type = NULL ;
    if (nargin == 2)
    { 
        type = gb_mxstring_to_type (pargin [1]) ;
        CHECK_ERROR (type == NULL, "unknown type") ;
    }

    GrB_UnaryOp op = gb_mxstring_to_unop (pargin [0], type) ;
    int pr = (nargout < 1) ? GxB_COMPLETE : GxB_SILENT ;
    OK (GxB_UnaryOp_fprint (op, opstring, pr, NULL)) ;
    if (nargout == 1)
    {
        pargout [0] = mxCreateLogicalScalar (true) ;
    }
    gb_wrapup ( ) ;
}

