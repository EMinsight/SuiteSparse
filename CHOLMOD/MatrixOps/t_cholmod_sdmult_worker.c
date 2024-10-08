//------------------------------------------------------------------------------
// CHOLMOD/MatrixOps/t_cholmod_sdmult_worker
//------------------------------------------------------------------------------

// CHOLMOD/MatrixOps Module.  Copyright (C) 2005-2023, Timothy A. Davis.
// All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

#include "cholmod_template.h"

//------------------------------------------------------------------------------
// ADVANCE macro
//------------------------------------------------------------------------------

#ifdef REAL
#define ADVANCE(x,z,d) x += d
#elif defined (COMPLEX)
#define ADVANCE(x,z,d) x += 2*d
#else
#define ADVANCE(x,z,d) x += d ; z += d
#endif

//------------------------------------------------------------------------------
// t_cholmod_sdmult_worker
//------------------------------------------------------------------------------

static void TEMPLATE (cholmod_sdmult_worker)
(
    // input:
    cholmod_sparse *A,  // sparse matrix to multiply
    int transpose,      // use A if 0, or A' otherwise
    Real alpha [2],     // scale factor for A
    Real beta [2],      // scale factor for Y
    cholmod_dense *X,   // dense matrix to multiply
    // input/output:
    cholmod_dense *Y,   // resulting dense matrix
    // workspace
    Real *W             // size 4*nx if needed, twice that for c/zomplex case
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    Real yx [8], xx [8], ax [2] ;

    #ifdef ZOMPLEX
    Real yz [4], xz [4], az [1] ;
    Real betaz [1], alphaz [1] ;
    betaz  [0] = beta  [1] ;
    alphaz [0] = alpha [1] ;
    #endif

    size_t ny = transpose ? A->ncol : A->nrow ;        // required length of Y
    size_t nx = transpose ? A->nrow : A->ncol ;        // required length of X

    Int nrow = A->nrow ;
    Int ncol = A->ncol ;

    Int *Ap  = A->p ;
    Int *Anz = A->nz ;
    Int *Ai  = A->i ;
    Real *Ax = A->x ;
    Real *Az = A->z ;
    bool packed = A->packed ;

    Real *Xx = X->x ;
    Real *Xz = X->z ;
    Real *Yx = Y->x ;
    Real *Yz = Y->z ;

    Int kcol = X->ncol ;
    size_t dy = Y->d ;
    size_t dx = X->d ;
    Real *w = W ;
    Real *Wz = W + 4*nx ;

    //--------------------------------------------------------------------------
    // Y = beta * Y
    //--------------------------------------------------------------------------

    if (ENTRY_IS_ZERO (beta, betaz, 0))
    {
        for (Int k = 0 ; k < kcol ; k++)
        {
            for (Int i = 0 ; i < ((Int) ny) ; i++)
            {
                // y [i] = 0
                CLEAR (Yx, Yz, i) ;
            }
            // y += dy
            ADVANCE (Yx,Yz,dy) ;
        }
    }
    else if (!ENTRY_IS_ONE (beta, betaz, 0))
    {
        for (Int k = 0 ; k < kcol ; k++)
        {
            for (Int i = 0 ; i < ((Int) ny) ; i++)
            {
                // y [i] *= beta [0]
                MULT (Yx,Yz,i, Yx,Yz,i, beta,betaz, 0) ;
            }
            // y += dy
            ADVANCE (Yx,Yz,dy) ;
        }
    }

    if (ENTRY_IS_ZERO (alpha, alphaz, 0))
    {
        // nothing else to do
        return ;
    }

    //--------------------------------------------------------------------------
    // Y += alpha * op(A) * X, where op(A)=A or A'
    //--------------------------------------------------------------------------

    Yx = Y->x ;
    Yz = Y->z ;
    Int k = 0 ;

    if (A->stype == 0)
    {

        if (transpose)
        {

            //------------------------------------------------------------------
            // Y += alpha * A' * x, unsymmetric case
            //------------------------------------------------------------------

            if (kcol % 4 == 1)
            {

                for (Int j = 0 ; j < ncol ; j++)
                {
                    // yj = 0
                    CLEAR (yx, yz, 0) ;
                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        // yj += conj(Ax [p]) * x [Ai [p]]
                        Int i = Ai [p] ;
                        ASSIGN_CONJ (ax,az,0, Ax,Az,p) ;
                        MULTADD (yx,yz,0, ax,az,0, Xx,Xz,i) ;
                    }
                    // y [j] += alpha [0] * yj
                    MULTADD (Yx,Yz,j, alpha,alphaz,0, yx,yz,0) ;
                }
                // y += dy
                // x += dx
                ADVANCE (Yx,Yz,dy) ;
                ADVANCE (Xx,Xz,dx) ;
                k++ ;

            }
            else if (kcol % 4 == 2)
            {

                for (Int j = 0 ; j < ncol ; j++)
                {
                    // yj0 = 0
                    // yj1 = 0
                    CLEAR (yx,yz,0) ;
                    CLEAR (yx,yz,1) ;

                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        Int i = Ai [p] ;
                        // aij = conj (Ax [p])
                        ASSIGN_CONJ (ax,az,0, Ax,Az,p) ;

                        // yj0 += aij * x [i   ]
                        // yj1 += aij * x [i+dx]
                        MULTADD (yx,yz,0, ax,az,0, Xx,Xz,i) ;
                        MULTADD (yx,yz,1, ax,az,0, Xx,Xz,i+dx) ;
                    }
                    // y [j   ] += alpha [0] * yj0
                    // y [j+dy] += alpha [0] * yj1
                    MULTADD (Yx,Yz,j,      alpha,alphaz,0, yx,yz,0) ;
                    MULTADD (Yx,Yz,j+dy,   alpha,alphaz,0, yx,yz,1) ;
                }
                // y += 2*dy
                // x += 2*dx
                ADVANCE (Yx,Yz,2*dy) ;
                ADVANCE (Xx,Xz,2*dx) ;
                k += 2 ;

            }
            else if (kcol % 4 == 3)
            {

                for (Int j = 0 ; j < ncol ; j++)
                {
                    // yj0 = 0
                    // yj1 = 0
                    // yj2 = 0
                    CLEAR (yx,yz,0) ;
                    CLEAR (yx,yz,1) ;
                    CLEAR (yx,yz,2) ;

                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        Int i = Ai [p] ;
                        // aij = conj (Ax [p])
                        ASSIGN_CONJ (ax,az,0, Ax,Az,p) ;

                        // yj0 += aij * x [i     ]
                        // yj1 += aij * x [i+  dx]
                        // yj2 += aij * x [i+2*dx]
                        MULTADD (yx,yz,0, ax,az,0, Xx,Xz,i) ;
                        MULTADD (yx,yz,1, ax,az,0, Xx,Xz,i+dx) ;
                        MULTADD (yx,yz,2, ax,az,0, Xx,Xz,i+2*dx) ;
                    }
                    // y [j     ] += alpha [0] * yj0
                    // y [j+  dy] += alpha [0] * yj1
                    // y [j+2*dy] += alpha [0] * yj2
                    MULTADD (Yx,Yz,j,      alpha,alphaz,0, yx,yz,0) ;
                    MULTADD (Yx,Yz,j+dy,   alpha,alphaz,0, yx,yz,1) ;
                    MULTADD (Yx,Yz,j+2*dy, alpha,alphaz,0, yx,yz,2) ;
                }
                // y += 3*dy
                // x += 3*dx
                ADVANCE (Yx,Yz,3*dy) ;
                ADVANCE (Xx,Xz,3*dx) ;
                k += 3 ;
            }

            for ( ; k < kcol ; k += 4)
            {
                for (Int j = 0 ; j < ncol ; j++)
                {
                    // yj0 = 0
                    // yj1 = 0
                    // yj2 = 0
                    // yj3 = 0
                    CLEAR (yx,yz,0) ;
                    CLEAR (yx,yz,1) ;
                    CLEAR (yx,yz,2) ;
                    CLEAR (yx,yz,3) ;

                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        Int i = Ai [p] ;
                        // aij = conj(Ax [p])
                        ASSIGN_CONJ (ax,az,0, Ax,Az,p) ;

                        // yj0 += aij * x [i     ]
                        // yj1 += aij * x [i+  dx]
                        // yj2 += aij * x [i+2*dx]
                        // yj3 += aij * x [i+3*dx]
                        MULTADD (yx,yz,0, ax,az,0, Xx,Xz,i) ;
                        MULTADD (yx,yz,1, ax,az,0, Xx,Xz,i+dx) ;
                        MULTADD (yx,yz,2, ax,az,0, Xx,Xz,i+2*dx) ;
                        MULTADD (yx,yz,3, ax,az,0, Xx,Xz,i+3*dx) ;

                    }
                    // y [j     ] += alpha [0] * yj0
                    // y [j+  dy] += alpha [0] * yj1
                    // y [j+2*dy] += alpha [0] * yj2
                    // y [j+3*dy] += alpha [0] * yj3
                    MULTADD (Yx,Yz,j,      alpha,alphaz,0, yx,yz,0) ;
                    MULTADD (Yx,Yz,j+dy,   alpha,alphaz,0, yx,yz,1) ;
                    MULTADD (Yx,Yz,j+2*dy, alpha,alphaz,0, yx,yz,2) ;
                    MULTADD (Yx,Yz,j+3*dy, alpha,alphaz,0, yx,yz,3) ;
                }
                // y += 4*dy
                // x += 4*dx
                ADVANCE (Yx,Yz,4*dy) ;
                ADVANCE (Xx,Xz,4*dx) ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Y += alpha * A * x, unsymmetric case
            //------------------------------------------------------------------

            if (kcol % 4 == 1)
            {

                for (Int j = 0 ; j < ncol ; j++)
                {
                    //  xj = alpha [0] * x [j]
                    MULT (xx,xz,0, alpha,alphaz,0, Xx,Xz,j) ;

                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        // y [Ai [p]] += Ax [p] * xj
                        Int i = Ai [p] ;
                        MULTADD (Yx,Yz,i, Ax,Az,p, xx,xz,0) ;
                    }
                }
                // y += dy
                // x += dx
                ADVANCE (Yx,Yz,dy) ;
                ADVANCE (Xx,Xz,dx) ;
                k++ ;

            }
            else if (kcol % 4 == 2)
            {

                for (Int j = 0 ; j < ncol ; j++)
                {
                    // xj0 = alpha [0] * x [j   ]
                    // xj1 = alpha [0] * x [j+dx]
                    MULT (xx,xz,0, alpha,alphaz,0, Xx,Xz,j) ;
                    MULT (xx,xz,1, alpha,alphaz,0, Xx,Xz,j+dx) ;

                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        Int i = Ai [p] ;
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i   ] += aij * xj0
                        // y [i+dy] += aij * xj1
                        MULTADD (Yx,Yz,i,    ax,az,0, xx,xz,0) ;
                        MULTADD (Yx,Yz,i+dy, ax,az,0, xx,xz,1) ;
                    }
                }
                // y += 2*dy
                // x += 2*dx
                ADVANCE (Yx,Yz,2*dy) ;
                ADVANCE (Xx,Xz,2*dx) ;
                k += 2 ;

            }
            else if (kcol % 4 == 3)
            {

                for (Int j = 0 ; j < ncol ; j++)
                {
                    // xj0 = alpha [0] * x [j     ]
                    // xj1 = alpha [0] * x [j+  dx]
                    // xj2 = alpha [0] * x [j+2*dx]
                    MULT (xx,xz,0, alpha,alphaz,0, Xx,Xz,j) ;
                    MULT (xx,xz,1, alpha,alphaz,0, Xx,Xz,j+dx) ;
                    MULT (xx,xz,2, alpha,alphaz,0, Xx,Xz,j+2*dx) ;

                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        Int i = Ai [p] ;
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i     ] += aij * xj0
                        // y [i+  dy] += aij * xj1
                        // y [i+2*dy] += aij * xj2
                        MULTADD (Yx,Yz,i,      ax,az,0, xx,xz,0) ;
                        MULTADD (Yx,Yz,i+dy,   ax,az,0, xx,xz,1) ;
                        MULTADD (Yx,Yz,i+2*dy, ax,az,0, xx,xz,2) ;
                    }
                }
                // y += 3*dy
                // x += 3*dx
                ADVANCE (Yx,Yz,3*dy) ;
                ADVANCE (Xx,Xz,3*dx) ;
                k += 3 ;
            }

            for ( ; k < kcol ; k += 4)
            {
                for (Int j = 0 ; j < ncol ; j++)
                {
                    // xj0 = alpha [0] * x [j     ]
                    // xj1 = alpha [0] * x [j+  dx]
                    // xj2 = alpha [0] * x [j+2*dx]
                    // xj3 = alpha [0] * x [j+3*dx]
                    MULT (xx,xz,0, alpha,alphaz,0, Xx,Xz,j) ;
                    MULT (xx,xz,1, alpha,alphaz,0, Xx,Xz,j+dx) ;
                    MULT (xx,xz,2, alpha,alphaz,0, Xx,Xz,j+2*dx) ;
                    MULT (xx,xz,3, alpha,alphaz,0, Xx,Xz,j+3*dx) ;

                    Int p = Ap [j] ;
                    Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                    for ( ; p < pend ; p++)
                    {
                        Int i = Ai [p] ;
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i     ] += aij * xj0
                        // y [i+  dy] += aij * xj1
                        // y [i+2*dy] += aij * xj2
                        // y [i+3*dy] += aij * xj3
                        MULTADD (Yx,Yz,i,      ax,az,0, xx,xz,0) ;
                        MULTADD (Yx,Yz,i+dy,   ax,az,0, xx,xz,1) ;
                        MULTADD (Yx,Yz,i+2*dy, ax,az,0, xx,xz,2) ;
                        MULTADD (Yx,Yz,i+3*dy, ax,az,0, xx,xz,3) ;
                    }
                }
                // y += 4*dy
                // x += 4*dx
                ADVANCE (Yx,Yz,4*dy) ;
                ADVANCE (Xx,Xz,4*dx) ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // Y += alpha * A * x, symmetric case (upper/lower)
        //----------------------------------------------------------------------

        // Only the upper/lower triangular part and the diagonal of A is used.
        // Since both x and y are written to in the innermost loop, this code
        // can experience cache bank conflicts if x is used directly.  Thus, a
        // copy is made of x, four columns at a time, if x has four or more
        // columns.

        if (kcol % 4 == 1)
        {

            for (Int j = 0 ; j < ncol ; j++)
            {
                // yj = 0
                CLEAR (yx,yz,0) ;

                // xj = alpha [0] * x [j]
                MULT (xx,xz,0, alpha,alphaz,0, Xx,Xz,j) ;

                Int p = Ap [j] ;
                Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                for ( ; p < pend ; p++)
                {
                    Int i = Ai [p] ;
                    if (i == j)
                    {
                        // y [i] += Ax [p] * xj
                        MULTADD (Yx,Yz,i, Ax,Az,p, xx,xz,0) ;
                    }
                    else if ((A->stype > 0 && i < j) || (A->stype < 0 && i > j))
                    {
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i] += aij * xj
                        // yj    += aij * x [i]
                        MULTADD     (Yx,Yz,i, ax,az,0, xx,xz,0) ;
                        MULTADDCONJ (yx,yz,0, ax,az,0, Xx,Xz,i) ;


                    }
                }
                // y [j] += alpha [0] * yj
                MULTADD (Yx,Yz,j, alpha,alphaz,0, yx,yz,0) ;

            }
            // y += dy
            // x += dx
            ADVANCE (Yx,Yz,dy) ;
            ADVANCE (Xx,Xz,dx) ;
            k++ ;

        }
        else if (kcol % 4 == 2)
        {

            for (Int j = 0 ; j < ncol ; j++)
            {
                // yj0 = 0
                // yj1 = 0
                CLEAR (yx,yz,0) ;
                CLEAR (yx,yz,1) ;

                // xj0 = alpha [0] * x [j   ]
                // xj1 = alpha [0] * x [j+dx]
                MULT (xx,xz,0, alpha,alphaz,0, Xx,Xz,j) ;
                MULT (xx,xz,1, alpha,alphaz,0, Xx,Xz,j+dx) ;

                Int p = Ap [j] ;
                Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                for ( ; p < pend ; p++)
                {
                    Int i = Ai [p] ;
                    if (i == j)
                    {
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i   ] += aij * xj0
                        // y [i+dy] += aij * xj1
                        MULTADD (Yx,Yz,i,    ax,az,0, xx,xz,0) ;
                        MULTADD (Yx,Yz,i+dy, ax,az,0, xx,xz,1) ;

                    }
                    else if ((A->stype > 0 && i < j) || (A->stype < 0 && i > j))
                    {
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i   ] += aij * xj0
                        // y [i+dy] += aij * xj1
                        // yj0 += aij * x [i   ]
                        // yj1 += aij * x [i+dx]
                        MULTADD     (Yx,Yz,i,    ax,az,0, xx,xz,0) ;
                        MULTADD     (Yx,Yz,i+dy, ax,az,0, xx,xz,1) ;
                        MULTADDCONJ (yx,yz,0,    ax,az,0, Xx,Xz,i) ;
                        MULTADDCONJ (yx,yz,1,    ax,az,0, Xx,Xz,i+dx) ;

                    }
                }
                // y [j   ] += alpha [0] * yj0
                // y [j+dy] += alpha [0] * yj1
                MULTADD (Yx,Yz,j,    alpha,alphaz,0, yx,yz,0) ;
                MULTADD (Yx,Yz,j+dy, alpha,alphaz,0, yx,yz,1) ;

            }
            // y += 2*dy
            // x += 2*dx
            ADVANCE (Yx,Yz,2*dy) ;
            ADVANCE (Xx,Xz,2*dx) ;
            k += 2 ;

        }
        else if (kcol % 4 == 3)
        {

            for (Int j = 0 ; j < ncol ; j++)
            {
                // yj0 = 0
                // yj1 = 0
                // yj2 = 0
                CLEAR (yx,yz,0) ;
                CLEAR (yx,yz,1) ;
                CLEAR (yx,yz,2) ;

                // xj0 = alpha [0] * x [j     ]
                // xj1 = alpha [0] * x [j+  dx]
                // xj2 = alpha [0] * x [j+2*dx]
                MULT (xx,xz,0, alpha,alphaz,0, Xx,Xz,j) ;
                MULT (xx,xz,1, alpha,alphaz,0, Xx,Xz,j+dx) ;
                MULT (xx,xz,2, alpha,alphaz,0, Xx,Xz,j+2*dx) ;

                Int p = Ap [j] ;
                Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                for ( ; p < pend ; p++)
                {
                    Int i = Ai [p] ;
                    if (i == j)
                    {

                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i     ] += aij * xj0
                        // y [i+  dy] += aij * xj1
                        // y [i+2*dy] += aij * xj2
                        MULTADD (Yx,Yz,i,      ax,az,0, xx,xz,0) ;
                        MULTADD (Yx,Yz,i+dy,   ax,az,0, xx,xz,1) ;
                        MULTADD (Yx,Yz,i+2*dy, ax,az,0, xx,xz,2) ;

                    }
                    else if ((A->stype > 0 && i < j) || (A->stype < 0 && i > j))
                    {

                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i     ] += aij * xj0
                        // y [i+  dy] += aij * xj1
                        // y [i+2*dy] += aij * xj2
                        // yj0 += aij * x [i     ]
                        // yj1 += aij * x [i+  dx]
                        // yj2 += aij * x [i+2*dx]
                        MULTADD     (Yx,Yz,i,      ax,az,0, xx,xz,0) ;
                        MULTADD     (Yx,Yz,i+dy,   ax,az,0, xx,xz,1) ;
                        MULTADD     (Yx,Yz,i+2*dy, ax,az,0, xx,xz,2) ;
                        MULTADDCONJ (yx,yz,0,      ax,az,0, Xx,Xz,i) ;
                        MULTADDCONJ (yx,yz,1,      ax,az,0, Xx,Xz,i+dx) ;
                        MULTADDCONJ (yx,yz,2,      ax,az,0, Xx,Xz,i+2*dx) ;

                    }
                }
                // y [j     ] += alpha [0] * yj0
                // y [j+  dy] += alpha [0] * yj1
                // y [j+2*dy] += alpha [0] * yj2
                MULTADD (Yx,Yz,j,      alpha,alphaz,0, yx,yz,0) ;
                MULTADD (Yx,Yz,j+dy,   alpha,alphaz,0, yx,yz,1) ;
                MULTADD (Yx,Yz,j+2*dy, alpha,alphaz,0, yx,yz,2) ;

            }
            // y += 3*dy
            // x += 3*dx
            ADVANCE (Yx,Yz,3*dy) ;
            ADVANCE (Xx,Xz,3*dx) ;

            k += 3 ;
        }

        // copy four columns of X into W, and put in row form
        for ( ; k < kcol ; k += 4)
        {

            for (Int j = 0 ; j < ncol ; j++)
            {
                // w [4*j  ] = x [j     ]
                // w [4*j+1] = x [j+  dx]
                // w [4*j+2] = x [j+2*dx]
                // w [4*j+3] = x [j+3*dx]
                ASSIGN (w,Wz,4*j  , Xx,Xz,j     ) ;
                ASSIGN (w,Wz,4*j+1, Xx,Xz,j+dx  ) ;
                ASSIGN (w,Wz,4*j+2, Xx,Xz,j+2*dx) ;
                ASSIGN (w,Wz,4*j+3, Xx,Xz,j+3*dx) ;
            }

            for (Int j = 0 ; j < ncol ; j++)
            {
                // yj0 = 0
                // yj1 = 0
                // yj2 = 0
                // yj3 = 0
                CLEAR (yx,yz,0) ;
                CLEAR (yx,yz,1) ;
                CLEAR (yx,yz,2) ;
                CLEAR (yx,yz,3) ;

                // xj0 = alpha [0] * w [4*j  ]
                // xj1 = alpha [0] * w [4*j+1]
                // xj2 = alpha [0] * w [4*j+2]
                // xj3 = alpha [0] * w [4*j+3]
                MULT (xx,xz,0, alpha,alphaz,0, w,Wz,4*j) ;
                MULT (xx,xz,1, alpha,alphaz,0, w,Wz,4*j+1) ;
                MULT (xx,xz,2, alpha,alphaz,0, w,Wz,4*j+2) ;
                MULT (xx,xz,3, alpha,alphaz,0, w,Wz,4*j+3) ;

                Int p = Ap [j] ;
                Int pend = (packed) ? (Ap [j+1]) : (p + Anz [j]) ;
                for ( ; p < pend ; p++)
                {
                    Int i = Ai [p] ;
                    if (i == j)
                    {
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i     ] += aij * xj0
                        // y [i+  dy] += aij * xj1
                        // y [i+2*dy] += aij * xj2
                        // y [i+3*dy] += aij * xj3
                        MULTADD (Yx,Yz,i     , ax,az,0, xx,xz,0) ;
                        MULTADD (Yx,Yz,i+dy  , ax,az,0, xx,xz,1) ;
                        MULTADD (Yx,Yz,i+2*dy, ax,az,0, xx,xz,2) ;
                        MULTADD (Yx,Yz,i+3*dy, ax,az,0, xx,xz,3) ;

                    }
                    else if ((A->stype > 0 && i < j) || (A->stype < 0 && i > j))
                    {
                        // aij = Ax [p]
                        ASSIGN (ax,az,0, Ax,Az,p) ;

                        // y [i     ] += aij * xj0
                        // y [i+  dy] += aij * xj1
                        // y [i+2*dy] += aij * xj2
                        // y [i+3*dy] += aij * xj3
                        // yj0 += aij * w [4*i  ]
                        // yj1 += aij * w [4*i+1]
                        // yj2 += aij * w [4*i+2]
                        // yj3 += aij * w [4*i+3]
                        MULTADD     (Yx,Yz,i,      ax,az,0, xx,xz,0) ;
                        MULTADD     (Yx,Yz,i+dy,   ax,az,0, xx,xz,1) ;
                        MULTADD     (Yx,Yz,i+2*dy, ax,az,0, xx,xz,2) ;
                        MULTADD     (Yx,Yz,i+3*dy, ax,az,0, xx,xz,3) ;
                        MULTADDCONJ (yx,yz,0,     ax,az,0, w,Wz,4*i) ;
                        MULTADDCONJ (yx,yz,1,     ax,az,0, w,Wz,4*i+1) ;
                        MULTADDCONJ (yx,yz,2,     ax,az,0, w,Wz,4*i+2) ;
                        MULTADDCONJ (yx,yz,3,     ax,az,0, w,Wz,4*i+3) ;

                    }
                }
                // y [j     ] += alpha [0] * yj0
                // y [j+  dy] += alpha [0] * yj1
                // y [j+2*dy] += alpha [0] * yj2
                // y [j+3*dy] += alpha [0] * yj3
                MULTADD (Yx,Yz,j     , alpha,alphaz,0, yx,yz,0) ;
                MULTADD (Yx,Yz,j+dy  , alpha,alphaz,0, yx,yz,1) ;
                MULTADD (Yx,Yz,j+2*dy, alpha,alphaz,0, yx,yz,2) ;
                MULTADD (Yx,Yz,j+3*dy, alpha,alphaz,0, yx,yz,3) ;

            }
            // y += 4*dy
            // x += 4*dx
            ADVANCE (Yx,Yz,4*dy) ;
            ADVANCE (Xx,Xz,4*dx) ;
        }
    }
}

#undef PATTERN
#undef REAL
#undef COMPLEX
#undef ZOMPLEX

#undef ADVANCE

