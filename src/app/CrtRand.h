#pragma once

#include <cstdint>

namespace webo::app {

// The MSVC CRT linear congruential generator the reference executable uses at 0x0049C84A
// (rand) and 0x0049C840 (srand), global state at 0x004C4570 (image value 1):
//   state = state * 0x343FD + 0x269EC3;  return (int32(state) >> 16) & 0x7FFF;
// 0x004040A0 scales it as rand() / 32767.0 (constant at 0x004B03B0), and 0x00447580 seeds it as
// srand(ftol(ms * 32767.0)) with ftol = truncating fistp, low 32 bits (0x0049C5AC).
// The default state (1) exists so unit tests can construct an object; production seeds the
// process generator from its own millisecond clock (see main.cpp). Source-built support only:
// the original seed is wall-clock derived and not reproducible.
class CrtRand {
 public:
  CrtRand() = default;
  explicit CrtRand(std::uint32_t state) : state_(state) {}

  void srand(std::uint32_t seed) { state_ = seed; }
  // srand(uint32(int64(trunc(ms * 32767.0)))). Values the x87 fistp cannot represent store the
  // "integer indefinite" pattern 0x8000000000000000, whose low 32 bits are zero.
  void seedFromMs(double ms);
  int rand() {
    state_ = state_ * 0x343FDu + 0x269EC3u;
    return static_cast<int>((static_cast<std::int32_t>(state_) >> 16) & 0x7FFF);
  }
  // 0x004040A0: rand() / 32767.0, in [0, 1].
  double unit() { return static_cast<double>(rand()) / 32767.0; }
  std::uint32_t state() const { return state_; }

 private:
  std::uint32_t state_ = 1;
};

}  // namespace webo::app
