//------------------------------------------------------------------------------
// GxB_IndexUnaryOp_ytype_name: return the type_name of y for z=f(x,i,j,y)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_IndexUnaryOp_ytype_name    // return name of type of scalar y
(
    char *type_name,        // name of the type (char array of size at least
                            // GxB_MAX_NAME_LEN, owned by the user application).
    const GrB_IndexUnaryOp op
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_CHECK_INIT ;
    GB_RETURN_IF_NULL (type_name) ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;
    ASSERT_INDEXUNARYOP_OK (op, "op for ytype_name", GB0) ;

    //--------------------------------------------------------------------------
    // get the type_name
    //--------------------------------------------------------------------------

    memcpy (type_name, op->ytype->name, GxB_MAX_NAME_LEN) ;
    #pragma omp flush
    return (GrB_SUCCESS) ;
}

