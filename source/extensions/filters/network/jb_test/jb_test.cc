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

#include "source/extensions/filters/network/jb_test/jb_test.h"

#include <cstdint>
#include <string>

#include <cstdio>

#include "absl/base/internal/endian.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "envoy/network/connection.h"
#include "envoy/stats/scope.h"
#include "source/common/buffer/buffer_impl.h"
#include "source/common/protobuf/utility.h"

namespace Envoy {
namespace Tcp {
namespace JBTest {

  void print_bytes(const Buffer::Instance& data) {
    std::string bytes(data.length() + 1, '\0');
    data.copyOut(0, data.length(), bytes.data());
    bytes[data.length()] = '\0';

    for (unsigned long i = 0; i < data.length(); i++) {
      if (bytes[i] < ' ' || bytes[i] >= 127) bytes[i] = '.';
    }
    printf("%s\n", bytes.data());
  }

JBTestConfig::JBTestConfig(
  const std::string prefix)
    : prefix(prefix) {
}

Network::FilterStatus JBTestFilter::onData(Buffer::Instance& data, bool end_stream) {
  printf("JBTestFilter#%lu(%s)::onData(%lu bytes, %s)\n", read_callbacks_->connection().id(), config_->prefix.c_str(), data.length(), end_stream ? "EOF" : "continue");
  print_bytes(data);
  fflush(stdout);
  return Network::FilterStatus::Continue;
}

Network::FilterStatus JBTestFilter::onNewConnection() {
  printf("JBTestFilter#%lu(%s)::onNewConnection\n", read_callbacks_->connection().id(), config_->prefix.c_str());
  fflush(stdout);
  return Network::FilterStatus::Continue;
}

Network::FilterStatus JBTestFilter::onWrite(Buffer::Instance& data, bool end_stream) {
  printf("JBTestFilter#%lu(%s)::onWrite(%lu bytes, %s)\n", read_callbacks_->connection().id(), config_->prefix.c_str(), data.length(), end_stream ? "EOF" : "continue");
  print_bytes(data);
  fflush(stdout);
  return Network::FilterStatus::Continue;
}

void JBTestFilter::onEvent(Network::ConnectionEvent event) {
  std::string event_desc = "?";
  if (event == Network::ConnectionEvent::RemoteClose) event_desc = "RemoteClose";
  else if (event == Network::ConnectionEvent::LocalClose) event_desc = "LocalClose";
  else if (event == Network::ConnectionEvent::Connected) event_desc = "Connected";
  else if (event == Network::ConnectionEvent::ConnectedZeroRtt) event_desc = "ConnectedZeroRtt";
  printf("JBTestFilter#%lu(%s)::onEvent(%s)\n", read_callbacks_->connection().id(), config_->prefix.c_str(), event_desc.c_str());
}
void JBTestFilter::onAboveWriteBufferHighWatermark() {}
void JBTestFilter::onBelowWriteBufferLowWatermark() {}

} // namespace JBTest
} // namespace Tcp
} // namespace Envoy
