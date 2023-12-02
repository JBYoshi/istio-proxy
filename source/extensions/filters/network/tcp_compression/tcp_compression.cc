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

#include "source/extensions/filters/network/tcp_compression/tcp_compression.h"

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

#include "source/extensions/compression/gzip/compressor/zlib_compressor_impl.h"
#include "source/extensions/compression/gzip/decompressor/zlib_decompressor_impl.h"

namespace Envoy {
namespace Tcp {
namespace Compression {

  static const std::string COMPRESSION_PREFIX = "COMPRESS!";

  void print_bytes(const Buffer::Instance& data) {
    std::string bytes(data.length() + 1, '\0');
    data.copyOut(0, data.length(), bytes.data());
    bytes[data.length()] = '\0';

    for (unsigned long i = 0; i < data.length(); i++) {
      if (bytes[i] < ' ') bytes[i] = '.';
    }
    printf("%s\n", bytes.data());
  }

TcpCompressionFilter::TcpCompressionFilter(TcpCompressionConfigSharedPtr config,
                         const LocalInfo::LocalInfo& local_info): config_(config) {
  UNUSED(local_info);

  uint64_t chunk_size = 4096; // TODO tune (this is a default)
  uint64_t max_inflate_ratio = 5000;
  uint64_t window_bits = 15; // TODO tune (this is a default; range 8-15, it's a logarithm)
  uint64_t memory_level = 8; // TODO tune (this is a default)

  std::unique_ptr<Envoy::Extensions::Compression::Gzip::Compressor::ZlibCompressorImpl> compressor_impl = 
    std::make_unique<Envoy::Extensions::Compression::Gzip::Compressor::ZlibCompressorImpl>(
      chunk_size
    );
  std::unique_ptr<Envoy::Extensions::Compression::Gzip::Decompressor::ZlibDecompressorImpl> decompressor_impl =
    std::make_unique<Envoy::Extensions::Compression::Gzip::Decompressor::ZlibDecompressorImpl>(
      config->scope, "decompressor_stats.", chunk_size, max_inflate_ratio
    );
  compressor_impl->init(
    Envoy::Extensions::Compression::Gzip::Compressor::ZlibCompressorImpl::CompressionLevel::Standard,
    Envoy::Extensions::Compression::Gzip::Compressor::ZlibCompressorImpl::CompressionStrategy::Standard,
    window_bits,
    memory_level
  );
  decompressor_impl->init(window_bits);

  compressor = Envoy::Compression::Compressor::CompressorPtr(std::move(compressor_impl));
  decompressor = Envoy::Compression::Decompressor::DecompressorPtr(std::move(decompressor_impl));
}

Network::FilterStatus TcpCompressionFilter::onData(Buffer::Instance& data, bool end_stream) {
  UNUSED(end_stream);
  printf("TcpCompressionFilter::onData\n");
  if (config_->direction == FilterDirection::OUTGOING) {
    printf("TcpCompressionFilter::onData outgoing; will call doDecompress\n");
    doDecompress(data);
  } else {
    printf("TcpCompressionFilter::onData incoming; will cal doDecompress\n");
    doDecompress(data);
  }
  return Network::FilterStatus::Continue;
}

Network::FilterStatus TcpCompressionFilter::onNewConnection() {
  return Network::FilterStatus::Continue;
}

Network::FilterStatus TcpCompressionFilter::onWrite(Buffer::Instance& data, bool end_stream) {
  printf("TcpCompressionFilter::onWrite\n");
  if (config_->direction == FilterDirection::OUTGOING) {
    printf("TcpCompressionFilter::onWrite outgoing; will call doCompress\n");
    doCompress(data, end_stream);
  } else {
    printf("TcpCompressionFilter::onWrite incoming; will call doCompress\n");
    doCompress(data, end_stream);
  }
  return Network::FilterStatus::Continue;
}

void TcpCompressionFilter::doCompress(Buffer::Instance& data, bool end_stream) {
  UNUSED(end_stream);
  printf("doCompress: input is %lu bytes\n", data.length());
  fflush(stdout);
  UNUSED(compressionState);
  bool sentCompressHeader = compressionState != CompressionState::WAITING_FOR_HEADER;
  if (compressionState == CompressionState::WAITING_FOR_HEADER) {
    if (data.startsWith(COMPRESSION_PREFIX)) {
      compressionState = CompressionState::INPUT_COMPRESSED;
    } else {
      compressionState = CompressionState::INPUT_NOT_COMPRESSED;
    }
  }
  if (compressionState == CompressionState::INPUT_NOT_COMPRESSED) {
    compressor->compress(data, end_stream ? Envoy::Compression::Compressor::State::Finish : Envoy::Compression::Compressor::State::Flush);
    printf("doCompress: did compress, output is %lu bytes\n", data.length());
    fflush(stdout);
    if (!sentCompressHeader) {
      data.prepend(COMPRESSION_PREFIX);
      printf("\tbefore prepending prefix\n");
      fflush(stdout);
    }
  }
}
void TcpCompressionFilter::doDecompress(Buffer::Instance& data) {
  printf("doDecompress: input is %lu bytes\n", data.length());
  fflush(stdout);
  UNUSED(decompressionState);
  if (decompressionState == CompressionState::WAITING_FOR_HEADER) {
    if (data.startsWith(COMPRESSION_PREFIX)) {
      printf("\tbefore dropping prefix\n");
      decompressionState = CompressionState::INPUT_COMPRESSED;
      data.drain(COMPRESSION_PREFIX.length());
      fflush(stdout);
    } else {
      decompressionState = CompressionState::INPUT_NOT_COMPRESSED;
    }
  }
  
  if (decompressionState == CompressionState::INPUT_COMPRESSED) {
    Buffer::OwnedImpl out_buf;
    decompressor->decompress(data, out_buf);
    data.drain(data.length());
    data.add(out_buf);
    printf("doDecompress: output is %lu bytes\n", data.length());
    fflush(stdout);
  }
}

} // namespace Compression
} // namespace Tcp
} // namespace Envoy
