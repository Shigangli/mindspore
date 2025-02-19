/**
 * Copyright 2020 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP16_MATMUL_FP16_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP16_MATMUL_FP16_H_

#include <vector>
#include "src/runtime/kernel/cpu/fp16/matmul_base_fp16.h"

namespace mindspore::kernel {
class MatmulFP16CPUKernel : public MatmulBaseFP16CPUKernel {
 public:
  explicit MatmulFP16CPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                               const std::vector<lite::Tensor *> &outputs, const lite::InnerContext *ctx)
      : MatmulBaseFP16CPUKernel(parameter, inputs, outputs, ctx) {}
  ~MatmulFP16CPUKernel() override = default;
  int Prepare() override;
  int ReSize() override;
  int Run() override;
  int Eval() override;

 private:
  int InitAShape() override;
  int InitBShape() override;
  int InitBroadcastParams();
};
}  // namespace mindspore::kernel

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP16_MATMUL_FP16_H_
