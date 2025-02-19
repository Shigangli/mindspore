/**
 * Copyright 2021 Huawei Technologies Co., Ltd
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

#include "nnacl/infer/where_infer.h"
#include <stdio.h>
#include "nnacl/infer/infer_register.h"

static size_t GetAxisout(const TensorC *input0, const TensorC *input1, const TensorC *input2, size_t index) {
  if (input0->shape_[index] == input1->shape_[index] && input0->shape_[index] != input2->shape_[index]) {
    return index;
  }
  if (input0->shape_[index] == input2->shape_[index] && input0->shape_[index] != input1->shape_[index]) {
    return index;
  }
  if (input1->shape_[index] == input2->shape_[index] && input0->shape_[index] != input1->shape_[index]) {
    return index;
  }
  return MAX_SHAPE_SIZE + 1;
}

int WhereInferShape(const TensorC *const *inputs, size_t inputs_size, TensorC **outputs, size_t outputs_size,
                    OpParameter *parameter) {
  int check_ret = CheckAugmentWithMinSize(inputs, inputs_size, outputs, outputs_size, parameter, 1, 1);
  if (check_ret != NNACL_OK) {
    return check_ret;
  }

  const TensorC *input = inputs[0];
  TensorC *output = outputs[0];

  // Need to dynamically allocate at runtime.
  if (inputs_size == 1) {
    output->data_type_ = kNumberTypeInt32;
    output->format_ = input->format_;
    return NNACL_INFER_INVALID;
  }

  if (inputs_size < 3 || outputs_size != 1) {
    return NNACL_INPUT_TENSOR_ERROR;
  }

  const TensorC *input0 = inputs[0];
  const TensorC *input1 = inputs[1];
  const TensorC *input2 = inputs[2];
  SetDataTypeFormat(output, input1);
  if (!InferFlag(inputs, inputs_size)) {
    return NNACL_INFER_INVALID;
  }

  int num = GetElementNum(input0);
  int num1 = GetElementNum(input1);
  int num2 = GetElementNum(input2);
  int nummax = num > num1 ? num : (num1 > num2 ? num1 : num2);
  size_t min_input_shape_size = input1->shape_size_ < input2->shape_size_ ? input1->shape_size_ : input2->shape_size_;
  size_t axisout = MAX_SHAPE_SIZE + 1;
  size_t temp = 0;
  for (size_t j = 0; j < input0->shape_size_; j++) {
    if (j >= MAX_SHAPE_SIZE) {
      return NNACL_ERR;
    }
    if (j < min_input_shape_size) {
      axisout = GetAxisout(input0, input1, input2, j);
      if (axisout != MAX_SHAPE_SIZE + 1) {
        break;
      }
    }
    temp += 1;
    if (temp == input0->shape_size_) {
      SetShapeTensor(output, input);
      return NNACL_OK;
    }
  }

  ShapeSet(output->shape_, &output->shape_size_, input0->shape_, input0->shape_size_);
  if (axisout != MAX_SHAPE_SIZE + 1) {
    output->shape_[axisout] = nummax;
  }
  return NNACL_OK;
}

REG_INFER(Where, PrimType_Where, WhereInferShape)
