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

#include "source/extensions/filters/network/jb_test/config.h"

#include "envoy/network/connection.h"
#include "envoy/registry/registry.h"
#include "source/extensions/filters/network/jb_test/jb_test.h"

namespace Envoy {
namespace Tcp {
namespace JBTest {
namespace {

Network::FilterFactoryCb createFilterFactoryHelper(
    const envoy::tcp::jb_test::config::JBTest& proto_config,
    Server::Configuration::ServerFactoryContext& context, FilterDirection filter_direction) {
  UNUSED(filter_direction);
  JBTestConfigSharedPtr filter_config(std::make_shared<JBTestConfig>(proto_config.prefix()));
  return [filter_config, &context](Network::FilterManager& filter_manager) -> void {
    filter_manager.addFilter(
        std::make_shared<JBTestFilter>(filter_config, context.localInfo()));
  };
}
} // namespace

Network::FilterFactoryCb JBTestConfigFactory::createFilterFactoryFromProto(
    const Protobuf::Message& config, Server::Configuration::FactoryContext& context) {
  return createFilterFactory(
      dynamic_cast<const envoy::tcp::jb_test::config::JBTest&>(config), context);
}

ProtobufTypes::MessagePtr JBTestConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::tcp::jb_test::config::JBTest>();
}

Network::FilterFactoryCb JBTestConfigFactory::createFilterFactory(
    const envoy::tcp::jb_test::config::JBTest& proto_config,
    Server::Configuration::FactoryContext& context) {
  return createFilterFactoryHelper(proto_config, context.getServerFactoryContext(),
                                   FilterDirection::Downstream);
}

Network::FilterFactoryCb JBTestUpstreamConfigFactory::createFilterFactoryFromProto(
    const Protobuf::Message& config, Server::Configuration::UpstreamFactoryContext& context) {
  return createFilterFactory(
      dynamic_cast<const envoy::tcp::jb_test::config::JBTest&>(config), context);
}

ProtobufTypes::MessagePtr JBTestUpstreamConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::tcp::jb_test::config::JBTest>();
}

Network::FilterFactoryCb JBTestUpstreamConfigFactory::createFilterFactory(
    const envoy::tcp::jb_test::config::JBTest& proto_config,
    Server::Configuration::UpstreamFactoryContext& context) {
  return createFilterFactoryHelper(proto_config, context.getServerFactoryContext(),
                                   FilterDirection::Upstream);
}

/**
 * Static registration for the JBTest Downstream filter. @see
 * RegisterFactory.
 */
static Registry::RegisterFactory<JBTestConfigFactory,
                                 Server::Configuration::NamedNetworkFilterConfigFactory>
    registered_;

/**
 * Static registration for the JBTest Upstream filter. @see
 * RegisterFactory.
 */
static Registry::RegisterFactory<JBTestUpstreamConfigFactory,
                                 Server::Configuration::NamedUpstreamNetworkFilterConfigFactory>
    registered_upstream_;

} // namespace JBTest
} // namespace Tcp
} // namespace Envoy
