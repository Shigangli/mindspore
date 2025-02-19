/**
 * Copyright 2022 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugin/device/gpu/kernel/cuda_impl/cuda_ops/determinant_by_lu_impl.cuh"
#include "plugin/device/gpu/kernel/cuda_impl/cuda_ops/util.cuh"

__inline__ __device__ int PermutationOrder(int m, const int *per_batch_pivot) {
  int permutation_order = 0;
  for (int i = 0; i < m - 1; ++i) {
    // Refer to http://icl.cs.utk.edu/lapack-forum/viewtopic.php?f=2&t=340 .
    permutation_order += per_batch_pivot[i] != (i + 1);
  }
  return permutation_order;
}

template <typename T>
__global__ void CalculateDeterminantByLuKernel(const T *lu_input, const int *pivot, int m, int batch_size,
                                               bool is_sign_log_determinant, T *determinant_output, T *sign_output) {
  for (size_t index = blockIdx.x * blockDim.x + threadIdx.x; index < (batch_size); index += blockDim.x * gridDim.x) {
    const int permutation_order = PermutationOrder(m, pivot + index * m);
    int prod_sign = permutation_order % 2 ? (-1) : 1;
    T template_zero = static_cast<T>(0);
    T sum_abs_log_det = template_zero;
    int matrix_size = m * m;
    int stride = m + 1;
    size_t lu_i_index = matrix_size * index;
    // Get lu data's diagonal by stride.
    for (int i = 0; i < m; ++i, lu_i_index += stride) {
      const T abs_i = abs(lu_input[lu_i_index]);
      sum_abs_log_det += log(abs_i);
      prod_sign = prod_sign * (lu_input[lu_i_index] / abs_i);
    }
    if (!isfinite(sum_abs_log_det)) {
      prod_sign = 0;
      sum_abs_log_det = sum_abs_log_det > 0 ? -log(template_zero) : log(template_zero);
    }
    if (is_sign_log_determinant) {
      sign_output[index] = prod_sign;
      determinant_output[index] = sum_abs_log_det;
    } else {
      determinant_output[index] = prod_sign * exp(sum_abs_log_det);
    }
  }
}

template <typename T>
CUDA_LIB_EXPORT void CalculateDeterminantByLu(const T *lu_input, const int *pivot, int m, int batch_size,
                                              bool is_sign_log_determinant, T *determinant_output, T *sign_output,
                                              cudaStream_t cuda_stream) {
  // Parallelization by batch_size.
  CalculateDeterminantByLuKernel<<<GET_BLOCKS(batch_size), GET_THREADS, 0, cuda_stream>>>(
    lu_input, pivot, m, batch_size, is_sign_log_determinant, determinant_output, sign_output);
}

template CUDA_LIB_EXPORT void CalculateDeterminantByLu<float>(const float *lu_input, const int *pivot, int m,
                                                              int batch_size, bool is_sign_log_determinant,
                                                              float *determinant_output, float *sign_output,
                                                              cudaStream_t cuda_stream);

template CUDA_LIB_EXPORT void CalculateDeterminantByLu<double>(const double *lu_input, const int *pivot, int m,
                                                               int batch_size, bool is_sign_log_determinant,
                                                               double *determinant_output, double *sign_output,
                                                               cudaStream_t cuda_stream);
