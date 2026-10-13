#include "PolyList.h"

#include <cstring>
#include <format>

namespace webo {
namespace {

std::uint32_t u32At(std::span<const std::byte> d, std::size_t o) {
  std::uint32_t v;
  std::memcpy(&v, d.data() + o, 4);
  return v;
}

double f64At(std::span<const std::byte> d, std::size_t o) {
  double v;
  std::memcpy(&v, d.data() + o, 8);
  return v;
}

std::size_t strideFor(std::uint32_t kind) {
  if (kind == 5)
    return 160;
  if (kind == 7)
    return 212;
  return 0;
}

}  // namespace

std::expected<PolyLists, std::string> findPolyLists(std::span<const std::byte> d) {
  PolyLists out;
  constexpr char kName[] = "PolyList 09";
  constexpr std::size_t kNameLen = 11;

  std::vector<std::size_t> hits;
  for (std::size_t i = 0; i + 4 + kNameLen <= d.size();) {
    if (u32At(d, i) == kNameLen &&
        std::memcmp(d.data() + i + 4, kName, kNameLen) == 0) {
      hits.push_back(i);
      i += 4 + kNameLen;
      continue;
    }
    ++i;
  }
  out.rawCount = hits.size();
  if (hits.empty())
    return std::unexpected("no PolyList 09 chunks");

  for (std::size_t hi = 0; hi + 1 < hits.size(); ++hi) {
    const std::size_t start = hits[hi] + 4 + kNameLen;
    const std::size_t next = hits[hi + 1];
    if (next < start + 16)
      continue;
    const std::size_t payload = next - start;
    const std::uint32_t kind = u32At(d, start);
    const std::uint32_t count = u32At(d, start + 4);
    const std::size_t stride = strideFor(kind);
    if (stride == 0 || count == 0 || payload != 16 + stride * count)
      continue;
    if (start + payload > d.size())
      continue;

    PolyListBlock block;
    block.kind = kind;
    block.count = count;
    block.u2 = u32At(d, start + 8);
    block.u3 = u32At(d, start + 12);
    block.offset = start;
    block.vertices.reserve(count);
    for (std::uint32_t n = 0; n < count; ++n) {
      const std::size_t o = start + 16 + n * stride;
      PolyVertex v;
      v.index = u32At(d, o);
      v.u = f64At(d, o + 4);
      v.v = f64At(d, o + 12);
      v.r = f64At(d, o + 20);
      v.g = f64At(d, o + 28);
      v.b = f64At(d, o + 36);
      v.a = f64At(d, o + 44);
      block.vertices.push_back(v);
    }
    out.blocks.push_back(std::move(block));
    ++out.matchedCount;
  }

  if (out.blocks.empty())
    return std::unexpected(
        std::format("found {} PolyList names but none matched the size formula", out.rawCount));
  return out;
}

}  // namespace webo
