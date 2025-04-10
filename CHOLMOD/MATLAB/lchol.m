function [L,p,q] = lchol (A)                                          %#ok
%LCHOL sparse A=L*L' factorization.
% Note that L*L' (LCHOL) and L*D*L' (LDLCHOL) factorizations are faster
% than R'*R (CHOL2 and CHOL) and use less memory.  The LL' and LDL'
% factorization methods use tril(A).  A must be sparse.
%
% Example:
%   L = lchol (A)         same as L = chol (A')', just faster
%   [L,p] = lchol (A)     same as [R,p] = chol(A') ; L=R', just faster
%   [L,p,q] = lchol (A)   factorizes A(q,q) into L*L', where q is a
%                         fill-reducing ordering
%
% See also chol2, ldlchol, chol.

 % Copyright 2006-2023, Timothy A. Davis, All Rights Reserved.
 % SPDX-License-Identifier: GPL-2.0+

error ('lchol mexFunction not found') ;

