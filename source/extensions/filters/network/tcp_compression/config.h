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

#include "envoy/server/filter_config.h"
#include "source/extensions/filters/network/tcp_compression/config/tcp_compression.pb.h"

namespace Envoy {
namespace Tcp {
namespace Compression {

/**
 * Config registration for the TcpCompression filter. @see
 *  NamedNetworkFilterConfigFactory.
 */
class TcpCompressionConfigFactory
    : public Server::Configuration::NamedNetworkFilterConfigFactory {
public:
  Network::FilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message&,
                               Server::Configuration::FactoryContext&) override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override;

  std::string name() const override { return "envoy.filters.network.tcp_compression"; }

private:
  Network::FilterFactoryCb
  createFilterFactory(const envoy::tcp::compression::config::TcpCompression& proto_config,
                      Server::Configuration::FactoryContext& context);
};

/**
 * Config registration for the JBTest Upstream filter. @see
 *  NamedUpstreamNetworkFilterConfigFactory.
 */
 // TODO: This class *should* be removable, but whenever I've tried, it seems to break the
 // downstream handler registration.
class TcpCompressionUpstreamConfigFactory
    : public Server::Configuration::NamedUpstreamNetworkFilterConfigFactory {
public:
  Network::FilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message&,
                               Server::Configuration::UpstreamFactoryContext&) override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override;

  std::string name() const override { return "envoy.filters.network.upstream.tcp_compression"; }

private:
  Network::FilterFactoryCb
  createFilterFactory(const envoy::tcp::compression::config::TcpCompression& proto_config,
                      Server::Configuration::UpstreamFactoryContext& context);
};

} // namespace Compression
} // namespace Tcp
} // namespace Envoy
