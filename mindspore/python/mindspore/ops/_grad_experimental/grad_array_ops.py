# Copyright 2021-2022 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================

"""array_ops"""

from mindspore import Tensor
from ...common import dtype as mstype
from .._grad.grad_math_ops import binop_grad_common
from .._grad.grad_base import bprop_getters
from ..composite.multitype_ops.zeros_like_impl import zeros_like
from ..operations.array_ops import Tril
from ..operations.array_ops import MatrixDiagV3
from ..operations.array_ops import MatrixDiagPartV3
from ..operations.array_ops import MatrixSetDiagV3
from ..operations.array_ops import Triu
from .. import functional as F
from .. import operations as P
from .._utils.utils import is_shape_unknown


@bprop_getters.register(P.MaskedFill)
def get_bprop_masked_select(self):
    """Generate bprop for MaskedFill"""
    mul_op = P.Mul()
    sum_op = P.ReduceSum()
    is_instance_op = P.IsInstance()

    def bprop(input_data, mask, value, out, dout):
        mask = F.cast(mask, mstype.float32)
        dinput = mul_op(dout, (1 - mask))
        dvalue = mul_op(dout, mask)
        dinput, dvalue = binop_grad_common(input_data, mask, dinput, dvalue)
        dvalue = sum_op(dvalue)
        dinput = F.cast(dinput, F.dtype(input_data))
        if is_instance_op(value, mstype.number):
            dvalue = 0
        else:
            dvalue = F.cast(dvalue, F.dtype(value))
        return dinput, zeros_like(mask), dvalue

    return bprop


@bprop_getters.register(P.TensorScatterSub)
def get_bprop_tensor_scatter_sub(self):
    """Generate bprop for TensorScatterSub"""
    gather_nd = P.GatherNd()
    neg = P.Neg()

    def bprop(x, indices, update, out, dout):
        update_grad = neg(gather_nd(dout, indices))
        return dout, zeros_like(indices), update_grad

    return bprop


@bprop_getters.register(MatrixDiagV3)
def get_bprop_matrix_diag_v3(self):
    """Generate bprop for MatrixDiagV3"""
    align = self.align
    matrix_diag_part_v3 = MatrixDiagPartV3(align=align)
    zeros = P.Zeros()

    def bprop(x, k, num_rows, num_cols, padding_value, out, dout):
        result = (matrix_diag_part_v3(dout, k, zeros((), dout.dtype)), zeros_like(k), zeros_like(num_rows),
                  zeros_like(num_cols), zeros_like(padding_value))
        return result

    return bprop


@bprop_getters.register(MatrixDiagPartV3)
def get_bprop_matrix_diag_part_v3(self):
    """Generate bprop for MatrixDiagPartV3"""
    align = self.align
    matrix_diag_v3 = MatrixDiagV3(align=align)
    matrix_set_diag_v3 = MatrixSetDiagV3(align=align)
    zeros = P.Zeros()

    def bprop(x, k, padding_value, out, dout):
        shape_this = P.Shape()(x)[-2:]
        if not is_shape_unknown(shape_this):
            row = shape_this[0]
            col = shape_this[1]
            result = (matrix_diag_v3(dout, k, Tensor(row, dtype=mstype.int32), Tensor(col, dtype=mstype.int32),
                                     zeros((), dout.dtype)), zeros_like(k), zeros_like(padding_value))
        else:
            result = (matrix_set_diag_v3(zeros_like(x), dout, k), zeros_like(k), zeros_like(padding_value))
        return result

    return bprop


@bprop_getters.register(MatrixSetDiagV3)
def get_bprop_matrix_set_diag_v3(self):
    """Generate bprop for MatrixSetDiagV3"""
    align = self.align
    matrix_diag_part_v3 = MatrixDiagPartV3(align=align)
    matrix_set_diag_v3 = MatrixSetDiagV3(align=align)
    resha = P.Reshape()
    zeros = P.Zeros()
    minimum = P.Minimum()
    concat = P.Concat()

    def bprop(x, diagonal, k, out, dout):
        diagonal_cal = matrix_diag_part_v3(dout, k, zeros((), dout.dtype))

        diagonal_shape = P.Shape()(diagonal)
        if is_shape_unknown(diagonal_shape):
            shape_dout = P.Shape()(dout)
            pre_shape = shape_dout[:-2]
            back_shape = shape_dout[-2:]

            site_dia = resha(k, (-1))
            index_min = -1 * site_dia[0]
            index_max = site_dia[-1]
            col = 0
            if index_max < 0:
                col = index_max
            row = 0
            if index_min < 0:
                row = index_min
            max_diag_len = minimum(back_shape[0] + col, back_shape[1] + row)

            back = [max_diag_len]
            if index_max != index_min:
                back = [index_max-index_min+1, max_diag_len]
            diagonal_shape = concat([pre_shape, back])
        x_cal = matrix_set_diag_v3(dout, zeros(diagonal_shape, dout.dtype), k)

        return x_cal, diagonal_cal, zeros_like(k)

    return bprop


def tensor_scatter_possible_replacement(x, indices, updates, out, dout):
    """bpropr for any TensorScatter* op that possibly replaces values in the input tensor"""
    gather_nd = P.GatherNd()
    scatter_nd = P.ScatterNd()
    equal = P.Equal()
    shape = P.Shape()

    x_indicators = F.cast(equal(x, out), mstype.int32)
    possibly_updated = gather_nd(out, indices)
    out_indicators = F.cast(equal(updates, possibly_updated), mstype.int32)
    scattered_out_indicators = scatter_nd(indices, out_indicators, shape(x))
    indicators = x_indicators + scattered_out_indicators
    dx = dout * F.cast(x_indicators, F.dtype(dout)) / F.cast(indicators, F.dtype(dout))
    dupdates = gather_nd(dout / F.cast(indicators, F.dtype(dout)), indices) * F.cast(out_indicators, F.dtype(dout))

    return F.cast(dx, F.dtype(x)), zeros_like(indices), F.cast(dupdates, F.dtype(updates))


@bprop_getters.register(P.TensorScatterMax)
def get_bprop_tensor_scatter_max(self):
    """Generate bprop for TensorScatterMax"""
    def bprop(x, indices, updates, out, dout):
        return tensor_scatter_possible_replacement(x, indices, updates, out, dout)

    return bprop


@bprop_getters.register(P.TensorScatterMin)
def get_bprop_tensor_scatter_min(self):
    """Generate bprop for TensorScatterMin"""
    def bprop(x, indices, updates, out, dout):
        return tensor_scatter_possible_replacement(x, indices, updates, out, dout)

    return bprop


@bprop_getters.register(P.Coalesce)
def get_bprop_coalesce(self):
    """Grad definition for `Coalesce` operation."""

    def bprop(x_indices, x_values, x_shape, out, dout):
        return dout

    return bprop


@bprop_getters.register(Triu)
def get_bprop_triu(self):
    """Grad definition for 'Triu' operation"""
    diagonal = self.diagonal
    triu = Triu(diagonal)

    def bprop(x, out, dout):
        dx = triu(dout)
        return (dx,)

    return bprop


@bprop_getters.register(P.SplitV)
def get_bprop_split_v(self):
    """Generate bprop for SplitV"""
    split_dim = self.split_dim
    concat_op = P.Concat(split_dim)

    def bprop(x_input, output, dout):
        dx = concat_op(dout)
        return (dx,)

    return bprop


@bprop_getters.register(P.ExtractVolumePatches)
def get_bprop_extract_volume_patches(self):
    """Generate bprop for ExtractVolumePatches"""
    extract_volume_patches = P.ExtractVolumePatches(kernel_size=self.kernel_size,
                                                    strides=self.strides, padding=self.padding)
    concat = P.Concat(axis=-1)
    expend_dims = P.ExpandDims()
    scatter_nd = P.ScatterNd()
    slice_op = P.Slice()
    fill = P.Fill()
    dtype = P.DType()
    cast = P.Cast()
    matmul = P.MatMul()
    _, _, ksize_d, ksize_h, ksize_w = self.kernel_size

    def bprop(x, out, dout):
        x_shape = P.Shape()(x)
        x_n, x_c, x_d, x_h, x_w = x_shape
        x_indices_num = 1 + x_d * x_h * x_w
        x_idx = cast(F.tuple_to_array(range(1, x_indices_num)), mstype.float16)
        x_idx = P.Reshape()(x_idx, (1, 1, x_d, x_h, x_w))
        x_idx_patched = extract_volume_patches(x_idx)
        x_idx_patched = P.Transpose()(x_idx_patched, (0, 2, 3, 4, 1))
        x_idx_patched = cast(x_idx_patched, mstype.int32)

        out_shape = P.Shape()(out)
        _, _, out_d, out_h, out_w = out_shape
        out_indices_num = out_d * out_h * out_w * ksize_d * ksize_h * ksize_w
        out_idx = F.tuple_to_array(range(0, out_indices_num))
        out_idx = P.Reshape()(out_idx, (1, out_d, out_h, out_w, ksize_d * ksize_h * ksize_w))

        idx_tensor = concat((expend_dims(x_idx_patched, -1), expend_dims(out_idx, -1)))
        idx_map = P.Reshape()(idx_tensor, (-1, 2))
        sp_shape = (x_indices_num, out_indices_num)
        sp_mat_full = scatter_nd(idx_map, fill(dtype(dout), (out_indices_num,), 1), sp_shape)
        sp_tensor = slice_op(sp_mat_full, (1, 0), (x_indices_num - 1, out_indices_num))

        grad = P.Transpose()(dout, (0, 2, 3, 4, 1))
        grad = P.Reshape()(grad, (x_n, out_d, out_h, out_w, ksize_d,
                                  ksize_h, ksize_w, x_c))
        grad_expended = P.Transpose()(grad, (1, 2, 3, 4, 5, 6, 0, 7))
        grad_flat = P.Reshape()(grad_expended, (-1, x_n * x_c))

        jac = matmul(sp_tensor, grad_flat)
        dx = P.Reshape()(jac, (x_d, x_h, x_w, x_n, x_c))
        dx = P.Transpose()(dx, (3, 4, 0, 1, 2))
        return (dx,)

    return bprop


@bprop_getters.register(Tril)
def get_bprop_tril(self):
    """Grad definition for 'Tril' operation"""
    diagonal = self.diagonal
    tril = Tril(diagonal)

    def bprop(x, out, dout):
        dx = tril(dout)
        return (dx,)

    return bprop
