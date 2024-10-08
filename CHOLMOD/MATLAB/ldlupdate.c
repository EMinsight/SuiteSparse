//------------------------------------------------------------------------------
// CHOLMOD/MATLAB/ldlupdate: MATLAB interface to CHOLMOD update/downdate
//------------------------------------------------------------------------------

// CHOLMOD/MATLAB Module.  Copyright (C) 2005-2023, Timothy A. Davis.
// All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

// Multiple-rank update or downdate of a sparse LDL' factorization.
//
// Usage:
//
//      LD = ldlupdate (LD,C)           update an LDL' factorization
//      LD = ldlupdate (LD,C,'+')       update an LDL' factorization
//      LD = ldlupdate (LD,C,'-')       downdate an LDL' factorization
//
// See ldlupdate.m for details.  LD and C must be real and sparse.
//
// The bulk of the time is spent copying the input LD to the output LD.  This
// mexFunction could be much faster if it could safely modify its input LD.

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
    double *Lx, *Lx2 ;
    int64_t *Li, *Lp, *Li2, *Lp2, *Lnz2, *ColCount ;
    cholmod_sparse Cmatrix, *C, *Lsparse ;
    cholmod_factor *L ;
    cholmod_common Common, *cm ;
    int64_t j, k, s, update, n, lnz ;
    char buf [LEN] ;

    //--------------------------------------------------------------------------
    // start CHOLMOD and set parameters
    //--------------------------------------------------------------------------

    cm = &Common ;
    cholmod_l_start (cm) ;
    sputil2_config (SPUMONI, cm) ;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (nargout > 1 || nargin < 2 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: L = ldlupdate (L, C, '+')") ;
    }

    n = mxGetN (pargin [0]) ;
    k = mxGetN (pargin [1]) ;

    if (!mxIsSparse (pargin [0]) || !mxIsSparse (pargin [1])
            || n != mxGetM (pargin [0]) || n != mxGetM (pargin [1])
            || mxIsComplex (pargin [0]) || mxIsComplex (pargin [1]))
    {
        mexErrMsgTxt ("ldlupdate: C and/or L not sparse, complex, or wrong"
                " dimensions") ;
    }

    //--------------------------------------------------------------------------
    // determine if we're doing an update or downdate
    //--------------------------------------------------------------------------

    update = TRUE ;
    if (nargin > 2 && mxIsChar (pargin [2]))
    {
        mxGetString (pargin [2], buf, LEN) ;
        if (buf [0] == '-')
        {
            update = FALSE ;
        }
        else if (buf [0] != '+')
        {
            mexErrMsgTxt ("ldlupdate: update string must be '+' or '-'") ;
        }
    }

    //--------------------------------------------------------------------------
    // get C: sparse matrix of incoming/outgoing columns
    //--------------------------------------------------------------------------

    size_t C_xsize = 0 ;
    C = sputil2_get_sparse (pargin [1], 0, CHOLMOD_DOUBLE, &Cmatrix,
        &C_xsize, cm) ;

    //--------------------------------------------------------------------------
    // construct a copy of the input sparse matrix L
    //--------------------------------------------------------------------------

    // get the MATLAB L
    Lp = (int64_t *) mxGetJc (pargin [0]) ;
    Li = (int64_t *) mxGetIr (pargin [0]) ;
    Lx = (double *) mxGetData (pargin [0]) ;

    // allocate the CHOLMOD symbolic L
    L = cholmod_l_allocate_factor (n, cm) ;
    L->ordering = CHOLMOD_NATURAL ;
    ColCount = L->ColCount ;
    for (j = 0 ; j < n ; j++)
    {
        ColCount [j] = Lp [j+1] - Lp [j] ;
    }

    // allocate space for a CHOLMOD LDL' packed factor
    cholmod_l_change_factor (CHOLMOD_REAL, FALSE, FALSE, TRUE, TRUE, L, cm) ;

    // copy MATLAB L into CHOLMOD L
    Lp2 = L->p ;
    Li2 = L->i ;
    Lx2 = L->x ;
    Lnz2 = L->nz ;
    lnz = L->nzmax ;
    for (j = 0 ; j <= n ; j++)
    {
        Lp2 [j] = Lp [j] ;
    }
    for (j = 0 ; j < n ; j++)
    {
        Lnz2 [j] = Lp [j+1] - Lp [j] ;
    }
    for (s = 0 ; s < lnz ; s++)
    {
        Li2 [s] = Li [s] ;
    }
    for (s = 0 ; s < lnz ; s++)
    {
        Lx2 [s] = Lx [s] ;
    }

    //--------------------------------------------------------------------------
    // update/downdate the LDL' factorization
    //--------------------------------------------------------------------------

    if (!cholmod_l_updown (update, C, L, cm))
    {
        mexErrMsgTxt ("ldlupdate failed\n") ;
    }

    //--------------------------------------------------------------------------
    // copy the results back to MATLAB
    //--------------------------------------------------------------------------

    // change L back to packed LDL' (it may have become unpacked if the
    // sparsity pattern changed).  This change takes O(n) time if the pattern
    // of L wasn't updated.
    Lsparse = cholmod_l_factor_to_sparse (L, cm) ;

    // return L as a sparse matrix; it may contain numerically zero entries,
    // which must be kept to allow update/downdate to work.
    pargout [0] = sputil2_put_sparse (&Lsparse, mxDOUBLE_CLASS,
        /* return L with explicit zeros kept */ false, cm) ;

    //--------------------------------------------------------------------------
    // free workspace and the CHOLMOD L, except for what is copied to MATLAB
    //--------------------------------------------------------------------------

    sputil2_free_sparse (&C, &Cmatrix, C_xsize, cm) ;
    cholmod_l_free_factor (&L, cm) ;
    cholmod_l_finish (cm) ;
    if (SPUMONI > 0) cholmod_l_print_common (" ", cm) ;
}

