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

#ifndef MINDSPORE_CCSRC_DISTRIBUTED_CLUSTER_TOPOLOGY_COMMON_H_
#define MINDSPORE_CCSRC_DISTRIBUTED_CLUSTER_TOPOLOGY_COMMON_H_

#include <string>
#include <chrono>

namespace mindspore {
namespace distributed {
namespace cluster {
namespace topology {
// The address of meta server node used by compute graph nodes to register and get addresses of other compute graph
// nodes dynamically.
struct MetaServerAddress {
  std::string GetUrl() { return ip + ":" + std::to_string(port); }
  std::string ip;
  int port;
};

// The address of meta server node.
// This address is set or obtained through environment variables.
constexpr char kEnvMetaServerHost[] = "MS_SCHED_HOST";
constexpr char kEnvMetaServerPort[] = "MS_SCHED_PORT";

constexpr char kEnvNodeId[] = "MS_NODE_ID";

// For port number conversion.
static const int kDecimal = 10;

// The timeout for initializing the cluster topology.
static const std::chrono::milliseconds kTopoInitTimeout = std::chrono::milliseconds(1000 * 60 * 10);

// All kinds of messages sent between compute graph nodes and meta server node.
enum class MessageName {
  kRegistration,
  kUnregistration,
  kHeartbeat,
  kSuccess,
  kInvalidNode,
  kUninitTopo,
  kWriteMetadata,
  kReadMetadata,
  kValidMetadata,
  kInvalidMetadata
};

// The retry and interval configuration used for the macro `EXECUTE_WITH_RETRY`.
static const size_t kExecuteRetryNum = 30;
static const uint32_t kExecuteInterval = 10;

#define EXECUTE_WITH_RETRY(func, retry, interval, err_msg)                   \
  do {                                                                       \
    bool success = false;                                                    \
    for (size_t i = 1; i <= retry; ++i) {                                    \
      success = func();                                                      \
      if (!success) {                                                        \
        MS_LOG(ERROR) << err_msg << ", retry(" << i << "/" << retry << ")."; \
        sleep(interval);                                                     \
      } else {                                                               \
        break;                                                               \
      }                                                                      \
    }                                                                        \
    if (!success) {                                                          \
      return false;                                                          \
    }                                                                        \
  } while (false)
}  // namespace topology
}  // namespace cluster
}  // namespace distributed
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_DISTRIBUTED_CLUSTER_TOPOLOGY_COMMON_H_
