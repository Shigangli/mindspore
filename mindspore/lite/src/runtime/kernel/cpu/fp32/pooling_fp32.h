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

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_POOLING_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_POOLING_H_

#include <vector>
#include "src/runtime/kernel/cpu/base/pooling_base.h"
#include "src/lite_kernel.h"

namespace mindspore::kernel {
class PoolingCPUKernel : public PoolingBaseCPUKernel {
 public:
  PoolingCPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                   const std::vector<lite::Tensor *> &outputs, const InnerContext *ctx)
      : PoolingBaseCPUKernel(parameter, inputs, outputs, ctx) {}
  ~PoolingCPUKernel() override = default;

  int Prepare() override;
  int ReSize() override;
  int Run() override;
  int RunImpl(int task_id) const;

 private:
};
}  // namespace mindspore::kernel

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_POOLING_H_
