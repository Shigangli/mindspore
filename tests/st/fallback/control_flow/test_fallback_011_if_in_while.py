# Copyright 2022 Huawei Technologies Co., Ltd
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
""" test graph fallback control flow if in while scenario"""
import pytest
import numpy as np
from mindspore import Tensor, ms_function, context

context.set_context(mode=context.GRAPH_MODE)


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_arm_ascend_training
@pytest.mark.platform_x86_ascend_training
@pytest.mark.env_onecard
def test_if_in_while_1():
    """
    Feature: JIT Fallback
    Description: Test fallback with control flow.
    Expectation: No exception.
    """
    @ms_function
    def control_flow_if_in_while():
        x = Tensor(1)
        y = Tensor(0)
        while x < Tensor(5):
            if x % 2 == Tensor(0):
                y += Tensor(1)
            x += Tensor(1)
        return x + y
    res = control_flow_if_in_while()
    assert res == 7


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_arm_ascend_training
@pytest.mark.platform_x86_ascend_training
@pytest.mark.env_onecard
def test_if_in_while_2():
    """
    Feature: JIT Fallback
    Description: Test fallback with control flow.
    Expectation: No exception.
    """
    @ms_function
    def control_flow_if_in_while():
        x = Tensor(1)
        while x < Tensor(5):
            if x % 3 == Tensor(0):
                break
            x += Tensor(1)
        return x
    res = control_flow_if_in_while()
    assert res == 3


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_arm_ascend_training
@pytest.mark.platform_x86_ascend_training
@pytest.mark.env_onecard
def test_if_in_while_3():
    """
    Feature: JIT Fallback
    Description: Test fallback with control flow.
    Expectation: No exception.
    """
    @ms_function
    def control_flow_if_in_while():
        x = Tensor(1)
        y = Tensor(0)
        while x < Tensor(5):
            if x % 3 == Tensor(0):
                x += Tensor(1)
                y += Tensor(1)
                continue
            x += Tensor(1)
        return x + y
    res = control_flow_if_in_while()
    assert res == 6


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_arm_ascend_training
@pytest.mark.platform_x86_ascend_training
@pytest.mark.env_onecard
def test_if_in_while_4():
    """
    Feature: JIT Fallback
    Description: Test fallback with control flow.
    Expectation: No exception.
    """
    @ms_function
    def control_flow_if_in_while():
        x = Tensor(1)
        y = Tensor(0)
        while x < Tensor(10) and x + y < Tensor(20):
            if x % 3 == Tensor(0):
                x += Tensor(1)
                y += Tensor(1)
                continue
            elif y % 2 == Tensor(0):
                x += Tensor(1)
            elif (x+y) % 5 == Tensor(0):
                break
            else:
                x += Tensor(1)
        return x + y
    res = control_flow_if_in_while()
    assert res == 5


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_arm_ascend_training
@pytest.mark.platform_x86_ascend_training
@pytest.mark.env_onecard
def test_if_in_while_built_in_func():
    """
    Feature: JIT Fallback
    Description: Test fallback with control flow.
    Expectation: No exception.
    """
    @ms_function
    def control_flow_if_in_while():
        x = np.array([1, 2, 3, 4])
        y = 0
        while sum(x) <= 40 and y % 2 == 0:
            if min(x) % 2 == 0:
                x += 1
            else:
                x += 3
        return Tensor(sum(x))
    res = control_flow_if_in_while()
    assert res == 42


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_arm_ascend_training
@pytest.mark.platform_x86_ascend_training
@pytest.mark.env_onecard
def test_if_in_while_built_in_func_3():
    """
    Feature: JIT Fallback
    Description: Test fallback with control flow.
    Expectation: No exception.
    """
    @ms_function
    def control_flow_if_in_while():
        x = np.array([1, 2, 3, 4])
        while sum(x) <= 40:
            if min(x) % 2 == 0:
                x += 1
            else:
                x += 3
            if max(x) == 8:
                break
        return Tensor(sum(x))
    res = control_flow_if_in_while()
    assert res == 26
