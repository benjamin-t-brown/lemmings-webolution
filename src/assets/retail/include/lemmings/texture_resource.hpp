#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace lemmings {

// Normal valid-stream behavior of 0x467DE8 / 0x467E13 / 0x467EB7 / 0x467FED.
// Three Huffman/unary tables encode literal runs, match lengths, and distances.
// Malformed/truncated streams, invalid tables/backreferences, and writes past
// expected_size throw std::invalid_argument rather than emulate memory faults.
std::vector<std::uint8_t> decode_texture_block(std::span<const std::uint8_t>,
                                             std::size_t expected_size);

// The in-memory packed bitmap transformation in 0x45015A after file read/close.
// BM-prefixed data is returned unchanged. Otherwise: u8 block count, followed
// by {u16 packed_bytes_including_header,u16 output_bytes,u8 codec,payload}.
// Codec 3 copies raw bytes; codec 4 uses decode_texture_block. Trailing input is
// ignored as in the original. Valid raw blocks require payload==output size.
// File I/O, allocation callbacks/failure handling, GDI and GPU upload excluded.
std::vector<std::uint8_t> decode_texture_bitmap(std::span<const std::uint8_t>);

} // namespace lemmings
