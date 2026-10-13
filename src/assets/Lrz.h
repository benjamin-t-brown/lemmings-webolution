// LRZ -- the compression used by Lemmings Revolution for texture entries.
//
// Transcribed from the hand-written assembly in the original executable:
//
//     .text:00467DE8  lrzDecodeChunk(src, dst, packedSize, unpackedSize)
//     .text:00467EB7  read one canonical-Huffman tree (nibble depth list)
//     .text:00467F0D  build the tree into the node arena
//     .text:00467FED  decode one number, with or without a tree
//
// A compressed entry is a one-byte chunk count followed by that many chunks:
//
//     u16 packedSize      total chunk size, the 5-byte header included
//     u16 unpackedSize
//     u8  method          3 = stored (plain memcpy), 4 = LRZ
//     ... three trees, then the bitstream ...
//
// The decompressed result of a texture entry is an ordinary Windows BMP.
//
// Bits are read most-significant first. Each chunk codes an alternating
// sequence of RAW runs (a count, then that many literal bytes, themselves
// read 8 bits at a time from the bitstream rather than byte-aligned) and
// COPY references (length + 2, distance + 1, copied byte by byte so that
// overlapping runs replicate).

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

// Decodes every chunk of one compressed entry. Returns the decompressed bytes,
// or a message describing where it gave up.
std::expected<std::vector<std::byte>, std::string> lrzDecode(std::span<const std::byte> src);

// Size of the decoded result without decoding it: the sum of the chunk
// unpackedSize fields. Cheap; useful for sizing and for sanity checks.
std::expected<std::size_t, std::string> lrzDecodedSize(std::span<const std::byte> src);

}  // namespace webo
