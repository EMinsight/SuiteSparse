// =============================================================================
// === spqr_stranspose2 ========================================================
// =============================================================================

// SPQR, Copyright (c) 2008-2022, Timothy A Davis. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

// Construct the numerical values of S = A (p,q) in compressed-row form

#include "spqr.hpp"

template <typename Entry, typename Int> void spqr_stranspose2
(
    // input, not modified
    cholmod_sparse *A,  // m-by-n
    Int *Qfill,        // size n, fill-reducing column permutation;
                        // Qfill [k] = j
                        // if the kth column of S is the jth column of A.
                        // Identity permutation is used if Qfill is NULL.

    Int *Sp,           // size m+1, row pointers of S
    Int *PLinv,        // size m, inverse row permutation, PLinv [i] = k

    // output, contents not defined on input
    Entry *Sx,          // size nz, numerical values of S

    // workspace, not defined on input or output
    Int *W             // size m
)
{
    Int i, j, p, pend, row, col, s, m, n, *Ap, *Ai ;
    Entry *Ax ;

    // -------------------------------------------------------------------------
    // get inputs
    // -------------------------------------------------------------------------

    m = A->nrow ;
    n = A->ncol ;
    Ap = (Int *) A->p ;
    Ai = (Int *) A->i ;
    Ax = (Entry *) A->x ;

    // -------------------------------------------------------------------------
    // create S = A (p,q)', or S=A(p,q) if S is considered to be in row-form
    // -------------------------------------------------------------------------

    for (row = 0 ; row < m ; row++)
    {
        W [row] = Sp [row] ;
    }

    for (col = 0 ; col < n ; col++)     // for each column of A(:,Qfill)
    {
        j = Qfill ? Qfill [col] : col ; // col of S is column j of A
        pend = Ap [j+1] ;
        for (p = Ap [j] ; p < pend ; p++)
        {
            i = Ai [p] ;                // the entry A(i,j)
            row = PLinv [i] ;           // row of S is row i of A
            s = W [row]++ ;             // place S(row,col) in position
            Sx [s] = Ax [p] ;
        }
    }
}

template void spqr_stranspose2 <double, int32_t>
(
    // input, not modified
    cholmod_sparse *A,  // m-by-n
    int32_t *Qfill,        // size n, fill-reducing column permutation;
                        // Qfill [k] = j
                        // if the kth column of S is the jth column of A.
                        // Identity permutation is used if Qfill is NULL.

    int32_t *Sp,           // size m+1, row pointers of S
    int32_t *PLinv,        // size m, inverse row permutation, PLinv [i] = k

    // output, contents not defined on input
    double *Sx,          // size nz, numerical values of S

    // workspace, not defined on input or output
    int32_t *W             // size m
) ;
template void spqr_stranspose2 <Complex, int32_t>
(
    // input, not modified
    cholmod_sparse *A,  // m-by-n
    int32_t *Qfill,        // size n, fill-reducing column permutation;
                        // Qfill [k] = j
                        // if the kth column of S is the jth column of A.
                        // Identity permutation is used if Qfill is NULL.

    int32_t *Sp,           // size m+1, row pointers of S
    int32_t *PLinv,        // size m, inverse row permutation, PLinv [i] = k

    // output, contents not defined on input
    Complex *Sx,          // size nz, numerical values of S

    // workspace, not defined on input or output
    int32_t *W             // size m
) ;

template void spqr_stranspose2 <double, int64_t>
(
    // input, not modified
    cholmod_sparse *A,  // m-by-n
    int64_t *Qfill,        // size n, fill-reducing column permutation;
                        // Qfill [k] = j
                        // if the kth column of S is the jth column of A.
                        // Identity permutation is used if Qfill is NULL.

    int64_t *Sp,           // size m+1, row pointers of S
    int64_t *PLinv,        // size m, inverse row permutation, PLinv [i] = k

    // output, contents not defined on input
    double *Sx,          // size nz, numerical values of S

    // workspace, not defined on input or output
    int64_t *W             // size m
) ;
template void spqr_stranspose2 <Complex, int64_t>
(
    // input, not modified
    cholmod_sparse *A,  // m-by-n
    int64_t *Qfill,        // size n, fill-reducing column permutation;
                        // Qfill [k] = j
                        // if the kth column of S is the jth column of A.
                        // Identity permutation is used if Qfill is NULL.

    int64_t *Sp,           // size m+1, row pointers of S
    int64_t *PLinv,        // size m, inverse row permutation, PLinv [i] = k

    // output, contents not defined on input
    Complex *Sx,          // size nz, numerical values of S

    // workspace, not defined on input or output
    int64_t *W             // size m
) ;
