//------------------------------------------------------------------------------
// SPEX/Include/SPEX.h: Include file for SPEX Library
//------------------------------------------------------------------------------

// SPEX: (c) 2019-2024, Christopher Lourenco, Jinhao Chen,
// Lorena Mejia Domenzain, Erick Moreno-Centeno, and Timothy A. Davis.
// All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0-or-later or LGPL-3.0-or-later

//------------------------------------------------------------------------------

#ifndef SPEX_H
#define SPEX_H


// SPEX is a collection of functions for the SParse EXact package.
// Included are several routines for memory management, matrix operations, and
// wrappers to the GMP library.
//
// This is the global include file and should be included in all SPEX_* packages
//
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------Authors----------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Unless otherwise noted all functions are authored by:
//
//    Christopher Lourenco, Jinhao Chen,
//    Lorena Mejia Domenzain, Erick Moreno-Centeno, and Timothy A. Davis
//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------Contact Information----------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//                            Please contact
//    Chris Lourenco (chrisjlourenco@gmail.com, lourenco@usna.edu)
//                                  or
//    Tim Davis (timdavis@aldenmath.com, DrTimothyAldenDavis@gmail.com,
//                  davis@tamu.edu)

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------Copyright--------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//    SPEX is free software; you can redistribute it and/or modify
//     it under the terms of either:
//
//        * the GNU Lesser General Public License as published by the
//          Free Software Foundation; either version 3 of the License,
//          or (at your option) any later version.
//
//     or
//
//        * the GNU General Public License as published by the Free Software
//          Foundation; either version 2 of the License, or (at your option) any
//          later version.
//
//    or both in parallel, as here.
//
//    See license.txt for license info.
//
// This software is copyright by Christopher Lourenco, Jinhao Chen,
// Lorena Mejia Domenzain, Erick Moreno-Centeno, and Timothy A. Davis.
// All Rights Reserved.
//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//---------------------------DISCLAIMER-----------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// SPEX is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//---------------------Include files required by SPEX --------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <mpfr.h>
// #include <math.h>
// #include <time.h>
// #include <inttypes.h>
// #include <assert.h>
#include "SuiteSparse_config.h"

//------------------------------------------------------------------------------
// SPEX Version
//------------------------------------------------------------------------------

// Current version of the code
#define SPEX_DATE "Feb 20, 2025"
#define SPEX_VERSION_STRING "3.2.3"
#define SPEX_VERSION_MAJOR 3
#define SPEX_VERSION_MINOR 2
#define SPEX_VERSION_SUB   3

#define SPEX_VERSION_NUMBER(major,minor,sub) \
    (((major)*1000ULL + (minor))*1000ULL + (sub))
#define SPEX_VERSION \
    SPEX_VERSION_NUMBER (SPEX_VERSION_MAJOR, \
                         SPEX_VERSION_MINOR, \
                         SPEX_VERSION_SUB)

#define SPEX__VERSION SUITESPARSE__VERCODE(3,2,3)
#if !defined (SUITESPARSE__VERSION) || \
    (SUITESPARSE__VERSION < SUITESPARSE__VERCODE(7,8,2))
#error "SPEX 3.2.3 requires SuiteSparse_config 7.8.2 or later"
#endif

#if defined ( __cplusplus )
extern "C"
{
#endif

//------------------------------------------------------------------------------
// Error codes
//------------------------------------------------------------------------------

// Most SPEX functions return a code that indicates if it was successful
// or not. Otherwise the code returns a pointer to the object that was created
// or it returns void (in the case that an object was deleted)

typedef enum
{

    SPEX_OK = 0,                  // all is well
    SPEX_OUT_OF_MEMORY = -1,      // out of memory
    SPEX_SINGULAR = -2,           // the input matrix A is singular
    SPEX_INCORRECT_INPUT = -3,    // one or more input arguments are incorrect
    SPEX_NOTSPD = -4,             // The input matrix is not symmetric positive
                                  // definite (for a Cholesky factorization)
    SPEX_INCORRECT_ALGORITHM = -5,// The algorithm is not compatible with
                                  // the factorization
    SPEX_PANIC = -6,              // SPEX used without proper initialization,
                                  // or other unrecoverable error
    SPEX_ZERODIAG = -7,           // The diagonal element is zero meaning that
                                  // an LDL factorization is not possible
    SPEX_UNSYMMETRIC = -8         // Matrix is not symmetric
}
SPEX_info ;

//------------------------------------------------------------------------------
// SPEX Version, continued
//------------------------------------------------------------------------------

SPEX_info SPEX_version
(
    int version [3],            // SPEX major, minor, and sub version
    char date [128]             // date of this version
) ;

// Requirements: SPEX requires GMP 6.1.2 or later, and MPFR 4.0.2 or later.
// NOTE that these version numbers are from the original source distributions.
// It is NOT the "number 10" assigned to libgmp.so.10 in the Ubuntu linux
// distro.

// GMP v6.1.2 or later is required:
#if __GNU_MP_RELEASE < 60102
#error "GMP v6.1.2 or later is required."
#endif

// MPFR v4.0.2 or later is required:
#if MPFR_VERSION < MPFR_VERSION_NUM(4,0,2)
#error "MPFR v4.0.2 or later is required."
#endif

//------------------------------------------------------------------------------
// SPEX_TRY: try a SPEX method and check for errors
//------------------------------------------------------------------------------

// In a robust application, the return values from each call to SPEX should be
// checked, and corrective action should be taken if an error occurs.  The
// SPEX_TRY macros assist in this effort.
//
// SPEX is written in C, and so it cannot rely on the try/catch mechanism of
// C++.  To accomplish a similar goal, we provide our mechanism.  The SPEX_TRY
// macro calls a single SPEX method and then takes corrected action based on a
// user-defined macro SPEX_CATCH.

#define SPEX_TRY(method)            \
{                                   \
    SPEX_info info = (method) ;     \
    if (info != SPEX_OK)            \
    {                               \
        SPEX_CATCH (info) ;         \
    }                               \
}

// A typical example user application might #define SPEX_CATCH as follows.
// Suppose the user function needs to free some workspace and return to the
// caller if an error occurs:

/*
        #define SPEX_CATCH(info)                                            \
        {                                                                   \
            SPEX_matrix_free (&A, NULL) ;                                   \
            fprintf (stderr, "SPEX failed: info %d, line %d, file %s\n",    \
                info, __LINE__, __FILE__) ;                                 \
            return (info) ;                                                 \
        }                                                                   \
*/

//------------------------------------------------------------------------------
// Pivot scheme codes
//------------------------------------------------------------------------------

// SPEX_DEFAULT is only used to define the defaults for the following enums but
// in all other places we use the appropiate default (ie SPEX_DEFAULT_ORDERING)
// for ease of reading
#define SPEX_DEFAULT 0

// A code in SPEX_options to tell SPEX what type of pivoting to use for pivoting
// in unsymmetric LU factorization.

typedef enum
{
    SPEX_SMALLEST = SPEX_DEFAULT,      // Smallest pivot (the default method)
    SPEX_DIAGONAL = 1,      // Diagonal pivoting
    SPEX_FIRST_NONZERO = 2, // First nonzero per column chosen as pivot
    SPEX_TOL_SMALLEST = 3,  // Diagonal pivoting with tol for smallest pivot.
    SPEX_TOL_LARGEST = 4,   // Diagonal pivoting with tol. for largest pivot
    SPEX_LARGEST = 5        // Largest pivot
}
SPEX_pivot ;

//------------------------------------------------------------------------------
// Fill-reducing ordering scheme codes
//------------------------------------------------------------------------------

// A code in SPEX_options to tell SPEX which fill-reducing ordering to used
// prior to exact factorization

typedef enum
{
    SPEX_DEFAULT_ORDERING = SPEX_DEFAULT,   // Default: colamd for LU
                            // AMD for Cholesky
    SPEX_NO_ORDERING = 1,   // None: A is factorized as-is
    SPEX_COLAMD = 2,        // COLAMD: Default for LU (and QR in the FUTURE)
    SPEX_AMD = 3            // AMD: Default for Cholesky
}
SPEX_preorder ;

//------------------------------------------------------------------------------
// Factorization type codes
//------------------------------------------------------------------------------

// A code in SPEX_options to tell SPEX which factorization algorithm to use
// By default, left-looking is used for LU and up-looking is utilized
// for Cholesky/LDL. If desired, left-looking can be used for all 
// Cholesky or LDL routines by setting option->algo to SPEX_CHOL_LEFT
// or SPEX_LDL_LEFT, respectively. 
// Witin SPEX Backslash, option->algo utilizes the specified algorithm
// if option->algo is set. Otherwise, if option->algo is left as 
// default, SPEX Backslash attempts an LDL factorization and, if that
// fails, an LU factorization is done.

typedef enum
{
    SPEX_ALGORITHM_DEFAULT = SPEX_DEFAULT,    // Defaults: Left for LU,
                         // Up for Chol, UP for LDL
    SPEX_LU_LEFT = 1,    // Left looking LU factorization
    SPEX_CHOL_LEFT = 2,  // Left looking Cholesky factorization
    SPEX_CHOL_UP = 3,    // Up looking Cholesky factorization
    SPEX_LDL_LEFT = 4,   // Left looking LDL factorization
    SPEX_LDL_UP = 5      // Up looking LDL factorization
}
SPEX_factorization_algorithm ;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------Data Structures--------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// This struct serves as a global struct to define all user-selectable options.

typedef struct
{
    SPEX_pivot pivot ;     // row pivoting scheme used (LU only)
    SPEX_preorder order ;  // ordering scheme used
    double tol ;           // tolerance for the row-pivoting methods for LU.
                           // SPEX_TOL_SMALLEST and SPEX_TOL_LARGEST
    int print_level ;      // 0: print nothing, 1: just errors,
                           // 2: terse (basic stats from COLAMD/AMD and the
                           // factorization), 3: all, with matrices and results
    uint64_t prec ;        // Precision for MPFR
    mpfr_rnd_t round ;     // Type of MPFR rounding used
    SPEX_factorization_algorithm algo ; // parameter which tells the function
                           // which factorization algorithm to use
} SPEX_options_struct ;

// A SPEX_options object is a pointer to a SPEX_options_struct
typedef SPEX_options_struct *SPEX_options ;

// Purpose: Create SPEX_options object with default parameters
// upon successful allocation, which are defined in SPEX_util_nternal.h
// To free it, simply use SPEX_FREE (option).

SPEX_info SPEX_create_default_options (SPEX_options *option_handle) ;


//------------------------------------------------------------------------------
// SPEX_vector
//------------------------------------------------------------------------------

// NOTE: The SPEX_vector object will be used in a near-future version of SPEX.
// It appears here for future compatibility, but is currently unused.

typedef struct
{
    int64_t nz;   // number of explicit entries in the vector
    int64_t nzmax;// size of array i and x, nz <= nzmax
    int64_t *i;   // array of size nzmax that contains the column/row indices
                  // of each nnz.
    mpz_t *x;     // array of size nzmax that contains the values of each nnz
    mpq_t scale;  // a scale factor that has not applied to entries in this v.
                  // The real value of the k-th nonzero entry in the list should
                  // be computed as x[k]*scale. x[k]/den(scale) must be integer.
} SPEX_vector_struct ;

// A SPEX_vector is a pointer to a SPEX_vector_struct
typedef SPEX_vector_struct *SPEX_vector ;


//------------------------------------------------------------------------------
// SPEX_matrix: a sparse CSC, sparse triplet, or dense matrix
//------------------------------------------------------------------------------

// SPEX uses a single matrix data type, SPEX_matrix, which can be held in
// one of four kinds of formats:  sparse CSC (compressed sparse column),
// sparse triplet, dense, and sparse dynamic CSC.

typedef enum
{
    SPEX_CSC = 0,           // matrix is in compressed sparse column format
    SPEX_TRIPLET = 1,       // matrix is in sparse triplet format
    SPEX_DENSE = 2,         // matrix is in dense format (held by column)
}
SPEX_kind ;

// The last format (SPEX_DYNAMIC_CSC) only support mpz_t type, while each of
// the first three formats can have values of 5 different data types: mpz_t,
// mpq_t, mpfr_t, int64_t, and double:

typedef enum
{
    SPEX_MPZ = 0,           // matrix of mpz_t integers
    SPEX_MPQ = 1,           // matrix of mpq_t rational numbers
    SPEX_MPFR = 2,          // matrix of mpfr_t
    SPEX_INT64 = 3,         // matrix of int64_t integers
    SPEX_FP64 = 4           // matrix of doubles
}
SPEX_type ;

// This gives a total of 16 different matrix types:
// (sparse CSC, triplet, dense) x (5 data types) = 15 formats,

// Not all functions accept all 16 matrices types, however.

// Suppose A is an m-by-n matrix with nz <= nzmax entries.
// The p, i, j, and x components are defined as:

// (0) SPEX_CSC:  A sparse matrix in CSC (compressed sparse column) format.
//      A->p is an int64_t array of size n+1, A->i is an int64_t array of size
//      nzmax (with nz <= nzmax), and A->x.type is an array of size nzmax of
//      matrix entries ('type' is one of mpz, mpq, mpfr, int64, or fp64).  The
//      row indices of column j appear in A->i [A->p [j] ... A->p [j+1]-1], and
//      the values appear in the same locations in A->x.type.  The A->j array
//      is NULL.  A->nz is ignored; nz is A->p [A->n].

// (1) SPEX_TRIPLET:  A sparse matrix in triplet format.  A->i and A->j are
//      both int64_t arrays of size nzmax, and A->x.type is an array of values
//      of the same size.  The kth tuple has row index A->i [k], column index
//      A->j [k], and value A->x.type [k], with 0 <= k < A->nz.  The A->p array
//      is NULL.

// (2) SPEX_DENSE:  A dense matrix.  The integer arrays A->p, A->i, and A->j
//      are all NULL.  A->x.type is a pointer to an array of size m*n, stored
//      in column-oriented format.  The value of A(i,j) is A->x.type [p]
//      with p = i + j*A->m.  A->nz is ignored; nz is A->m * A->n.
//
// The SPEX_matrix may contain 'shallow' components, A->p, A->i, A->j, and
// A->x.  For example, if A->p_shallow is true, then a non-NULL A->p is a
// pointer to a read-only array, and the A->p array is not freed by
// SPEX_matrix_free.  If A->p is NULL (for a triplet or dense matrix), then
// A->p_shallow has no effect.

typedef struct
{
    SPEX_kind kind ;    // CSC, triplet, dense
    SPEX_type type ;    // mpz, mpq, mpfr, int64, or fp64 (double)


    int64_t m ;         // number of rows
    int64_t n ;         // number of columns

    mpq_t scale ;       // scale factor for mpz matrices (never shallow)
                        // For all matrices whose type is not mpz,
                        // mpz_scale = 1.
                        // The real value of the nonzero entry A(i,j)
                        // should be computed as A(i,j)/scale.

    //--------------------------------------------------------------------------
    // these are used for CSC, triplet or dense matrix
    //--------------------------------------------------------------------------

    int64_t nzmax ;     // size of A->i, A->j, and A->x.
    int64_t nz ;        // # nonzeros in a triplet matrix .
                        // Ignored for CSC, or dense.

    int64_t *p ;        // if CSC: column pointers, an array size is n+1.
                        // if triplet or dense: A->p is NULL.

    int64_t *i ;        // if CSC or triplet: row indices, of size nzmax.
                        // if dense: A->i is NULL.


    int64_t *j ;        // if triplet: column indices, of size nzmax.
                        // if CSC or dense: A->j is NULL.

    union               // A->x.type has size nzmax.
    {
        mpz_t *mpz ;            // A->x.mpz
        mpq_t *mpq ;            // A->x.mpq
        mpfr_t *mpfr ;          // A->x.mpfr
        int64_t *int64 ;        // A->x.int64
        double *fp64 ;          // A->x.fp64
    } x ;

    //--------------------------------------------------------------------------
    // This component is only used for SPEX_DYNAMIC_CSC matrix, and ignored for
    // CSC, triplet and dense matrix, for a future version of SPEX.
    //--------------------------------------------------------------------------

    SPEX_vector *v;     // In this version of SPEX, v is always NULL, and
                        // should not be used.

    //--------------------------------------------------------------------------
    // flags to indicate if any component is shallow
    //--------------------------------------------------------------------------

    bool p_shallow ;    // if true, A->p is shallow.
    bool i_shallow ;    // if true, A->i is shallow.
    bool j_shallow ;    // if true, A->j is shallow.
    bool x_shallow ;    // if true, A->x.type is shallow.

} SPEX_matrix_struct ;

// A SPEX_matrix is a pointer to a SPEX_matrix_struct
typedef SPEX_matrix_struct *SPEX_matrix ;

//------------------------------------------------------------------------------
// SPEX_matrix macros
//------------------------------------------------------------------------------

// These macros simplify the access to entries in a SPEX_matrix.
// The type parameter is one of: mpq, mpz, mpfr, int64, or fp64.

// To access the kth entry in a SPEX_matrix using 1D linear addressing,
// in any matrix kind (CSC, triplet, or dense), in any type:
#define SPEX_1D(A,k,type) ((A)->x.type [k])

// To access the (i,j)th entry in a 2D dense SPEX_matrix, in any type:
#define SPEX_2D(A,i,j,type) SPEX_1D (A, (i)+(j)*((A)->m), type)

//------------------------------------------------------------------------------
// SPEX_matrix_allocate: allocate an m-by-n SPEX_matrix
//------------------------------------------------------------------------------

// Allocate an m-by-n SPEX_matrix, in one of 15 data structures:
// (sparse CSC, sparse triplet, or dense) x
// (mpz, mpz, mfpr, int64, or double).

// The matrix may be created as 'shallow', in
// which case A->p, A->i, A->j, and A->x are all returned as NULL, and all
// A->*_shallow flags are returned as true.  The user can then set A->p, A->i,
// A->j, and/or A->x accordingly, from their own arrays. For non-shallow
// matrix, the components (p,i,j,x) are allocated according to the kind, type
// and size (m, n, nzmax) of the matrix.


// if shallow is false: All components (p,i,j,x) are allocated and set to zero,
//                      and then shallow flags are all false.

// if shallow is true:  All components (p,i,j,x) are NULL, and their shallow
//                      flags are all true.

SPEX_info SPEX_matrix_allocate
(
    SPEX_matrix *A_handle,  // matrix to allocate
    SPEX_kind kind,         // CSC, triplet, dense (and a future dynamic CSC)
    SPEX_type type,         // mpz, mpq, mpfr, int64, or double
    int64_t m,              // # of rows
    int64_t n,              // # of columns
    int64_t nzmax,          // max # of entries for CSC or triplet
                            // (ignored if A is dense)
    bool shallow,           // if true, matrix is shallow.  A->p, A->i, A->j,
                            // A->x are all returned as NULL and must be set
                            // by the caller.  All A->*_shallow are returned
                            // as true. Ignored if kind is dynamic_CSC.
    bool init,              // If true, and the data types are mpz, mpq, or
                            // mpfr, the entries are initialized (using the
                            // appropriate SPEX_mp*_init function). If false,
                            // the mpz, mpq, and mpfr arrays are malloced but
                            // not initialized. Utilized internally to reduce
                            // memory.  Ignored if shallow is true.
    const SPEX_options option
) ;

//------------------------------------------------------------------------------
// SPEX_matrix_free: free a SPEX_matrix
//------------------------------------------------------------------------------

SPEX_info SPEX_matrix_free
(
    SPEX_matrix *A_handle, // matrix to free
    const SPEX_options option
) ;

//------------------------------------------------------------------------------
// SPEX_matrix_nnz: # of entries in a matrix
//------------------------------------------------------------------------------

SPEX_info SPEX_matrix_nnz       // find the # of entries in A
(
    int64_t *nnz,               // # of entries in A, -1 if A is NULL
    const SPEX_matrix A,        // matrix to query
    const SPEX_options option   // command options, currently unused
) ;

//------------------------------------------------------------------------------
// SPEX_matrix_check: check and print a SPEX_matrix
//------------------------------------------------------------------------------

SPEX_info SPEX_matrix_check     // returns a SPEX status code
(
    const SPEX_matrix A,        // matrix to check
    const SPEX_options option
) ;

//------------------------------------------------------------------------------
// SPEX_matrix_copy: makes a copy of a matrix
//------------------------------------------------------------------------------

// SPEX_matrix_copy: make a copy of a SPEX_matrix, into another kind and type.

// SPEX supports 16 matrix formats:  15 of them are all combinations of
// (CSC, triplet, dense) x (mpz, mpq, mpfr, int64, double).

SPEX_info SPEX_matrix_copy
(
    SPEX_matrix *C_handle,  // matrix to create (never shallow)
    // inputs, not modified:
    SPEX_kind C_kind,       // C->kind: CSC, triplet, dense,
                            // (or future dynamic CSC)
    SPEX_type C_type,       // C->type: mpz_t, mpq_t, mpfr_t, int64_t, or double
    const SPEX_matrix A,    // matrix to make a copy of (may be shallow)
    const SPEX_options option
) ;

//------------------------------------------------------------------------------
// SPEX symbolic analysis and factorization
//------------------------------------------------------------------------------

typedef enum
{
    SPEX_LU_FACTORIZATION = 0,            // LU factorization
    SPEX_CHOLESKY_FACTORIZATION = 1,      // Cholesky factorization
    SPEX_LDL_FACTORIZATION = 2,           // LDL factorization 
    SPEX_QR_FACTORIZATION = 3             // QR factorization (FUTURE)    
}
SPEX_factorization_kind ;

//------------------------------------------------------------------------------
// SPEX_symbolic_analysis: symbolic pre-analysis
//------------------------------------------------------------------------------

// This struct stores the results of symbolic analysis

// This object is constructed by SPEX_lu_analyze and SPEX_cholesky_analyze.
// All these functions allocate space and assign values, and thus do not
// require user to perform any memory allocation. Certain components of this
// object can still be NULL after it is constructed. User can access (read or
// print) components of this object, but should not try to modify any of them
// other than calling SPEX_symbolic_analysis_free to free the memory space.

typedef struct
{
    SPEX_factorization_kind kind;    // LU, Cholesky, LDL (or QR in the FUTURE)

    //--------------------------------------------------------------------------
    // The permutations of the matrix that are found during the symbolic
    // analysis process.  One or more of these permutations could be NULL for
    // some SPEX_symbolic_analysis_kind. Specifically,
    // For kind == SPEX_LU_FACTORIZATION, only Q_perm is not NULL.
    // For kind == SPEX_CHOLESKY_FACTORIZATION, both Q_perm and Qinv_perm are
    // NULL.
    //--------------------------------------------------------------------------
    int64_t *P_perm;                // row permutation
    int64_t *Pinv_perm;             // inverse of row permutation

    int64_t *Q_perm;                // column permutation
    int64_t *Qinv_perm;             // inverse of column permutation

    //--------------------------------------------------------------------------
    // estimates of nonzeros that will apprear in the factorization
    //--------------------------------------------------------------------------

    int64_t lnz ;                   // Approximate number of nonzeros in L.
                                    // Available only for SPEX_LU_FACTORIZATION
                                    // or SPEX_CHOLESKY_FACTORIZATION.
    int64_t unz ;                   // Approximate number of nonzeros in U.
                                    // lnz and unz are used to allocate
                                    // the initial space for L and U; the
                                    // space is reallocated as needed.
                                    // Available only for SPEX_LU_FACTORIZATION.

    //--------------------------------------------------------------------------
    // These are only used in the Cholesky analysis process
    //--------------------------------------------------------------------------
    int64_t *parent;                // Elimination tree of target matrix
                                    // for Cholesky factorization.
    int64_t *cp;                    // column pointers of L for Cholesky
                                    // factorization.

} SPEX_symbolic_analysis_struct ;

// A SPEX_symbolic_analysis object is a pointer to a
// SPEX_symbolic_analysis_struct
typedef SPEX_symbolic_analysis_struct *SPEX_symbolic_analysis ;

//------------------------------------------------------------------------------
// SPEX_symbolic_analysis_free frees the SPEX_symbolic_analysis object.
//------------------------------------------------------------------------------

SPEX_info SPEX_symbolic_analysis_free
(
    SPEX_symbolic_analysis *S_handle,   // Structure to be deleted
    const SPEX_options option
) ;

//------------------------------------------------------------------------------
// SPEX_factorization: data structure for factorization
//------------------------------------------------------------------------------

// The SPEX_factorization object holds an LU, Cholesky, or (in the future) QR
// numerical factorization, in either non-updatable (static) or updatable form
// (also future work).
//
// NOTE:
// The components of the factorization structure are accessible to the user
// application.  However, they should only be modified by calling SPEX_*
// methods.  Changing them directly can lead to undefined behavior.

// To create this object, users can call SPEX_lu_factorize, or
// SPEX_cholesky_factorize. All these function will create a
// static factorization of corresponding kind.
//
// To free the factorization object, simply call SPEX_factorization_free.

typedef struct
{
    SPEX_factorization_kind kind;         // LU, Cholesky, LDL, QR factorization

    bool updatable;                       // flag to denote if the factorization
                                          // is in the updatable format
                                          // (for a future SPEX version)

    mpq_t scale_for_A;                    // the scale of the target matrix

    //--------------------------------------------------------------------------
    // These are used for LU, Cholesky or LDL factorization, but ignored for QR
    // factorization.
    //--------------------------------------------------------------------------

    SPEX_matrix L;                        // The lower-triangular matrix from LU
                                          // or Cholesky factorization.
    SPEX_matrix U;                        // The upper-triangular matrix from LU
                                          // factorization. NULL for Cholesky
                                          // factorization.
    SPEX_matrix rhos;                     // A n-by-1 dense matrix for the
                                          // pivot values


    //--------------------------------------------------------------------------
    // The permutations of the matrix that are used during the factorization.
    // These are currently used only for LU, Cholesky or LDL factorization.
    // One or more of these permutations could be NULL for some
    // SPEX_factorization_kind. Specifically,
    // For kind == SPEX_LU_FACTORIZATION, Qinv_perm can be NULL
    // For kind == SPEX_CHOLESKY_FACTORIZATION, and SPEX_LDL_FACTORIZATION
    // both Q_perm and Qinv_perm are NULL.
    //--------------------------------------------------------------------------

    int64_t *P_perm;                     // row permutation
    int64_t *Pinv_perm;                  // inverse of row permutation

    int64_t *Q_perm;                     // column permutation
    int64_t *Qinv_perm;                  // inverse of column permutation

} SPEX_factorization_struct ;

// A SPEX_factorization is a pointer to a SPEX_factorization_struct
typedef SPEX_factorization_struct *SPEX_factorization ;

//------------------------------------------------------------------------------
// SPEX_factorization_free frees the SPEX_factorization object.
//------------------------------------------------------------------------------

SPEX_info SPEX_factorization_free
(
    SPEX_factorization *F_handle,   // Structure to be deleted
    const SPEX_options option
) ;

//------------------------------------------------------------------------------
// Memory management
//------------------------------------------------------------------------------

// SPEX relies on the SuiteSparse memory management functions,
// SuiteSparse_malloc, SuiteSparse_calloc, SuiteSparse_realloc, and
// SuiteSparse_free.

// Allocate and initialize memory space for SPEX

void *SPEX_calloc
(
    size_t nitems,      // number of items to allocate
    size_t size         // size of each item
) ;

// Allocate memory space for SPEX

void *SPEX_malloc
(
    size_t size        // size of memory space to allocate
) ;

// Free the memory allocated by SPEX_calloc, SPEX_malloc, or SPEX_realloc.

void SPEX_free
(
    void *p         // pointer to memory space to free
) ;

// Free a pointer and set it to NULL.
#define SPEX_FREE(p)                        \
{                                           \
    SPEX_free (p) ;                         \
    (p) = NULL ;                            \
}

// SPEX_realloc is a wrapper for realloc.  If p is non-NULL on input, it points
// to a previously allocated object of size old_size * size_of_item.  The
// object is reallocated to be of size new_size * size_of_item.  If p is NULL
// on input, then a new object of that size is allocated.  On success, a
// pointer to the new object is returned.  If the reallocation fails, p is not
// modified, and a flag is returned to indicate that the reallocation failed.
// If the size decreases or remains the same, then the method always succeeds
// (ok is returned as true).

// Typical usage:  the following code fragment allocates an array of 10 int's,
// and then increases the size of the array to 20 int's.  If the SPEX_malloc
// succeeds but the SPEX_realloc fails, then the array remains unmodified,
// of size 10.
//
//      int *p ;
//      p = SPEX_malloc (10 * sizeof (int)) ;
//      if (p == NULL) { error here ... }
//      printf ("p points to an array of size 10 * sizeof (int)\n") ;
//      bool ok ;
//      p = SPEX_realloc (20, 10, sizeof (int), p, &ok) ;
//      if (ok) printf ("p has size 20 * sizeof (int)\n") ;
//      else printf ("realloc failed; p still has size 10 * sizeof (int)\n") ;
//      SPEX_free (p) ;

void *SPEX_realloc      // pointer to reallocated block, or original block
                        // if the realloc failed
(
    int64_t nitems_new,     // new number of items in the object
    int64_t nitems_old,     // old number of items in the object
    size_t size_of_item,    // sizeof each item
    void *p,                // old object to reallocate
    bool *ok                // true if success, false on failure
) ;

//------------------------------------------------------------------------------
// SPEX environment routines
//------------------------------------------------------------------------------

// SPEX_initialize: initializes the working evironment for SPEX library.
// It must be called prior to calling any other SPEX_* function.

SPEX_info SPEX_initialize ( void ) ;

// SPEX_initialize_expert is the same as SPEX_initialize, except that it allows
// for a redefinition of custom memory functions that are used for SPEX and
// GMP.  The four inputs to this function are pointers to four functions with
// the same signatures as the ANSI C malloc, calloc, realloc, and free.

SPEX_info SPEX_initialize_expert
(
    void *(*MyMalloc) (size_t),             // user-defined malloc
    void *(*MyCalloc) (size_t, size_t),     // user-defined calloc
    void *(*MyRealloc) (void *, size_t),    // user-defined realloc
    void  (*MyFree) (void *)                // user-defined free
) ;

// SPEX_finalize: This function finalizes the working evironment for SPEX
// library, and frees any internal workspace created by SPEX.  It must be
// called as the last SPEX_* function called.

SPEX_info SPEX_finalize
(
    void
) ;

// SPEX is thread-safe but it requires each user thread to call
// SPEX_thread_initialize when it starts, and SPEX_thread_finalize when it
// finishes.  These two functions must be called after the user's primary thread
// calls SPEX_initialize (or SPEX_initialize_experm) and before the user's
// primary thread calls SPEX_finalize.

SPEX_info SPEX_thread_initialize ( void ) ;

SPEX_info SPEX_thread_finalize ( void ) ;

//------------------------------------------------------------------------------
// SPEX matrix utilities
//------------------------------------------------------------------------------

// Purpose: This function sets C = A', where A must be a SPEX_CSC matrix
// C_handle is NULL on input. On output, C_handle contains a pointer to A'

SPEX_info SPEX_transpose
(
    SPEX_matrix *C_handle,      // C = A'
    SPEX_matrix A,              // Matrix to be transposed
    const SPEX_options option
) ;

// Purpose: Determine if the input A is symmetric.  Since SPEX is an exact
// framework, the method checks if the matrix is symmetric both numerically
// and in its symbolic pattern.  The method has no option for checking just
// pattern symmetry.

SPEX_info SPEX_determine_symmetry
(
    bool *is_symmetric,         // true if matrix is symmetric, false otherwise
    const SPEX_matrix A,        // Input matrix to be checked for symmetry
    const SPEX_options option   // Command options
) ;

//------------------------------------------------------------------------------
//---------------------------SPEX GMP/MPFR Functions----------------------------
//------------------------------------------------------------------------------

// The following functions are the SPEX interface to the GMP/MPFR libary.
// Each corresponding GMP/MPFR function is given a wrapper to ensure that no
// memory leaks or crashes occur. All covered GMP functions can be found in
// SPEX_gmp.c

// The GMP library does not handle out-of-memory failures.  However, it does
// provide a mechanism for passing function pointers that replace GMP's use of
// malloc, realloc, and free.  This mechanism is used to provide a try/catch
// mechanism for memory allocation errors, using setjmp and longjmp.

// When a GMP function is called, this wrapper keeps track of a list of objects
// allocated by that function.  The list is started fresh each time a GMP
// function is called.  If any allocation fails, the NULL pointer is not
// returned to GMP.  Instead, all allocated blocks in the list are freed,
// and the allocation routine passed to GMP returns directly to the wrapper.

SPEX_info SPEX_mpfr_asprintf (char **str, const char *format, ... ) ;
SPEX_info SPEX_mpfr_clear (mpfr_t x) ;
SPEX_info SPEX_mpfr_div_d (mpfr_t x, const mpfr_t y, const double z,
                    const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_free_cache (void) ;
SPEX_info SPEX_mpfr_free_str (char *str) ;
SPEX_info SPEX_mpfr_get_d (double *x, const mpfr_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_get_si (int64_t *x, const mpfr_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_get_q (mpq_t x, const mpfr_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_get_z (mpz_t x, const mpfr_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_init2(mpfr_t x, const uint64_t size) ;
SPEX_info SPEX_mpfr_mul (mpfr_t x, const mpfr_t y, const mpfr_t z,
                    const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_mul_d (mpfr_t x, const mpfr_t y, const double z,
                    const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_set (mpfr_t x, const mpfr_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_set_d (mpfr_t x, const double y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_set_null (mpfr_t x) ;
SPEX_info SPEX_mpfr_set_prec(mpfr_t x, const uint64_t size) ;
SPEX_info SPEX_mpfr_set_q (mpfr_t x, const mpq_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_set_si (mpfr_t x, int64_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_set_z (mpfr_t x, const mpz_t y, const mpfr_rnd_t rnd) ;
SPEX_info SPEX_mpfr_sgn (int *sgn, const mpfr_t x) ;
SPEX_info SPEX_mpfr_ui_pow_ui (mpfr_t x, const uint64_t y, const uint64_t z,
                    const mpfr_rnd_t rnd) ;

SPEX_info SPEX_gmp_fscanf (FILE *fp, const char *format, ... ) ;

SPEX_info SPEX_mpq_abs (mpq_t x, const mpq_t y) ;
SPEX_info SPEX_mpq_add (mpq_t x, const mpq_t y, const mpq_t z) ;
SPEX_info SPEX_mpq_canonicalize (mpq_t x);
SPEX_info SPEX_mpq_clear (mpq_t x) ;
SPEX_info SPEX_mpq_cmp (int *r, const mpq_t x, const mpq_t y) ;
SPEX_info SPEX_mpq_cmp_ui (int *r, const mpq_t x,
                    const uint64_t num, const uint64_t den) ;
SPEX_info SPEX_mpq_div (mpq_t x, const mpq_t y, const mpq_t z) ;
SPEX_info SPEX_mpq_equal (int *r, const mpq_t x, const mpq_t y) ;
SPEX_info SPEX_mpq_get_d (double *x, const mpq_t y) ;
SPEX_info SPEX_mpq_init (mpq_t x) ;
SPEX_info SPEX_mpq_mul (mpq_t x, const mpq_t y, const mpq_t z) ;
SPEX_info SPEX_mpq_neg (mpq_t x, const mpq_t y) ;
SPEX_info SPEX_mpq_set (mpq_t x, const mpq_t y) ;
SPEX_info SPEX_mpq_set_d (mpq_t x, const double y) ;
SPEX_info SPEX_mpq_set_den (mpq_t x, const mpz_t y) ;
SPEX_info SPEX_mpq_set_null (mpq_t x) ;
SPEX_info SPEX_mpq_set_num (mpq_t x, const mpz_t y) ;
SPEX_info SPEX_mpq_set_si (mpq_t x, const int64_t y, const uint64_t z) ;
SPEX_info SPEX_mpq_set_ui (mpq_t x, const uint64_t y, const uint64_t z) ;
SPEX_info SPEX_mpq_set_z (mpq_t x, const mpz_t y) ;
SPEX_info SPEX_mpq_sgn (int *sgn, const mpq_t x) ;

SPEX_info SPEX_mpz_abs (mpz_t x, const mpz_t y) ;
SPEX_info SPEX_mpz_add (mpz_t a, const mpz_t b, const mpz_t c) ;
SPEX_info SPEX_mpz_addmul (mpz_t x, const mpz_t y, const mpz_t z) ;
SPEX_info SPEX_mpz_cdiv_q (mpz_t q, const mpz_t n, const mpz_t d) ;
SPEX_info SPEX_mpz_cdiv_qr (mpz_t q, mpz_t r, const mpz_t n, const mpz_t d) ;
SPEX_info SPEX_mpz_clear (mpz_t x) ;
SPEX_info SPEX_mpz_cmp (int *r, const mpz_t x, const mpz_t y) ;
SPEX_info SPEX_mpz_cmp_ui (int *r, const mpz_t x, const uint64_t y) ;
SPEX_info SPEX_mpz_cmpabs (int *r, const mpz_t x, const mpz_t y) ;
SPEX_info SPEX_mpz_cmpabs_ui (int *r, const mpz_t x, const uint64_t y) ;
SPEX_info SPEX_mpz_divexact (mpz_t x, const mpz_t y, const mpz_t z) ;
SPEX_info SPEX_mpz_fdiv_q (mpz_t q, const mpz_t n, const mpz_t d) ;
SPEX_info SPEX_mpz_gcd (mpz_t x, const mpz_t y, const mpz_t z) ;
SPEX_info SPEX_mpz_get_d (double *x, const mpz_t y) ;
SPEX_info SPEX_mpz_get_si (int64_t *x, const mpz_t y) ;
SPEX_info SPEX_mpz_init (mpz_t x) ;
SPEX_info SPEX_mpz_init2(mpz_t x, const uint64_t size) ;
SPEX_info SPEX_mpz_lcm (mpz_t lcm, const mpz_t x, const mpz_t y) ;
SPEX_info SPEX_mpz_mul (mpz_t a, const mpz_t b, const mpz_t c) ;
SPEX_info SPEX_mpz_mul_si (mpz_t a, const mpz_t b, const int64_t c) ;
SPEX_info SPEX_mpz_neg (mpz_t x, const mpz_t y) ;
SPEX_info SPEX_mpz_set (mpz_t x, const mpz_t y) ;
SPEX_info SPEX_mpz_set_null (mpz_t x) ;
SPEX_info SPEX_mpz_set_si (mpz_t x, const int64_t y) ;
SPEX_info SPEX_mpz_set_ui (mpz_t x, const uint64_t y) ;
SPEX_info SPEX_mpz_sgn (int *sgn, const mpz_t x) ;
SPEX_info SPEX_mpz_sizeinbase (size_t *size, const mpz_t x, int64_t base) ;
SPEX_info SPEX_mpz_sub (mpz_t a, const mpz_t b, const mpz_t c) ;
SPEX_info SPEX_mpz_submul (mpz_t x, const mpz_t y, const mpz_t z) ;
SPEX_info SPEX_mpz_swap (mpz_t x, mpz_t y);



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------SPEX Left LU-----------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// This portion of the SPEX library solves a sparse system of linear equations
// using the SPEX Left LU factorization. This code accompanies the paper
// (submitted to ACM Transactions on Mathematical Software):

//    "Algorithm 1021: SPEX Left LU, Exactly Solving Sparse Linear Systems via
//    a Sparse Left-looking Integer-preserving LU Factorization",
//    C. Lourenco, J. Chen, E. Moreno-Centeno, T. Davis,
//    ACM Trans. Mathematical Software. pp 1-23, vol 48, no 2, 2022.

//    The theory associated with this software can be found in the paper
//    (published in SIAM journal on matrix analysis and applications):

//    "Exact Solution of Sparse Linear Systems via Left-Looking
//     Roundoff-Error-Free LU Factorization in Time Proportional to
//     Arithmetic Work", C. Lourenco, A. R. Escobedo, E. Moreno-Centeno,
//     T. Davis, SIAM J. Matrix Analysis and Applications. pp 609-638,
//     vol 40, no 2, 2019.

//    If you use this code, you must first download and install the GMP and
//    MPFR libraries. GMP and MPFR can be found at:
//              https://gmplib.org/
//              http://www.mpfr.org/

//    If you use SPEX Left LU for a publication, we request that you please cite
//    the above two papers.

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------Authors----------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// Christopher Lourenco, Jinhao Chen, Erick Moreno-Centeno, and Timothy A. Davis


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//--------------------------Summary---------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//    This software package solves the linear system Ax = b exactly. The input
//    matrix and right hand side vectors are stored as either integers, double
//    precision numbers, multiple precision floating points (through the mpfr
//    library) or as rational numbers (as a collection of numerators and
//    denominators using the GMP mpq_t data structure). Appropriate routines
//    within the code transform the input into an integral matrix in compressed
//    column form.

//    This package computes the factorization PAQ = LDU. Note that we store the
//    "functional" form of the factorization by only storing L and U. The user
//    is given some freedom to select the permutation matrices P and Q. The
//    recommended default settings select Q using the COLAMD column ordering
//    and select P via a partial pivoting scheme in which the diagonal entry
//    in column k is selected if it is the same magnitude as the smallest
//    entry, otherwise the smallest entry is selected as the kth pivot.
//    Alternative strategies allowed to select Q include the AMD column
//    ordering or no column permutation (Q=I).  For pivots, there are a variety
//    of potential schemes including traditional partial pivoting, diagonal
//    pivoting, tolerance pivoting etc. This package does not allow pivoting
//    based on sparsity criterion.

//    The factors L and U are computed via integer preserving operations via
//    integer-preserving Gaussian elimination. The key part of this algorithm
//    is a Roundoff Error Free (REF) sparse triangular solve function which
//    exploits sparsity to reduce the number of operations that must be
//    performed.

//    Once L and U are computed, a simplified version of the triangular solve
//    is performed which assumes the vector b is dense. The final solution
//    vector x is gauranteed to be exact. This vector can be output in one of
//    three ways: 1) full precision rational arithmetic (as a sequence of
//    numerators and denominators) using the GMP mpq_t data type, 2) double
//    precision while not exact will produce a solution accurate to machine
//    roundoff unless the size of the associated solution exceeds double
//    precision (i.e., the solution is 10^500 or something), 3) variable
//    precision floating point using the GMP mpfr_t data type. The associated
//    precision is user defined.



//------------------------------------------------------------------------------
// Primary factorization & solve routines
//------------------------------------------------------------------------------

// SPEX_lu_backslash solves the linear system Ax = b via LU factorization
// of A. This is the simplest way to use the SPEX Left LU package. This
// function encompasses both factorization and solve and returns the solution
// vector in the user desired type.  It can be thought of as an exact version
// of MATLAB sparse backslash.
// x and b be can be single vectors, or matrices.

SPEX_info SPEX_lu_backslash
(
    // Output
    SPEX_matrix *x_handle,        // Final solution vector
    // Input
    SPEX_type type,               // Type of output desired. Must be
                                  // SPEX_MPQ, SPEX_MPFR, or SPEX_FP64
    const SPEX_matrix A,          // Input matrix
    const SPEX_matrix b,          // Right hand side vector(s)
    const SPEX_options option     // Command options
) ;

SPEX_info SPEX_lu_analyze
(
    SPEX_symbolic_analysis *S_handle,   // symbolic analysis including
                                 // column permutation and nnz of L and U
    const SPEX_matrix A,         // Input matrix
    const SPEX_options option    // Control parameters, if NULL, use default
) ;

SPEX_info SPEX_lu_factorize
(
    // output:
    SPEX_factorization *F_handle,   // LU factorization
    // input:
    const SPEX_matrix A,            // matrix to be factored
    const SPEX_symbolic_analysis S, // symbolic analysis
    const SPEX_options option       // command options
) ;

// solves the linear system Ax = b via LU factorization.
// x and b be can be single vectors, or matrices.

SPEX_info SPEX_lu_solve     // solves the linear system LD^(-1)U x = b
(
    // Output
    SPEX_matrix *x_handle,  // rational solution to the system
    // input/output:
    SPEX_factorization F,   // The LU factorization.
                            // Mathematically, F is unchanged.
    // input:
    const SPEX_matrix b,    // right hand side vector(s)
    const SPEX_options option // Command options
) ;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------SPEX Cholesky----------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// This portion of the SPEX library exactly solves a sparse symmetric positive
// definite (SPD) system of linear equations using one of two Integer-
// Preserving Cholesky factorizations. This code accompanies the paper (to be
// submitted to ACM TOMs)

//    "Algorithm 1xxx: SPEX Cholesky and SPEX Backslash for Exactly Solving
//     Sparse Linear Systems," L. Mejia Domenzain, J. Chen, C. Lourenco, 
//     E. Moreno-Centeno, T. Davis, submitted to ACM TOMS


//     The theory associated with this paper is found at:

//    "Exactly Solving Sparse Rational Linear Systems via Roundoff-Error-Free
//    Cholesky Factorizations", C. Lourenco, E. Moreno-Centeno,
//    SIAM J. Matrix Analysis and Applications.
//     pp 609-638, vol 43, no 1, 2022.

//    To use this code you must first download and install the GMP,
//    MPFR, AMD, and COLAMD libraries. GMP and MPFR can be found at:
//              https://gmplib.org/
//              http://www.mpfr.org/
//
//   SPEX_Utilities, AMD, and COLAMD are distributed along with SPEX_Cholesky.
//   The easiest way ensure these dependencies are met is to only access this
//   package through the SPEX repository.
//
//   All of these codes are components of the SPEX software library. This code
//   may be found at:
//              https://github.com/clouren/spex
//              www.suitesparse.com
//
//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------Authors----------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//    Christopher Lourenco, Jinhao Chen,
//    Lorena Mejia Domenzain, Erick Moreno-Centeno, and Timothy A. Davis.


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//--------------------------Summary---------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//    This software package solves the SPD linear system Ax = b exactly. The
//    key property of this package is that it can exactly solve any SPD input
//    system.  The input matrix and right hand side vectors are stored as
//    either integers, double precision numbers, multiple precision floating
//    points (through the mpfr library) or as rational numbers (as a collection
//    of numerators and denominators using the GMP mpq_t data structure).
//    Appropriate routines within the code transform the input into an integral
//    matrix in compressed column form.

//    This package computes the factorization PAP' = LDL'. Note that we store
//    the "functional" form of the factorization by only storing the matrix L.
//    The user is given some freedom to select the permutation matrix P. The
//    recommended default settings select P using the AMD ordering.
//    Alternative strategies allowed to select P include the COLAMD
//    ordering or no column permutation (P=I).

//    The factor L is computed via integer preserving operations via
//    integer-preserving Gaussian elimination. The key part of this algorithm
//    is a REF Sparse triangular solve function which exploits sparsity and
//    symmetry to reduce the number of operations that must be performed.

//    Once L is computed, a simplified version of the triangular solve
//    is performed which assumes the vector b is dense. The final solution
//    vector x is gauranteed to be exact. This vector can be output in one of
//    three ways: 1) full precision rational arithmetic (as a sequence of
//    numerators and denominators) using the GMP mpq_t data type, 2) double
//    precision while not exact will produce a solution accurate to machine
//    roundoff unless the size of the associated solution exceeds double
//    precision (i.e., the solution is 10^500 or something), 3) variable
//    precision floating point using the GMP mpfr_t data type. The associated
//    precision is user defined.


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-----------------------Primary SPEX Cholesky routines-------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// Purpose: Compute the exact solution of Ax = b via Cholesky factorization of A
// On input, A is expected to be SPD and x is NULL
// On output, x contains the solution of the linear system

SPEX_info SPEX_cholesky_backslash
(
    // Output
    SPEX_matrix *x_handle,      // On input: undefined.
                                // On output: solution vector(s)
    // Input
    SPEX_type type,             // Type of output desired
                                // Must be SPEX_FP64, SPEX_MPFR, or SPEX_MPQ
    const SPEX_matrix A,        // Input matrix. Must be SPEX_MPZ and SPEX_CSC
    const SPEX_matrix b,        // Right hand side vector(s). Must be
                                // SPEX_MPZ and SPEX_DENSE
    const SPEX_options option   // Command options (Default if NULL)
) ;

SPEX_info SPEX_cholesky_analyze
(
    // Output
    SPEX_symbolic_analysis *S_handle, // Symbolic analysis data structure
    // Input
    const SPEX_matrix A,        // Input matrix. Must be SPEX_MPZ and SPEX_CSC
    const SPEX_options option   // Command options (Default if NULL)
) ;

SPEX_info SPEX_cholesky_factorize
(
    // Output
    SPEX_factorization *F_handle,   // Cholesky factorization struct
    //Input
    const SPEX_matrix A,            // Matrix to be factored. Must be SPEX_MPZ
                                    // and SPEX_CSC
    const SPEX_symbolic_analysis S, // Symbolic analysis struct containing the
                                    // elimination tree of A, the column
                                    // pointers of L, and the exact number of
                                    // nonzeros of L.
    const SPEX_options option       // command options.
                                    // Notably, option->algo indicates whether
                                    // SPEX_CHOL_UP (default) or SPEX_CHOL_LEFT
                                    // is used.
) ;

// Purpose: After computing the REF Cholesky factorization A = LDL',
// this function solves the associated linear system LDL' x = b.
//
// On input x is undefined, F contains the REF Cholesky factorization
// of A (including L, rhos, and row permutation), b contains
// the user's right hand side.
//
// On output x contains the rational solution of the system LDL' x = b
// x and b be can be single vectors, or matrices.


SPEX_info SPEX_cholesky_solve
(
    // Output
    SPEX_matrix *x_handle,      // On input: undefined.
                                // On output: Rational solution (SPEX_MPQ)
                                // to the system.
    // input/output:
    SPEX_factorization F,       // The Cholesky factorization.
                                // Mathematically, F is unchanged.
    // input:
    const SPEX_matrix b,        // Right hand side vector
    const SPEX_options option   // command options
) ;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-----------------------Primary SPEX LDL routines-------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//    This portion of the SPEX library exactly solves a sparse symmetric 
//    indefinite or negative definite system with nonzero leading principle minors
//    via an LDL factorization. The approach is very similar to the one for SPEX 
//    Cholesky and is also described in the paper:

//    "Algorithm 1xxx: SPEX Cholesky and SPEX Backslash for Exactly Solving
//     Sparse Linear Systems," L. Mejia Domenzain, J. Chen, C. Lourenco, 
//     E. Moreno-Centeno, T. Davis, submitted to ACM TOMS

//     The theory associated with this paper is found at:

//    "Exactly Solving Sparse Rational Linear Systems via Roundoff-Error-Free
//    Cholesky Factorizations", C. Lourenco, E. Moreno-Centeno,
//    SIAM J. Matrix Analysis and Applications.
//     pp 609-638, vol 43, no 1, 2022.

//    To use this code you must first download and install the GMP,
//    MPFR, AMD, and COLAMD libraries. GMP and MPFR can be found at:
//              https://gmplib.org/
//              http://www.mpfr.org/
//
//   SPEX_Utilities, AMD, and COLAMD are distributed along with SPEX_Cholesky.
//   The easiest way ensure these dependencies are met is to only access this
//   package through the SPEX repository.
//
//   All of these codes are components of the SPEX software library. This code
//   may be found at:
//              https://github.com/clouren/spex
//              www.suitesparse.com
//
//

// Perform symbolic analysis prior to factorization
SPEX_info SPEX_ldl_analyze
(
    // Output
    SPEX_symbolic_analysis *S_handle, // Symbolic analysis data structure
    // Input
    const SPEX_matrix A,        // Input matrix. Must be SPEX_MPZ and SPEX_CSC
    const SPEX_options option   // Command options (Default if NULL)
) ;


// Factorize a given matrix with SPEX LDL. A must be symmetric
// with nonzero leading principle minors
SPEX_info SPEX_ldl_factorize
(
    // Output
    SPEX_factorization *F_handle,   // Cholesky factorization struct
    //Input
    const SPEX_matrix A,            // Matrix to be factored. Must be SPEX_MPZ
                                    // and SPEX_CSC
    const SPEX_symbolic_analysis S, // Symbolic analysis struct containing the
                                    // elimination tree of A, the column
                                    // pointers of L, and the exact number of
                                    // nonzeros of L.
    const SPEX_options option       // command options.
                                    // Notably, option->algo indicates whether
                                    // SPEX_LDL_UP (default) or SPEX_LDL_LEFT
                                    // is used.
);

// Solve the system after factorization
SPEX_info SPEX_ldl_solve
(
    // Output
    SPEX_matrix *x_handle,      // On input: undefined.
                                // On output: Rational solution (SPEX_MPQ)
                                // to the system.
    // input/output:
    SPEX_factorization F,       // The non-updatable Cholesky factorization.
                                // Mathematically, F is unchanged.  However, if
                                // F is updatable on input, it is converted to
                                // non-updatable.  If F is already
                                // non-updatable, it is not modified.
    // input:
    const SPEX_matrix b,        // Right hand side vector
    const SPEX_options option   // command options
) ;

// Solve the system Ax = b via LDL factorization
SPEX_info SPEX_ldl_backslash
(
    // Output
    SPEX_matrix *x_handle,      // On input: undefined.
                                // On output: solution vector(s)
    // Input
    SPEX_type type,             // Type of output desired
                                // Must be SPEX_FP64, SPEX_MPFR, or SPEX_MPQ
    const SPEX_matrix A,        // Input matrix. Must be SPEX_MPZ and SPEX_CSC
    const SPEX_matrix b,        // Right hand side vector(s). Must be
                                // SPEX_MPZ and SPEX_DENSE
    const SPEX_options option   // Command options (Default if NULL)
);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//--------------------------SPEX Backslash--------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// SPEX_backslash is a wrapper for the exact routines contained within the
// SPEX software package.

// SPEX_BACKSLASH: solve Ax=b via sparse integer-preserving factorization.
// SPEX_backslash: computes the exact solution to the sparse linear system Ax =
// b. A and b may be stored as either int64, double precision, arbitrary
// precision floating point (mpfr_t), arbitrary sized integer (mpz_t), or
// arbitrary size rational numbers (mpq_t).  The result x is computed exactly,
// represented in arbitrary-precision rational values.  This solution vector
// may be returned in either this rational form, or in double precision or in
// arbitrary precision floating point.
//
// A must be square. The bahavior of SPEX backslash depends on the value of
// option->algo.
// If option->algo is left as default:
//      an up-looking LDL factorization is attempted if A is symmetric
//      otherwise an exact left-looking LU factorization is applied.
// If option->algo is specified:
//      The exact algorithm requested is utilized with no substitutions.
//      An appropriate error code is returned if the selected algorithm
//      is not appopriate for the given matrix (e.g., if option->algo
//      is SPEX_LDL_LEFT but the matrix is not symmetric).

//------------------------------------------------------------------------------
// Purpose: Solve Ax = b by analyzing the input matrix and applying the
// appropiate factorization approach
//------------------------------------------------------------------------------

SPEX_info SPEX_backslash
(
    // Output
    SPEX_matrix *x_handle,      // On output: Final solution vector(s)
                                // On input: undefined
    // Input
    const SPEX_type type,       // Type of output desired
                                // Must be SPEX_MPQ, SPEX_MPFR, or SPEX_FP64
    const SPEX_matrix A,        // Input matrix
    const SPEX_matrix b,        // Right hand side vector(s)
    SPEX_options option         // Command options (NULL: means use defaults)
) ;

#if defined ( __cplusplus )
}
#endif

#endif

