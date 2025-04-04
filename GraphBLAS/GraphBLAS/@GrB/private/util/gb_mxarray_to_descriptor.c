//------------------------------------------------------------------------------
// gb_mxarray_to_descriptor: get the contents of a GraphBLAS Descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// get a GraphBLAS descriptor from a built-in struct.

#include "gb_interface.h"

#define LEN 100

static void get_descriptor
(
    GrB_Descriptor desc,            // GraphBLAS descriptor to modify
    const mxArray *desc_builtin,    // built-in struct with d.out, etc
    const char *fieldname,          // fieldname to extract from desc_builtin
    const int field                 // field to set in desc
)
{

    // find the field in the built-in struct
    int fieldnumber = mxGetFieldNumber (desc_builtin, fieldname) ;
    if (fieldnumber >= 0)
    {

        // the field is present
        mxArray *value = mxGetFieldByNumber (desc_builtin, 0, fieldnumber) ;

        // get the string from the built-in field
        char s [LEN+2] ;
        gb_mxstring_to_string (s, LEN, value, "field") ;

        // convert the string to a Descriptor value, and set the value
        if (MATCH (s, "default"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GxB_DEFAULT, field)) ;
        }
        else if (MATCH (s, "transpose"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GrB_TRAN, field)) ;
        }
        else if (MATCH (s, "complement"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GrB_COMP, field)) ;
        }
        else if (MATCH (s, "structure") || MATCH (s, "structural"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GrB_STRUCTURE, field)) ;
        }
        else if (MATCH (s, "structural complement"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GrB_COMP+GrB_STRUCTURE,
                field)) ;
        }
        else if (MATCH (s, "replace"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GrB_REPLACE, field)) ;
        }
        else if (MATCH (s, "gustavson"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GxB_AxB_GUSTAVSON, field)) ;
        }
        else if (MATCH (s, "dot"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GxB_AxB_DOT, field)) ;
        }
        else if (MATCH (s, "saxpy"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GxB_AxB_SAXPY, field)) ;
        }
        else if (MATCH (s, "hash"))
        { 
            OK (GrB_Descriptor_set_INT32 (desc, GxB_AxB_HASH, field)) ;
        }
        else
        { 
            // the string must be one of the strings listed above
            ERROR ("unrecognized descriptor value") ;
        }
    }
}

//------------------------------------------------------------------------------
// gb_mxarray_to_descriptor
//------------------------------------------------------------------------------

GrB_Descriptor gb_mxarray_to_descriptor // new descriptor, or NULL if none
(
    const mxArray *desc_builtin, // built-in struct with possible descriptor
    kind_enum_t *kind,          // GrB, sparse, or full
    int *fmt,                   // by row or by col
    int *sparsity,              // hypersparse/sparse/bitmap/full
    base_enum_t *base           // 0-based int, 1-based int, or 1-based double
)
{

    //--------------------------------------------------------------------------
    // check inputs and find the descriptor
    //--------------------------------------------------------------------------

    // set the defaults if no descriptor is present
    (*kind) = KIND_GRB ;
    (*fmt) = GxB_NO_FORMAT ;
    (*base) = BASE_DEFAULT ;
    (*sparsity) = 0 ;

    if (desc_builtin == NULL || !mxIsStruct (desc_builtin)
        || (mxGetField (desc_builtin, 0, "GraphBLASv10") != NULL)
        || (mxGetField (desc_builtin, 0, "GraphBLASv7_3") != NULL)
        || (mxGetField (desc_builtin, 0, "GraphBLASv5_1") != NULL)
        || (mxGetField (desc_builtin, 0, "GraphBLASv5") != NULL)
        || (mxGetField (desc_builtin, 0, "GraphBLASv4") != NULL)
        || (mxGetField (desc_builtin, 0, "GraphBLAS") != NULL))
    {
        // If present, the descriptor is a struct whose first field is not
        // "desc.GraphBLAS*". If not present, the GraphBLAS descriptor is NULL.
        // This is not an error.
        return (NULL) ;
    }

    //--------------------------------------------------------------------------
    // create the GraphBLAS descriptor
    //--------------------------------------------------------------------------

    GrB_Descriptor desc ;
    OK (GrB_Descriptor_new (&desc)) ;

    // get each component for the GraphBLAS GrB_Descriptor
    get_descriptor (desc, desc_builtin, "out" , GrB_OUTP) ;
    get_descriptor (desc, desc_builtin, "in0" , GrB_INP0) ;
    get_descriptor (desc, desc_builtin, "in1" , GrB_INP1) ;
    get_descriptor (desc, desc_builtin, "mask", GrB_MASK) ;
    get_descriptor (desc, desc_builtin, "axb" , GxB_AxB_METHOD) ;

    //--------------------------------------------------------------------------
    // get the desired kind of output
    //--------------------------------------------------------------------------

    mxArray *mxkind = mxGetField (desc_builtin, 0, "kind") ;
    if (mxkind != NULL)
    {
        // get the string from the built-in field
        char s [LEN+2] ;
        gb_mxstring_to_string (s, LEN, mxkind, "kind") ;
        if (MATCH (s, "grb") || MATCH (s, "default"))
        { 
            (*kind) = KIND_GRB ;        // @GrB matrix
        }
        else if (MATCH (s, "sparse"))
        { 
            (*kind) = KIND_SPARSE ;     // built-in sparse matrix
        }
        else if (MATCH (s, "full"))
        { 
            (*kind) = KIND_FULL ;       // built-in full matrix
        }
        else if (MATCH (s, "builtin")   // preferred
            || MATCH (s, "matlab")      // deprecated (use 'builtin')
            || MATCH (s, "octave"))     // 'builtin' is preferred
        {
            (*kind) = KIND_BUILTIN ;    // built-in sparse or full matrix
        }
        else
        { 
            ERROR ("invalid descriptor.kind") ;
        }
    }

    //--------------------------------------------------------------------------
    // get the desired format of output, if any
    //--------------------------------------------------------------------------

    mxArray *mxfmt = mxGetField (desc_builtin, 0, "format") ;
    if (mxfmt != NULL)
    {
        bool ok = gb_mxstring_to_format (mxfmt, fmt, sparsity) ;
        if (!ok)
        { 
            ERROR ("unknown format") ;
        }
    }

    //--------------------------------------------------------------------------
    // get the desired base
    //--------------------------------------------------------------------------

    mxArray *mxbase = mxGetField (desc_builtin, 0, "base") ;
    if (mxbase != NULL)
    {
        // get the string from the struct field
        char s [LEN+2] ;
        gb_mxstring_to_string (s, LEN, mxbase, "base") ;
        if (MATCH (s, "default"))
        { 
            // The indices are one-based integer by default.
            (*base) = BASE_DEFAULT ;
        }
        else if (MATCH (s, "zero-based") || MATCH (s, "zero-based int"))
        { 
            // zero-based indices are always uint64/uint32.  This is the
            // fastest option since GraphBLAS uses zero-based indices.
            (*base) = BASE_0_INT ;
        }
        else if (MATCH (s, "one-based") || MATCH (s, "one-based int"))
        { 
            // one-based indices, but in uint64/uint32 (the default)
            (*base) = BASE_1_INT ;
        }
        else if (MATCH (s, "double") || MATCH (s, "one-based double"))
        { 
            // one-based double indices
            (*base) = BASE_1_DOUBLE ;
        }
        else
        { 
            ERROR ("invalid descriptor.base") ;
        }
    }

    //--------------------------------------------------------------------------
    // return the non-null Descriptor to the caller
    //--------------------------------------------------------------------------

    return (desc) ;
}

