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
#include "source/extensions/filters/network/tcp_compression/config/tcp_compression.pb.h"
#include "source/extensions/common/workload_discovery/api.h"
#include "envoy/compression/compressor/compressor.h"
#include "envoy/compression/decompressor/decompressor.h"

namespace Envoy {
namespace Tcp {
namespace Compression {

using ::Envoy::Extensions::Filters::Common::Expr::CelStatePrototype;

// http://stackoverflow.com/questions/3599160#3599170
#define UNUSED(x) (void)(x)

enum class CompressionState {
  WAITING_FOR_HEADER, INPUT_NOT_COMPRESSED, INPUT_COMPRESSED
};

/**
 * Configuration for the TcpCompression filter.
 */
class TcpCompressionConfig {
public:
  TcpCompressionConfig(std::string log_prefix, Stats::Scope& scope): scope(scope), log_prefix(log_prefix) {}

  Stats::Scope& scope;
  std::string log_prefix;

  static const CelStatePrototype& nodeInfoPrototype() {
    static const CelStatePrototype* const prototype = new CelStatePrototype(
        true, ::Envoy::Extensions::Filters::Common::Expr::CelStateType::FlatBuffers,
        ::Wasm::Common::nodeInfoSchema(), StreamInfo::FilterState::LifeSpan::Connection);
    return *prototype;
  }
};

using TcpCompressionConfigSharedPtr = std::shared_ptr<TcpCompressionConfig>;

/**
 * A TcpCompression filter instance. One per connection.
 */
class TcpCompressionFilter : public Network::Filter,
                               protected Logger::Loggable<Logger::Id::filter> {
public:
  TcpCompressionFilter(TcpCompressionConfigSharedPtr config,
                         const LocalInfo::LocalInfo& local_info);

  // Network::ReadFilter
  Network::FilterStatus onData(Buffer::Instance& data, bool end_stream) override;
  Network::FilterStatus onNewConnection() override;
  Network::FilterStatus onWrite(Buffer::Instance& data, bool end_stream) override;
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks) override {
    read_callbacks_ = &callbacks;
  }
  void initializeWriteFilterCallbacks(Network::WriteFilterCallbacks& callbacks) override {
    write_callbacks_ = &callbacks;
  }

private:
  // Config for TcpCompression filter.
  TcpCompressionConfigSharedPtr config_;
  // Read callback instance.
  Network::ReadFilterCallbacks* read_callbacks_{};
  // Write callback instance.
  Network::WriteFilterCallbacks* write_callbacks_{};

  ::Envoy::Compression::Compressor::CompressorPtr compressor;
  ::Envoy::Compression::Decompressor::DecompressorPtr decompressor;

  CompressionState compressionState = CompressionState::WAITING_FOR_HEADER;
  CompressionState decompressionState = CompressionState::WAITING_FOR_HEADER;
};

} // namespace Compression
} // namespace Tcp
} // namespace Envoy
