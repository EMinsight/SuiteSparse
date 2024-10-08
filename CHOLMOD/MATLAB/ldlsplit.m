function [L,D] = ldlsplit (LD)
%LDLSPLIT split an LDL' factorization into L and D
%
% Example:
%   [L,D] = ldlsplit (LD)
%
% LD contains an LDL' factorization, computed with LD = ldlchol(A), for
% example.  The diagonal of LD contains D, and the entries below the
% diagonal contain L (which has a unit diagonal).  This function splits
% LD into its two components L and D so that L*D*L' = A.
%
% See also ldlchol, ldlsolve, ldlupdate.

 % Copyright 2006-2023, Timothy A. Davis, All Rights Reserved.
 % SPDX-License-Identifier: GPL-2.0+

n = size (LD,1) ;
D = diag (diag (LD)) ;
L = tril (LD,-1) + speye (n) ;
