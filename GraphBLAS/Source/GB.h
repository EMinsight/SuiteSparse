//------------------------------------------------------------------------------
// GB.h: definitions visible only inside GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_H
#define GB_H

// These methods and definitions are also available to JIT kernels;
// see GB_jit_kernel.h:
#include "include/GB_include.h"

// These are not needed by JIT kernels and do not appear in GB_jit_kernel.h:
#include "global/GB_Global.h"
#include "pji_control/GB_determine_pji_is_32.h"
#include "print/GB_printf.h"
#include "ok/GB_assert_library.h"
#if defined ( GRAPHBLAS_HAS_CUDA )
#include "rmm_wrap.h"
#endif
#include "positional/GB_positional.h"
#include "math/GB_bitwise.h"
#include "print/GB_check.h"
#include "nvals/GB_nnz.h"
#include "omp/GB_omp.h"
#include "memory/GB_memory.h"
#include "iso/GB_iso.h"
#include "pending/GB_Pending_n.h"
#include "nvals/GB_nvals.h"
#include "aliased/GB_aliased.h"
#include "matrix/GB_matrix.h"
#include "clear/GB_clear.h"
#include "dup/GB_dup.h"
#include "compatible/GB_code_compatible.h"
#include "compatible/GB_compatible.h"
#include "slice/GB_task_methods.h"
#include "transplant/GB_transplant.h"
#include "type/GB_type.h"
#include "math/GB_uint64_multiply.h"
#include "math/GB_int64_multiply.h"
#include "math/GB_size_t_multiply.h"
#include "cumsum/GB_cumsum.h"
#include "get_set/GB_Descriptor_get.h"
#include "element/GB_Element.h"
#include "op/GB_op.h"
#include "hyper/GB_hyper.h"
#include "werk/GB_where.h"
#include "cast/GB_cast.h"
#include "wait/GB_wait.h"
#include "convert/GB_convert.h"
#include "context/GB_Context.h"
#include "gateway/GB_cuda_gateway.h"
#include "callback/GB_callbacks.h"
#include "helper/GB_factory.h"

#endif

