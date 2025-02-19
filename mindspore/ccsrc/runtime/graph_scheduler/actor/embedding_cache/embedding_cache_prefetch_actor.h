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

#ifndef MINDSPORE_CCSRC_RUNTIME_GRAPH_SCHEDULER_ACTOR_EMBEDDING_CACHE_EMBEDDING_CACHE_PREFETCH_ACTOR_H_
#define MINDSPORE_CCSRC_RUNTIME_GRAPH_SCHEDULER_ACTOR_EMBEDDING_CACHE_EMBEDDING_CACHE_PREFETCH_ACTOR_H_

#include <map>
#include <memory>
#include <string>

#include "runtime/graph_scheduler/actor/actor_common.h"
#include "runtime/graph_scheduler/actor/rpc/send_actor.h"
#include "runtime/graph_scheduler/actor/rpc/rpc_actor.h"

namespace mindspore {
namespace runtime {
// The EmbeddingCachePrefetchActor is used to cache large embedding table scenarios. The cache level is: Device
// Cache->Local Host Cache->Remote Cache. This Actor is used to perform Local and Device Cache hit analysis and cache
// prefetching (the feature weights corresponding to the ids of subsequent batches are assigned in advance Prefetching
// into the Device Cache, so that it is pipelined with the calculation on the Device side), cache prefetching may
// involve RPC communication with the Server side.
class EmbeddingCachePrefetchActor : public ActorBase {
 public:
  explicit EmbeddingCachePrefetchActor(const device::DeviceContext *device_context)
      : ActorBase("EmbeddingCachePrefetchActor"), device_context_(device_context) {}

  ~EmbeddingCachePrefetchActor() override;

  // Initialize embedding cache prefetch actor.
  // 1. Build and Link rpc actors between local cache and remote cache.
  // 2. Build network connection of rpc actors.
  void Initialize();

  // Perform local cache hit analysis, prefetch the feature vector corresponding to the next batch into the cache.
  void Run();

  // Finalize embedding cache prefetch actor and push latest embedding from local cache to remote cache.
  void Finalize();

 private:
  // The cache prefetch phase may involve RPC communication with the server, implemented through Send Actor and
  // Recv Actor.
  // Build rpc actors.
  void BuildRpcActors();
  // Link rpc actors by inter-process arrows.
  void LinkRpcActors();

  // Record Send Actor and Recv Actor.
  // Key: Inter process edge(Parameter name), Value: Send Actor.
  std::map<std::string, SendActorPtr> send_actors_;
  // Key: Inter process edge(Parameter name), Value: Recv Actor.
  std::map<std::string, RecvActorPtr> recv_actors_;

  // The device interface.
  const device::DeviceContext *device_context_;
};

using EmbeddingCachePrefetchActorPtr = std::shared_ptr<EmbeddingCachePrefetchActor>;
}  // namespace runtime
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_RUNTIME_GRAPH_SCHEDULER_ACTOR_EMBEDDING_CACHE_EMBEDDING_CACHE_PREFETCH_ACTOR_H_
