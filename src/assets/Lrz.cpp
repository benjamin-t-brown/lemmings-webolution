#include "Lrz.h"

#include <cstring>
#include <format>

namespace webo {
namespace {

constexpr std::size_t kMaxSymbols = 17;    // low nibble (<=15) + 2
constexpr std::size_t kArenaBytes = 8192;  // node arena, shared by the three trees

// The original rotates an 8-bit mask and carries into the pointer. Rotating
// right past bit 0 steps forward a byte; rotating left past bit 7 steps back.
inline void maskForward(std::uint8_t& mask, std::size_t& index) {
  const bool wrapped = (mask & 0x01) != 0;
  mask = static_cast<std::uint8_t>((mask >> 1) | (mask << 7));
  if (wrapped)
    ++index;
}

inline void maskBack(std::uint8_t& mask, std::size_t& index) {
  const bool wrapped = (mask & 0x80) != 0;
  mask = static_cast<std::uint8_t>((mask << 1) | (mask >> 7));
  if (wrapped)
    --index;
}

class Decoder {
public:
  Decoder(const std::uint8_t* src, std::size_t srcSize, std::uint8_t* dst, std::size_t dstSize)
      : src_(src), srcSize_(srcSize), dst_(dst), dstSize_(dstSize) {}

  // Returns bytes produced, or 0 on failure (matching the original's contract).
  std::size_t run(std::size_t unpackedSize);

  const std::string& error() const { return error_; }

private:
  // -- source bitstream ---------------------------------------------------
  bool bit() {
    if (srcPos_ >= srcSize_) {
      fail("bitstream ran past the end of the chunk");
      return false;
    }
    return (src_[srcPos_] & bitMask_) != 0;
  }
  void advance() { maskForward(bitMask_, srcPos_); }
  bool takeBit() {
    const bool b = bit();
    advance();
    return b;
  }

  bool byteAvailable() const { return srcBytePos_ < srcSize_; }
  std::uint8_t takeByte() {
    if (srcBytePos_ >= srcSize_) {
      fail("header/tree data ran past the end of the chunk");
      return 0;
    }
    return src_[srcBytePos_++];
  }

  void fail(std::string what) {
    if (error_.empty())
      error_ = std::move(what);
    failed_ = true;
  }

  bool readTree(int index);
  bool buildTree(int index, std::uint8_t count, std::uint8_t maxDepth);
  std::uint32_t decodeNumber(int tree);

  const std::uint8_t* src_;
  std::size_t srcSize_;
  std::uint8_t* dst_;
  std::size_t dstSize_;

  std::size_t srcBytePos_ = 0;  // byte-wise cursor, used for the tree headers
  std::size_t srcPos_ = 0;      // bitstream cursor
  std::uint8_t bitMask_ = 0x80;
  std::size_t dstPos_ = 0;

  // Per-tree state. marker == 0 means "no tree": numbers use the plain code.
  std::uint8_t marker_[3] = {0, 0, 0};
  std::size_t table_[3] = {0, 0, 0};

  std::uint8_t depths_[kMaxSymbols] = {};
  std::uint8_t codeBits_[4] = {};
  std::uint8_t arena_[kArenaBytes] = {};
  std::size_t arenaUsed_ = 0;

  bool failed_ = false;
  std::string error_;
};

bool Decoder::readTree(int index) {
  const std::uint8_t first = takeByte();
  if (failed_)
    return false;

  if ((first & 0x0F) == 0) {
    marker_[index] = first;
    // Only a plain zero means "this tree is unused"; anything else is corrupt.
    if (first != 0) {
      fail(std::format("tree {}: bad header byte 0x{:02x}", index, first));
      return false;
    }
    return true;
  }

  // Low nibble of the first byte is the symbol count, less two. The depth
  // values are nibbles: the high nibble of this first byte, then for every
  // following byte the low nibble and then the high nibble.
  const std::uint8_t count = static_cast<std::uint8_t>((first & 0x0F) + 2);
  std::uint8_t maxDepth = 0;
  std::size_t stored = 0;

  const auto store = [&](std::uint8_t v) {
    if (maxDepth < v)
      maxDepth = v;
    depths_[stored++] = v;
  };

  store(static_cast<std::uint8_t>(first >> 4));
  while (stored < count) {
    const std::uint8_t b = takeByte();
    if (failed_)
      return false;
    store(static_cast<std::uint8_t>(b & 0x0F));
    if (stored >= count)
      break;
    store(static_cast<std::uint8_t>(b >> 4));
  }

  marker_[index] = count;
  return buildTree(index, count, maxDepth);
}

// Walks the code space depth by depth, emitting nodes as it assigns a code to
// each symbol. Nodes are bytes: at node p, bit 0 steps to p + 1 and bit 1 steps
// to p + arena[p]; a zero byte marks a leaf, whose value is the next byte.
bool Decoder::buildTree(int index, std::uint8_t count, std::uint8_t maxDepth) {
  const auto emit = [&](std::uint8_t v) {
    if (arenaUsed_ >= kArenaBytes) {
      fail("tree node arena overflowed");
      return false;
    }
    arena_[arenaUsed_++] = v;
    return true;
  };

  table_[index] = arenaUsed_;
  std::size_t nodeStart = arenaUsed_;
  if (!emit(0))
    return false;

  codeBits_[0] = 0;
  std::size_t cb = 0;          // index into codeBits_
  std::uint8_t dh = 0x80;      // bit within codeBits_[cb]
  std::uint8_t depth = 1;      // ch
  std::uint8_t remaining = 0;  // cl -- symbols left to scan at this depth
  std::size_t sym = 0;         // ebx while scanning depths_

scanDepth:
  sym = 0;
  remaining = count;
  while (true) {
    if (depths_[sym] == depth)
      goto haveSymbol;
    ++sym;
    if (--remaining == 0)
      break;
  }

noSymbolAtDepth:
  if (!emit(0))
    return false;
  maskForward(dh, cb);
  codeBits_[cb] &= static_cast<std::uint8_t>(~dh);
  ++depth;
  if (maxDepth >= depth)
    goto scanDepth;
  return true;

haveSymbol:
  // Leaf: a zero byte then the symbol index.
  if (!emit(0))
    return false;
  if (!emit(static_cast<std::uint8_t>(count - remaining)))
    return false;

  {
    // Advance the canonical code: scan back over set bits for a clear one.
    // Running out means the code space is full and the tree is complete.
    std::uint8_t back = depth;  // al
    while ((codeBits_[cb] & dh) != 0) {
      maskBack(dh, cb);
      if (--back == 0)
        return true;
    }
    codeBits_[cb] |= dh;

    // Walk from the root along that code, materialising nodes as needed.
    // This reuses the shared code cursor rather than a private one: the walk
    // deliberately leaves it at the END of the code just placed, which is
    // where the next symbol's backward scan has to start from.
    std::uint8_t steps = depth;  // dl
    dh = 0x80;
    cb = 0;
    std::size_t node = nodeStart;

    while (true) {
      if (node >= kArenaBytes) {
        fail("tree walk left the arena");
        return false;
      }
      if ((codeBits_[cb] & dh) != 0) {
        const std::uint8_t off = arena_[node];
        if (off == 0) {
          // An unwritten right branch: aim it at the nodes we now append.
          const std::size_t delta = arenaUsed_ - node;
          if (delta >= 0x100) {
            fail("tree branch offset exceeded one byte");
            return false;
          }
          arena_[node] = static_cast<std::uint8_t>(delta);
          while (true) {
            if (--steps == 0)
              goto nextSymbol;
            if (!emit(0))
              return false;
            maskForward(dh, cb);
            codeBits_[cb] &= static_cast<std::uint8_t>(~dh);
          }
        }
        node += off;
      } else {
        ++node;
      }
      maskForward(dh, cb);
      if (--steps == 0) {
        fail("tree walk ran out of code bits");
        return false;
      }
    }
  }

nextSymbol:
  ++sym;
  if (--remaining == 0)
    goto noSymbolAtDepth;
  if (depths_[sym] != depth)
    goto nextSymbol;
  goto haveSymbol;
}

std::uint32_t Decoder::decodeNumber(int tree) {
  if (marker_[tree] == 0) {
    // No tree: a unary length prefix, then that many bits.
    // value = 2^n - 1 + bits
    std::uint32_t n = 0;
    while (bit() && !failed_) {
      ++n;
      advance();
      if (n > 32) {
        fail("unary prefix too long");
        return 0;
      }
    }
    advance();  // the terminating zero
    if (n == 0)
      return 0;
    std::uint32_t acc = 1;
    for (std::uint32_t i = 0; i < n; ++i)
      acc = (acc << 1) | (takeBit() ? 1u : 0u);
    return acc - 1;
  }

  std::size_t node = table_[tree];
  while (true) {
    if (node >= kArenaBytes) {
      fail("decode walked outside the arena");
      return 0;
    }
    if (!takeBit())
      ++node;
    else
      node += arena_[node];
    if (failed_)
      return 0;
    if (node >= kArenaBytes) {
      fail("decode walked outside the arena");
      return 0;
    }
    if (arena_[node] == 0)
      break;
  }
  ++node;
  if (node >= kArenaBytes) {
    fail("leaf value outside the arena");
    return 0;
  }

  const std::uint8_t value = arena_[node];
  if (value < 2)
    return value;

  // value >= 2: read value - 1 further bits.  result = 2^(value-1) + bits
  std::uint32_t acc = 1;
  for (std::uint8_t i = 0, n = static_cast<std::uint8_t>(value - 1); i < n; ++i)
    acc = (acc << 1) | (takeBit() ? 1u : 0u);
  return acc;
}

std::size_t Decoder::run(std::size_t unpackedSize) {
  for (int i = 0; i < 3; ++i)
    if (!readTree(i))
      return 0;

  // The bitstream begins at the first byte after the trees.
  srcPos_ = srcBytePos_;
  bitMask_ = 0x80;

  // The original counts down from unpackedSize - 1 and stops on borrow.
  std::int64_t remaining = static_cast<std::int64_t>(unpackedSize) - 1;

  while (true) {
    const std::uint32_t literals = decodeNumber(0);
    if (failed_)
      return 0;

    if (literals != 0) {
      for (std::uint32_t i = 0; i < literals; ++i) {
        std::uint32_t acc = 1;
        while ((acc & 0x100) == 0)
          acc = (acc << 1) | (takeBit() ? 1u : 0u);
        if (failed_)
          return 0;
        if (dstPos_ >= dstSize_) {
          fail("literal run overran the output buffer");
          return 0;
        }
        dst_[dstPos_++] = static_cast<std::uint8_t>(acc & 0xFF);
      }
      remaining -= static_cast<std::int64_t>(literals);
      if (remaining < 0)
        break;
    }

    const std::uint32_t length = decodeNumber(1);
    const std::uint32_t distance = decodeNumber(2);
    if (failed_)
      return 0;

    const std::size_t count = length + 2;
    const std::size_t back = static_cast<std::size_t>(distance) + 1;
    if (back > dstPos_) {
      fail(std::format("copy reaches {} bytes back with only {} written", back, dstPos_));
      return 0;
    }
    if (dstPos_ + count > dstSize_) {
      fail("copy overran the output buffer");
      return 0;
    }
    // Byte at a time: overlapping copies are meant to replicate.
    std::size_t from = dstPos_ - back;
    for (std::size_t i = 0; i < count; ++i)
      dst_[dstPos_++] = dst_[from++];

    remaining -= static_cast<std::int64_t>(count);
    if (remaining < 0)
      break;
  }

  return dstPos_;
}

struct ChunkHeader {
  std::uint16_t packed;
  std::uint16_t unpacked;
  std::uint8_t method;
};

std::expected<std::vector<ChunkHeader>, std::string> scanChunks(std::span<const std::byte> src) {
  if (src.empty())
    return std::unexpected("empty entry");

  const auto* p = reinterpret_cast<const std::uint8_t*>(src.data());
  const std::uint8_t chunks = p[0];
  if (chunks == 0)
    return std::unexpected("chunk count is zero");

  std::vector<ChunkHeader> out;
  out.reserve(chunks);
  std::size_t pos = 1;
  for (std::uint8_t i = 0; i < chunks; ++i) {
    if (pos + 5 > src.size())
      return std::unexpected(std::format("chunk {} header runs past the entry", i));
    ChunkHeader h;
    std::memcpy(&h.packed, p + pos, 2);
    std::memcpy(&h.unpacked, p + pos + 2, 2);
    h.method = p[pos + 4];
    if (h.packed < 5)
      return std::unexpected(std::format("chunk {} claims packed size {}", i, h.packed));
    if (pos + h.packed > src.size())
      return std::unexpected(std::format("chunk {} runs past the entry", i));
    out.push_back(h);
    pos += h.packed;
  }
  return out;
}

}  // namespace

std::expected<std::size_t, std::string> lrzDecodedSize(std::span<const std::byte> src) {
  auto chunks = scanChunks(src);
  if (!chunks)
    return std::unexpected(chunks.error());
  std::size_t total = 0;
  for (const auto& c : *chunks)
    total += c.unpacked;
  return total;
}

std::expected<std::vector<std::byte>, std::string> lrzDecode(std::span<const std::byte> src) {
  auto chunks = scanChunks(src);
  if (!chunks)
    return std::unexpected(chunks.error());

  std::size_t total = 0;
  for (const auto& c : *chunks)
    total += c.unpacked;

  std::vector<std::byte> out(total);
  const auto* p = reinterpret_cast<const std::uint8_t*>(src.data());
  auto* o = reinterpret_cast<std::uint8_t*>(out.data());

  std::size_t pos = 1;
  std::size_t written = 0;
  for (std::size_t i = 0; i < chunks->size(); ++i) {
    const ChunkHeader& h = (*chunks)[i];
    const std::uint8_t* body = p + pos + 5;
    const std::size_t bodySize = h.packed - 5u;

    if (h.method == 3) {
      if (bodySize != h.unpacked)
        return std::unexpected(
            std::format("chunk {}: stored size {} != unpacked {}", i, bodySize, h.unpacked));
      std::memcpy(o + written, body, bodySize);
      written += bodySize;
    } else if (h.method == 4) {
      Decoder d(body, bodySize, o + written, h.unpacked);
      const std::size_t n = d.run(h.unpacked);
      if (n != h.unpacked) {
        return std::unexpected(std::format("chunk {}: produced {} of {} bytes{}", i, n, h.unpacked,
                                           d.error().empty() ? "" : " -- " + d.error()));
      }
      written += n;
    } else {
      return std::unexpected(std::format("chunk {}: unknown method {}", i, h.method));
    }
    pos += h.packed;
  }

  out.resize(written);
  return out;
}

}  // namespace webo
