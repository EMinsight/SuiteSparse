//------------------------------------------------------------------------------
// CHOLMOD/MATLAB/spsym: MATLAB interface to CHOLMOD sparse symmetry metrics
//------------------------------------------------------------------------------

// CHOLMOD/MATLAB Module.  Copyright (C) 2005-2023, Timothy A. Davis.
// All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

// [result xmatched pmatched nzoffdiag nzdiag] = spsym (A, quick).
// See the spsym.m file for a description of what it computes.

#include "sputil2.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    double dummy = 0 ;
    cholmod_sparse Amatrix, *A ;
    cholmod_common Common, *cm ;
    int64_t result, quick, option, xmatched, pmatched, nzoffdiag, nzdiag ;

    //--------------------------------------------------------------------------
    // start CHOLMOD and set parameters
    //--------------------------------------------------------------------------

    cm = &Common ;
    cholmod_l_start (cm) ;
    sputil2_config (SPUMONI, cm) ;

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    if (nargin > 2 || nargin < 1 || nargout > 5)
    {
        mexErrMsgTxt ("usage: [s xmatch pmatch nzoff nzd] = spsym (A,quick)") ;
    }
    if (!mxIsSparse (pargin [0]))
    {
        mexErrMsgTxt ("A must be sparse and double") ;
    }

    // get sparse matrix A
    size_t A_xsize = 0 ;
    A = sputil2_get_sparse (pargin [0], 0, CHOLMOD_DOUBLE, &Amatrix,
        &A_xsize, cm) ;

    // get the "quick" parameter
    quick = (nargin > 1) ? (mxGetScalar (pargin [1]) != 0) : FALSE ;

    if (nargout > 1)
    {
        option = 2 ;
    }
    else if (quick)
    {
        option = 0 ;
    }
    else
    {
        option = 1 ;
    }

    //--------------------------------------------------------------------------
    // determine symmetry
    //--------------------------------------------------------------------------

    xmatched = 0 ;
    pmatched = 0 ;
    nzoffdiag = 0 ;
    nzdiag = 0 ;

    result = cholmod_l_symmetry (A, option, &xmatched, &pmatched, &nzoffdiag,
        &nzdiag, cm) ;

    //--------------------------------------------------------------------------
    // return results to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = sputil2_put_int (&result, 1, 0) ;

    if (nargout > 1) pargout [1] = sputil2_put_int (&xmatched, 1, 0) ;
    if (nargout > 2) pargout [2] = sputil2_put_int (&pmatched, 1, 0) ;
    if (nargout > 3) pargout [3] = sputil2_put_int (&nzoffdiag, 1, 0) ;
    if (nargout > 4) pargout [4] = sputil2_put_int (&nzdiag, 1, 0) ;

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    sputil2_free_sparse (&A, &Amatrix, A_xsize, cm) ;
    cholmod_l_finish (cm) ;
    if (SPUMONI > 0) cholmod_l_print_common (" ", cm) ;
}

