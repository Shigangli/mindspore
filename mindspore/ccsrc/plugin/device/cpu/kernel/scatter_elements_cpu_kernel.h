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

#ifndef MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_CPU_SCATTER_ELEMENTS_CPU_KERNEL_H_
#define MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_CPU_SCATTER_ELEMENTS_CPU_KERNEL_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <utility>
#include "plugin/device/cpu/kernel/cpu_kernel.h"
#include "plugin/factory/ms_factory.h"

namespace mindspore::kernel {
constexpr auto kUnKnown = "UnKnown";
constexpr auto kScatterElements = "ScatterElements";
constexpr auto kScatterAddWithAxis = "ScatterAddWithAxis";

class ScatterElementsCpuKernelMod : public NativeCpuKernelMod {
 public:
  ScatterElementsCpuKernelMod() = default;

  explicit ScatterElementsCpuKernelMod(const std::string &kernel_type) : kernel_type_(kernel_type) {}
  ~ScatterElementsCpuKernelMod() override = default;

  bool Init(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
            const std::vector<KernelTensorPtr> &outputs) override;

  bool Resize(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
              const std::vector<KernelTensorPtr> &outputs,
              const std::map<uint32_t, tensor::TensorPtr> &others = std::map<uint32_t, tensor::TensorPtr>()) override;

  bool Launch(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &workspace,
              const std::vector<AddressPtr> &outputs) override {
    return kernel_func_(this, inputs, outputs);
  }

 protected:
  std::vector<KernelAttr> GetOpSupport() override;

  template <typename T, typename S, typename ReductionT>
  bool LaunchKernel(const std::vector<kernel::AddressPtr> &inputs, const std::vector<kernel::AddressPtr> &outputs);

  template <typename T, typename ReductionT>
  bool Scatter(const ReductionT &reduction_func, T *output, const T *updates);

  template <typename S>
  bool AdjustIndices(S *in_indices);

  size_t ComputeOutoutOffset(const int64_t &index);

  void UpdateOutputDimIndex();

 private:
  using ScatterElementsLaunchFunc = std::function<bool(
    ScatterElementsCpuKernelMod *, const std::vector<kernel::AddressPtr> &, const std::vector<kernel::AddressPtr> &)>;

  static std::map<std::string, std::vector<std::pair<KernelAttr, ScatterElementsLaunchFunc>>> func_map_;
  ScatterElementsLaunchFunc kernel_func_;
  std::string kernel_type_{kUnKnown};
  int input_axis_size_{0};
  size_t input_size_{1};
  size_t indices_total_num_{1};
  size_t input_dims_{0};
  int64_t axis_{0};
  std::vector<int64_t> indices_shape_{};
  std::vector<size_t> output_dim_stride_{};
  std::vector<size_t> output_dim_index_{};
  std::vector<int64_t> adjusted_indices_{};
  std::string kernel_name_;
};
}  // namespace mindspore::kernel

#endif  // MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_CPU_SCATTER_ELEMENTS_CPU_KERNEL_H_
