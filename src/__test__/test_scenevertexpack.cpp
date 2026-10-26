#include "app/SceneVertexPack.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <span>
#include <sstream>
#include <string>

using webo::app::SceneVertexPackInput;

namespace {

void check(const SceneVertexPackInput& input,
           const std::array<std::uint8_t, 28>& expected) {
  std::array<std::byte, 32> bytes;
  for (std::size_t i = 0; i < bytes.size(); ++i)
    bytes[i] = std::byte((0xa5u + i * 37u) & 0xffu);
  const std::array<std::byte, 4> canary{bytes[20], bytes[21], bytes[22], bytes[23]};
  auto packed = webo::app::packSceneVertex(input, bytes);
  assert(packed);
  for (std::size_t i = 0; i < 20; ++i)
    assert(std::to_integer<std::uint8_t>(bytes[i]) == expected[i]);
  for (std::size_t i = 0; i < 4; ++i) assert(bytes[20 + i] == canary[i]);
  for (std::size_t i = 24; i < 32; ++i)
    assert(std::to_integer<std::uint8_t>(bytes[i]) == expected[i - 4]);
}

unsigned hexByte(const std::string& s, std::size_t at) {
  return static_cast<unsigned>(std::stoul(s.substr(at, 2), nullptr, 16));
}

bool fixture(const char* path) {
  std::ifstream in(path);
  if (!in) return false;
  std::string line;
  std::size_t rows = 0;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream row(line);
    SceneVertexPackInput input{};
    std::string known;
    if (!(row >> input.screenX >> input.screenY >> input.screenZ >> input.diffuse[0] >>
          input.diffuse[1] >> input.diffuse[2] >> input.diffuse[3] >> input.u >>
          input.v >> input.materialUOffset >> input.materialVOffset >> known) ||
        known.size() != 56)
      return false;
    std::array<std::uint8_t, 28> expected{};
    for (std::size_t i = 0; i < expected.size(); ++i)
      expected[i] = static_cast<std::uint8_t>(hexByte(known, i * 2));
    check(input, expected);
    ++rows;
  }
  std::cout << "candidate fixture records=" << rows << '\n';
  return rows == 2077;
}

}  // namespace

int main(int argc, char** argv) {
  // Deliberately asymmetric inputs discriminate field order, raw-colour
  // truncation, RHW's f32-Z reload, UV offsets, and preservation of specular.
  check({12.25, 34.75, 0.123456789, {1.99, 17.75, 129.5, 254.9},
         0.125, 0.875, 0.25, -0.125},
        {0x00,0x00,0x44,0x41, 0x00,0x00,0x0b,0x42, 0xea,0xd6,0xfc,0x3d,
         0x9a,0x99,0x01,0x40, 0x81,0x11,0x01,0xfe, 0x00,0x00,0xc0,0x3e,
         0x00,0x00,0x40,0x3f});
  check({1.0, 2.0, 1.0, {1.0, 1.0, 1.0, 1.0}, 0.25, 0.75, 0.0, 0.0},
        {0x00,0x00,0x80,0x3f, 0x00,0x00,0x00,0x40, 0x00,0x00,0x80,0x3f,
         0x00,0x00,0x80,0x3e, 0x01,0x01,0x01,0x01, 0x00,0x00,0x80,0x3e,
         0x00,0x00,0x40,0x3f});

  if (argc == 2) return fixture(argv[1]) ? 0 : 1;
  std::array<std::byte, 32> unchanged;
  unchanged.fill(std::byte{0x6d});
  auto invalid = webo::app::packSceneVertex(
      {0, 0, 0, {1, 2, 3, 4}, 0, 0, 0, 0}, unchanged);
  assert(!invalid);
  for (auto b : unchanged) assert(b == std::byte{0x6d});
}
