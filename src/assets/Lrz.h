// LRZ texture compression. Original decoder routines:
//   .text:00467DE8  lrzDecodeChunk(src, dst, packedSize, unpackedSize)
//   .text:00467EB7  read canonical Huffman depths as nibbles
//   .text:00467F0D  build a tree in the node arena
//   .text:00467FED  decode a number with or without a tree
// A u8 chunk count precedes chunks with u16 packedSize (including the 5-byte header),
// u16 unpackedSize and u8 method (3 stored, 4 LRZ). LRZ has three trees before the bitstream.
// MSB-first codes alternate RAW runs of unaligned literal bytes with COPY references
// (length + 2, distance + 1). Copy bytewise to replicate overlaps. Decoded textures are Windows BMPs.

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

// Decode all chunks or report the failing chunk.
std::expected<std::vector<std::byte>, std::string> lrzDecode(std::span<const std::byte> src);

// Sum unpackedSize fields without decoding.
std::expected<std::size_t, std::string> lrzDecodedSize(std::span<const std::byte> src);

}  // namespace webo
