//------------------------------------------------------------------------------
// GB_subassign_02_template: C(I,J) += A ; using S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Method 04: C(I,J) += A ; using S

// M:           NULL
// Mask_comp:   false
// C_replace:   false
// accum:       present
// A:           matrix
// S:           constructed

// C: not bitmap: use GB_bitmap_assign instead
// A: any sparsity structure.

{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GB_EMPTY_TASKLIST ;
    GB_GET_C ;      // C must not be bitmap
    GB_GET_S ;
    GB_GET_ACCUM_MATRIX ;

    //--------------------------------------------------------------------------
    // Method 04: C(I,J) += A ; using S
    //--------------------------------------------------------------------------

    // Time: Close to Optimal.  Every entry in A must be visited, and the
    // corresponding entry in S must then be found.  Time for this phase is
    // Omega(nnz(A)), but S has already been constructed, in Omega(nnz(S))
    // time.  This method simply traverses all of A+S (like GB_add for
    // computing A+S), the same as Method 02.  Time taken is O(nnz(A)+nnz(S)).
    // The only difference is that the traversal of A+S can terminate if A is
    // exhausted.  Entries in S but not A do not actually require any work
    // (unlike Method 02, which must visit all entries in A+S).

    // Method 02 and Method 04 are somewhat similar.  They differ on how C is
    // modified when the entry is present in S but not A.

    // Compare with Method 16, which computes C(I,J)<!M> += A, using S.

    //--------------------------------------------------------------------------
    // Parallel: A+S (Methods 02, 04, 09, 10, 11, 12, 14, 16, 18, 20)
    //--------------------------------------------------------------------------

    if (GB_A_IS_BITMAP)
    { 
        // all of IxJ must be examined
        GB_SUBASSIGN_IXJ_SLICE ;
    }
    else
    { 
        // traverse all A+S
        GB_SUBASSIGN_TWO_SLICE (A, S) ;
    }

    //--------------------------------------------------------------------------
    // phase 1: create zombies, update entries, and count pending tuples
    //--------------------------------------------------------------------------

    if (GB_A_IS_BITMAP)
    {

        //----------------------------------------------------------------------
        // phase1: A is bitmap
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:nzombies)
        for (taskid = 0 ; taskid < ntasks ; taskid++)
        {

            //------------------------------------------------------------------
            // get the task descriptor
            //------------------------------------------------------------------

            GB_GET_IXJ_TASK_DESCRIPTOR_PHASE1 (iA_start, iA_end) ;

            //------------------------------------------------------------------
            // compute all vectors in this task
            //------------------------------------------------------------------

            for (int64_t j = kfirst ; j <= klast ; j++)
            {

                //--------------------------------------------------------------
                // get S(iA_start:iA_end,j)
                //--------------------------------------------------------------

                GB_LOOKUP_VECTOR_S_FOR_IXJ (j, pS, pS_end, iA_start) ;
                int64_t pA_start = j * Avlen ;

                //--------------------------------------------------------------
                // do a 2-way merge of S(iA_start:iA_end,j) and A(ditto,j)
                //--------------------------------------------------------------

                for (int64_t iA = iA_start ; iA < iA_end ; iA++)
                {
                    int64_t pA = pA_start + iA ;
                    bool Sfound = (pS < pS_end) && (GBi_S (Si,pS,Svlen) == iA) ;
                    bool Afound = Ab [pA] ;
                    if (Sfound && !Afound)
                    { 
                        // ----[C . 1] or [X . 1]-------------------------------
                        // S (i,j) is present but A (i,j) is not
                        // [C . 1]: action: ( C ): no change, with accum
                        // [X . 1]: action: ( X ): still a zombie
                        pS++ ;  // go to the next entry in S(:,j)
                    }
                    else if (!Sfound && Afound)
                    { 
                        // ----[. A 1]------------------------------------------
                        // S (i,j) is not present, A (i,j) is present
                        // [. A 1]: action: ( insert )
                        task_pending++ ;
                    }
                    else if (Sfound && Afound)
                    { 
                        // ----[C A 1] or [X A 1]-------------------------------
                        // both S (i,j) and A (i,j) present
                        // [C A 1]: action: ( =C+A ): apply accum
                        // [X A 1]: action: ( undelete ): zombie lives
                        GB_C_S_LOOKUP ;
                        GB_withaccum_C_A_1_matrix ;
                        pS++ ;  // go to the next entry in S(:,j)
                    }
                }
            }
            GB_PHASE1_TASK_WRAPUP ;
        }
    }
    else
    {

        //----------------------------------------------------------------------
        // phase1: A is hypersparse, sparse, or full
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:nzombies)
        for (taskid = 0 ; taskid < ntasks ; taskid++)
        {

            //------------------------------------------------------------------
            // get the task descriptor
            //------------------------------------------------------------------

            GB_GET_TASK_DESCRIPTOR_PHASE1 ;

            //------------------------------------------------------------------
            // compute all vectors in this task
            //------------------------------------------------------------------

            for (int64_t k = kfirst ; k <= klast ; k++)
            {

                //--------------------------------------------------------------
                // get A(:,j) and S(:,j)
                //--------------------------------------------------------------

                int64_t j = GBh (Zh, k) ;

//              GB_GET_MAPPED (pA, pA_end, pA, pA_end, Ap, j, k, Z_to_X, Avlen);
                int64_t pA = -1, pA_end = -1 ;
                if (fine_task)
                { 
                    // A fine task operates on a slice of A(:,k)
                    pA     = TaskList [taskid].pA ;
                    pA_end = TaskList [taskid].pA_end ;
                }
                else
                { 
                    // vectors are never sliced for a coarse task
                    int64_t kA = (Z_to_X == NULL) ? j : Z_to_X [k] ;
                    if (kA >= 0)
                    { 
                        pA     = GBp_A (Ap, kA, Avlen) ;
                        pA_end = GBp_A (Ap, kA+1, Avlen) ;
                    }
                }

//              GB_GET_MAPPED (pS, pS_end, pB, pB_end, Sp, j, k, Z_to_S, Svlen);
                int64_t pS = -1, pS_end = -1 ;
                if (fine_task)
                { 
                    // A fine task operates on a slice of X(:,k)
                    pS     = TaskList [taskid].pB ;
                    pS_end = TaskList [taskid].pB_end ;
                }
                else
                { 
                    // vectors are never sliced for a coarse task
                    int64_t kS = (Z_to_S == NULL) ? j : Z_to_S [k] ;
                    if (kS >= 0)
                    { 
                        pS     = GBp_S (Sp, kS, Svlen) ;
                        pS_end = GBp_S (Sp, kS+1, Svlen) ;
                    }
                }

                //--------------------------------------------------------------
                // do a 2-way merge of S(:,j) and A(:,j)
                //--------------------------------------------------------------

                // jC = J [j] ; or J is a colon expression
                // int64_t jC = GB_IJLIST (J, j, GB_J_KIND, Jcolon) ;

                // while both list S (:,j) and A (:,j) have entries
                while (pS < pS_end && pA < pA_end)
                {
                    int64_t iS = GBi_S (Si, pS, Svlen) ;
                    int64_t iA = GBi_A (Ai, pA, Avlen) ;

                    if (iS < iA)
                    { 
                        // ----[C . 1] or [X . 1]-------------------------------
                        // S (i,j) is present but A (i,j) is not
                        // [C . 1]: action: ( C ): no change, with accum
                        // [X . 1]: action: ( X ): still a zombie
                        pS++ ;  // go to the next entry in S(:,j)
                    }
                    else if (iA < iS)
                    { 
                        // ----[. A 1]------------------------------------------
                        // S (i,j) is not present, A (i,j) is present
                        // [. A 1]: action: ( insert )
                        task_pending++ ;
                        pA++ ;  // go to the next entry in A(:,j)
                    }
                    else
                    { 
                        // ----[C A 1] or [X A 1]-------------------------------
                        // both S (i,j) and A (i,j) present
                        // [C A 1]: action: ( =C+A ): apply accum
                        // [X A 1]: action: ( undelete ): zombie lives
                        GB_C_S_LOOKUP ;
                        GB_withaccum_C_A_1_matrix ;
                        pS++ ;  // go to the next entry in S(:,j)
                        pA++ ;  // go to the next entry in A(:,j)
                    }
                }

                // ignore the remainder of S (:,j)

                // List A (:,j) has entries.  List S (:,j) exhausted.
                task_pending += (pA_end - pA) ;
            }

            GB_PHASE1_TASK_WRAPUP ;
        }
    }

    //--------------------------------------------------------------------------
    // phase 2: insert pending tuples
    //--------------------------------------------------------------------------

    GB_PENDING_CUMSUM ;

    if (GB_A_IS_BITMAP)
    {

        //----------------------------------------------------------------------
        // phase2: A is bitmap
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(&&:pending_sorted)
        for (taskid = 0 ; taskid < ntasks ; taskid++)
        {

            //------------------------------------------------------------------
            // get the task descriptor
            //------------------------------------------------------------------

            GB_GET_IXJ_TASK_DESCRIPTOR_PHASE2 (iA_start, iA_end) ;

            //------------------------------------------------------------------
            // compute all vectors in this task
            //------------------------------------------------------------------

            for (int64_t j = kfirst ; j <= klast ; j++)
            {

                //--------------------------------------------------------------
                // get S(iA_start:iA_end,j)
                //--------------------------------------------------------------

                GB_LOOKUP_VECTOR_S_FOR_IXJ (j, pS, pS_end, iA_start) ;
                int64_t pA_start = j * Avlen ;

                //--------------------------------------------------------------
                // do a 2-way merge of S(iA_start:iA_end,j) and A(ditto,j)
                //--------------------------------------------------------------

                // jC = J [j] ; or J is a colon expression
                int64_t jC = GB_IJLIST (J, j, GB_J_KIND, Jcolon) ;

                for (int64_t iA = iA_start ; iA < iA_end ; iA++)
                {
                    int64_t pA = pA_start + iA ;
                    bool Sfound = (pS < pS_end) && (GBi_S (Si,pS,Svlen) == iA) ;
                    bool Afound = Ab [pA] ;
                    if (!Sfound && Afound)
                    { 
                        // ----[. A 1]------------------------------------------
                        // S (i,j) is not present, A (i,j) is present
                        // [. A 1]: action: ( insert )
                        int64_t iC = GB_IJLIST (I, iA, GB_I_KIND, Icolon) ;
                        GB_PENDING_INSERT_aij ;
                        pA++ ;  // go to the next entry in A(:,j)
                    }
                    else if (Sfound)
                    { 
                        // S (i,j) present
                        pS++ ;  // go to the next entry in S(:,j)
                    }
                }
            }
            GB_PHASE2_TASK_WRAPUP ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // phase2: A is hypersparse, sparse, or full
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(&&:pending_sorted)
        for (taskid = 0 ; taskid < ntasks ; taskid++)
        {

            //------------------------------------------------------------------
            // get the task descriptor
            //------------------------------------------------------------------

            GB_GET_TASK_DESCRIPTOR_PHASE2 ;

            //------------------------------------------------------------------
            // compute all vectors in this task
            //------------------------------------------------------------------

            for (int64_t k = kfirst ; k <= klast ; k++)
            {

                //--------------------------------------------------------------
                // get A(:,j) and S(:,j)
                //--------------------------------------------------------------

                int64_t j = GBh (Zh, k) ;

//              GB_GET_MAPPED (pA, pA_end, pA, pA_end, Ap, j, k, Z_to_X, Avlen);
                int64_t pA = -1, pA_end = -1 ;
                if (fine_task)
                { 
                    // A fine task operates on a slice of A(:,k)
                    pA     = TaskList [taskid].pA ;
                    pA_end = TaskList [taskid].pA_end ;
                }
                else
                { 
                    // vectors are never sliced for a coarse task
                    int64_t kA = (Z_to_X == NULL) ? j : Z_to_X [k] ;
                    if (kA >= 0)
                    { 
                        pA     = GBp_A (Ap, kA, Avlen) ;
                        pA_end = GBp_A (Ap, kA+1, Avlen) ;
                    }
                }

//              GB_GET_MAPPED (pS, pS_end, pB, pB_end, Sp, j, k, Z_to_S, Svlen);
                int64_t pS = -1, pS_end = -1 ;
                if (fine_task)
                { 
                    // A fine task operates on a slice of X(:,k)
                    pS     = TaskList [taskid].pB ;
                    pS_end = TaskList [taskid].pB_end ;
                }
                else
                { 
                    // vectors are never sliced for a coarse task
                    int64_t kS = (Z_to_S == NULL) ? j : Z_to_S [k] ;
                    if (kS >= 0)
                    { 
                        pS     = GBp_S (Sp, kS, Svlen) ;
                        pS_end = GBp_S (Sp, kS+1, Svlen) ;
                    }
                }

                //--------------------------------------------------------------
                // do a 2-way merge of S(:,j) and A(:,j)
                //--------------------------------------------------------------

                // jC = J [j] ; or J is a colon expression
                int64_t jC = GB_IJLIST (J, j, GB_J_KIND, Jcolon) ;

                // while both list S (:,j) and A (:,j) have entries
                while (pS < pS_end && pA < pA_end)
                {
                    int64_t iS = GBi_S (Si, pS, Svlen) ;
                    int64_t iA = GBi_A (Ai, pA, Avlen) ;

                    if (iS < iA)
                    { 
                        pS++ ;  // go to the next entry in S(:,j)
                    }
                    else if (iA < iS)
                    { 
                        // ----[. A 1]------------------------------------------
                        // S (i,j) is not present, A (i,j) is present
                        // [. A 1]: action: ( insert )
                        int64_t iC = GB_IJLIST (I, iA, GB_I_KIND, Icolon) ;
                        GB_PENDING_INSERT_aij ;
                        pA++ ;  // go to the next entry in A(:,j)
                    }
                    else
                    { 
                        pS++ ;  // go to the next entry in S(:,j)
                        pA++ ;  // go to the next entry in A(:,j)
                    }
                }

                // ignore the remainder of S (:,j)

                // while list A (:,j) has entries.  List S (:,j) exhausted.
                while (pA < pA_end)
                { 
                    // ----[. A 1]----------------------------------------------
                    // S (i,j) is not present, A (i,j) is present
                    // [. A 1]: action: ( insert )
                    int64_t iA = GBi_A (Ai, pA, Avlen) ;
                    int64_t iC = GB_IJLIST (I, iA, GB_I_KIND, Icolon) ;
                    GB_PENDING_INSERT_aij ;
                    pA++ ;  // go to the next entry in A(:,j)
                }
            }
            GB_PHASE2_TASK_WRAPUP ;
        }
    }

    //--------------------------------------------------------------------------
    // finalize the matrix and return result
    //--------------------------------------------------------------------------

    GB_SUBASSIGN_WRAPUP ;
}

