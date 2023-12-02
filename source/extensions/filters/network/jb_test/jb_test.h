/* Copyright 2019 Istio Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>

#include "envoy/local_info/local_info.h"
#include "envoy/network/filter.h"
#include "envoy/runtime/runtime.h"
#include "envoy/stats/scope.h"
#include "envoy/stats/stats_macros.h"
#include "envoy/stream_info/filter_state.h"
#include "extensions/common/context.h"
#include "extensions/common/node_info_bfbs_generated.h"
#include "extensions/common/proto_util.h"
#include "source/common/common/stl_helpers.h"
#include "source/common/protobuf/protobuf.h"
#include "source/extensions/filters/common/expr/cel_state.h"
#include "source/extensions/filters/network/jb_test/config/jb_test.pb.h"
#include "source/extensions/common/workload_discovery/api.h"

namespace Envoy {
namespace Tcp {
namespace JBTest {

using ::Envoy::Extensions::Filters::Common::Expr::CelStatePrototype;

// http://stackoverflow.com/questions/3599160#3599170
#define UNUSED(x) (void)(x)

/**
 * Configuration for the JBTest filter.
 */
class JBTestConfig {
public:
  JBTestConfig(const std::string prefix);

  const std::string prefix;

  static const CelStatePrototype& nodeInfoPrototype() {
    static const CelStatePrototype* const prototype = new CelStatePrototype(
        true, ::Envoy::Extensions::Filters::Common::Expr::CelStateType::FlatBuffers,
        ::Wasm::Common::nodeInfoSchema(), StreamInfo::FilterState::LifeSpan::Connection);
    return *prototype;
  }
};

using JBTestConfigSharedPtr = std::shared_ptr<JBTestConfig>;

/**
 * A JBTest filter instance. One per connection.
 */
class JBTestFilter : public Network::Filter, public Network::ConnectionCallbacks,
                               protected Logger::Loggable<Logger::Id::filter> {
public:
  JBTestFilter(JBTestConfigSharedPtr config,
                         const LocalInfo::LocalInfo& local_info)
      : config_(config) { UNUSED(local_info); }

  // Network::ReadFilter
  Network::FilterStatus onData(Buffer::Instance& data, bool end_stream) override;
  Network::FilterStatus onNewConnection() override;
  Network::FilterStatus onWrite(Buffer::Instance& data, bool end_stream) override;
  void onEvent(Network::ConnectionEvent event) override;
  void onAboveWriteBufferHighWatermark() override;
  void onBelowWriteBufferLowWatermark() override;
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks) override {
    read_callbacks_ = &callbacks;
    read_callbacks_->connection().addConnectionCallbacks(*this);
  }
  void initializeWriteFilterCallbacks(Network::WriteFilterCallbacks& callbacks) override {
    write_callbacks_ = &callbacks;
  }

private:
  // Config for JBTest filter.
  JBTestConfigSharedPtr config_;
  // Read callback instance.
  Network::ReadFilterCallbacks* read_callbacks_{};
  // Write callback instance.
  Network::WriteFilterCallbacks* write_callbacks_{};

  // Type url of google::protobuf::struct.
  const std::string StructTypeUrl = "type.googleapis.com/google.protobuf.Struct";
};

} // namespace JBTest
} // namespace Tcp
} // namespace Envoy
