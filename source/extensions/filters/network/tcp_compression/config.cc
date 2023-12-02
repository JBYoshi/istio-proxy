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

#include "source/extensions/filters/network/tcp_compression/config.h"

#include "envoy/network/connection.h"
#include "envoy/registry/registry.h"
#include "source/extensions/filters/network/tcp_compression/tcp_compression.h"

namespace Envoy {
namespace Tcp {
namespace Compression {
namespace {

Network::FilterFactoryCb createFilterFactoryHelper(
    const envoy::tcp::compression::config::TcpCompression& proto_config,
    Server::Configuration::ServerFactoryContext& context) {
  UNUSED(proto_config);
  TcpCompressionConfigSharedPtr filter_config(std::make_shared<TcpCompressionConfig>(context.scope()));
  return [filter_config, &context](Network::FilterManager& filter_manager) -> void {
    filter_manager.addFilter(
        std::make_shared<TcpCompressionFilter>(filter_config, context.localInfo()));
  };
}
} // namespace

Network::FilterFactoryCb TcpCompressionConfigFactory::createFilterFactoryFromProto(
    const Protobuf::Message& config, Server::Configuration::FactoryContext& context) {
  return createFilterFactory(
      dynamic_cast<const envoy::tcp::compression::config::TcpCompression&>(config), context);
}

ProtobufTypes::MessagePtr TcpCompressionConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::tcp::compression::config::TcpCompression>();
}

Network::FilterFactoryCb TcpCompressionConfigFactory::createFilterFactory(
    const envoy::tcp::compression::config::TcpCompression& proto_config,
    Server::Configuration::FactoryContext& context) {
  return createFilterFactoryHelper(proto_config, context.getServerFactoryContext());
}

Network::FilterFactoryCb TcpCompressionUpstreamConfigFactory::createFilterFactoryFromProto(
    const Protobuf::Message& config, Server::Configuration::UpstreamFactoryContext& context) {
  return createFilterFactory(
      dynamic_cast<const envoy::tcp::compression::config::TcpCompression&>(config), context);
}

ProtobufTypes::MessagePtr TcpCompressionUpstreamConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::tcp::compression::config::TcpCompression>();
}

Network::FilterFactoryCb TcpCompressionUpstreamConfigFactory::createFilterFactory(
    const envoy::tcp::compression::config::TcpCompression& proto_config,
    Server::Configuration::UpstreamFactoryContext& context) {
  return createFilterFactoryHelper(proto_config, context.getServerFactoryContext());
}

/**
 * Static registration for the TcpCompression filter. @see
 * RegisterFactory.
 */
static Registry::RegisterFactory<TcpCompressionConfigFactory,
                                 Server::Configuration::NamedNetworkFilterConfigFactory>
    registered_;
  
/**
 * Static registration for the TcpCompression Upstream filter. @see
 * RegisterFactory.
 */
static Registry::RegisterFactory<TcpCompressionUpstreamConfigFactory,
                                 Server::Configuration::NamedUpstreamNetworkFilterConfigFactory>
    registered_upstream_;

} // namespace Compression
} // namespace Tcp
} // namespace Envoy
