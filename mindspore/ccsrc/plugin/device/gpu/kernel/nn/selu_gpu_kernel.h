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

#ifndef MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_GPU_NN_SELU_GPU_KERNEL_H_
#define MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_GPU_NN_SELU_GPU_KERNEL_H_
#include <vector>
#include <string>
#include <utility>
#include <map>
#include "plugin/device/gpu/kernel/gpu_kernel.h"
#include "plugin/factory/ms_factory.h"

namespace mindspore {
namespace kernel {
class SeluGpuKernelMod : public NativeGpuKernelMod {
 public:
  SeluGpuKernelMod() { ResetResource(); }
  ~SeluGpuKernelMod() override = default;
  bool Launch(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &,
              const std::vector<AddressPtr> &outputs, void *cuda_stream) override {
    if (is_null_input_) {
      return true;
    }
    cuda_stream_ = cuda_stream;
    return kernel_func_(this, inputs, outputs);
  }

  bool Init(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
            const std::vector<KernelTensorPtr> &outputs) override;

  bool Resize(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
              const std::vector<KernelTensorPtr> &outputs, const std::map<uint32_t, tensor::TensorPtr> &) override;

  std::vector<KernelTensorPtr> GetOutputs() override { return outputs_; }

  std::vector<KernelAttr> GetOpSupport() override;

  void ResetResource() noexcept {
    is_null_input_ = false;
    input_elements_ = 0;
    input_size_list_.clear();
    output_size_list_.clear();
    workspace_size_list_.clear();
  }

 protected:
  void InitSizeLists() {
    input_size_list_.clear();
    output_size_list_.clear();
    workspace_size_list_.clear();
    size_t input_size = input_elements_ * unit_size_;
    input_size_list_.push_back(input_size);
    output_size_list_.push_back(input_size);
  }

 private:
  template <typename T>
  bool LaunchKernel(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &outputs);
  using SeluFunc = std::function<bool(SeluGpuKernelMod *, const std::vector<kernel::AddressPtr> &,
                                      const std::vector<kernel::AddressPtr> &)>;

 private:
  size_t unit_size_{1};
  bool is_null_input_{false};
  size_t input_elements_{};
  void *cuda_stream_{nullptr};
  std::vector<size_t> input_shape_;
  std::vector<size_t> output_shape_;
  std::vector<KernelTensorPtr> outputs_ = {};
  SeluFunc kernel_func_{};
  static std::vector<std::pair<KernelAttr, SeluFunc>> func_list_;
};
}  // namespace kernel
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_GPU_NN_SELU_GPU_KERNEL_H_
