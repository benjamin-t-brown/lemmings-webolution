// scene-branch-impl-001: asset-free formula tests for the branch-1 sprite scenery state.
// Every expectation is a hand-computed literal, not a call into the code under test.
// Source-built evidence only: no original code, no claim about original flame phases.
#include "app/CrtRand.h"
#include "app/SceneBillboardSource.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <string_view>

using namespace webo::app;
namespace {
int checks = 0, failures = 0;
void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) { ++failures; std::cerr << " FAIL " << what << '\n'; }
}

void testCrt() {
  // MSVC CRT sequence for srand(1) (0x0049C84A): state*0x343FD+0x269EC3, (int32(state)>>16)&0x7FFF.
  CrtRand r;
  check(r.state() == 1, "default state is the CRT image value 1");
  const int expected[] = {41, 18467, 6334, 26500, 19169, 15724, 11478, 29358, 26962, 24464};
  bool same = true;
  for (int e : expected) same = same && r.rand() == e;
  check(same, "first ten rand() values from state 1 are 41,18467,6334,26500,19169,15724,11478,29358,26962,24464");
  r.srand(1);
  check(r.rand() == 41 && r.state() == 2745024u, "srand(1): first state 1*214013+2531011 = 2745024, value 41");
  CrtRand u(1);
  check(u.unit() == 41.0 / 32767.0, "unit() is rand()/32767.0 (0x004040A0, constant 0x004B03B0)");
  // seedFromMs = srand(uint32(int64(trunc(ms*32767.0)))) (0x00447580 + ftol 0x0049C5AC).
  CrtRand s;
  s.seedFromMs(1000.0);
  check(s.state() == 32767000u, "seedFromMs(1000) = 32767000");
  s.seedFromMs(1000000.0);
  check(s.state() == 2702228928u, "seedFromMs(1e6): 32767000000 mod 2^32 = 2702228928 (wraps, no saturation)");
  s.seedFromMs(0.4);
  check(s.state() == 13106u, "seedFromMs truncates: 0.4*32767 = 13106.8 -> 13106");
  s.seedFromMs(std::numeric_limits<double>::quiet_NaN());
  check(s.state() == 0u, "NaN stores the fistp integer-indefinite pattern, low 32 bits 0");
}

void testLatch() {
  SpriteTemporalState st;
  check(!st.firstDrawDone && st.temporal == 0 && st.accumulator == 0.0 && st.mode == 1 && st.loop,
        "constructor state: temporal 0, accumulator 0, no latch, mode 1, loop (0x004680F0)");
  check(latchSpriteFirstDrawFrame(st, 16, 0.5) && st.temporal == 7 && st.firstDrawDone,
        "first draw: trunc(15*0.5) = 7, latch set");
  check(!latchSpriteFirstDrawFrame(st, 16, 0.99) && st.temporal == 7, "latch is once per node");
  SpriteTemporalState top; latchSpriteFirstDrawFrame(top, 16, 1.0);
  check(top.temporal == 15, "rand()==32767 reaches count-1 = 15 (range [0,count-1])");
  SpriteTemporalState zero; latchSpriteFirstDrawFrame(zero, 16, 0.0);
  check(zero.temporal == 0, "rand()==0 gives 0");
  SpriteTemporalState one; latchSpriteFirstDrawFrame(one, 1, 0.9);
  check(one.temporal == 0, "single-entry clip always picks 0");
  // Worked example with CRT state 1 (16 entries): 41/32767*15 -> 0, 18467/32767*15 = 8.45 -> 8,
  // 6334/32767*15 = 2.9 -> 2, 26500/32767*15 = 12.13 -> 12.
  CrtRand r;
  const std::uint32_t want[] = {0, 8, 2, 12};
  bool same = true;
  for (std::uint32_t w : want) { SpriteTemporalState n; latchSpriteFirstDrawFrame(n, 16, r.unit()); same = same && n.temporal == w; }
  check(same, "state-1 draws map to frames 0,8,2,12");
}

void testAdvance() {
  SpriteTemporalState s;
  advanceSpriteTemporal(s, 0.1, 1.0, 16);
  check(s.temporal == 1 && s.accumulator > 0.039 && s.accumulator < 0.041 && !s.finished,
        "dt 0.1, rate 1: one step (0.1>0.06), remainder 0.04");
  advanceSpriteTemporal(s, 0.1, 1.0, 16);
  check(s.temporal == 3 && s.accumulator > 0.019 && s.accumulator < 0.021, "second 0.1: 0.14 gives two steps, remainder 0.02");
  SpriteTemporalState exact;
  advanceSpriteTemporal(exact, 0.12, 1.0, 16);
  check(exact.temporal == 1 && exact.accumulator == 0.06, "0.12 -> one step; acc 0.06 is not > 0.06 (strict)");
  SpriteTemporalState eps;
  advanceSpriteTemporal(eps, 0.060000005, 1.0, 16);
  check(eps.temporal == 1 && eps.accumulator == 0.0, "remainder 5e-9 <= 1e-8 is zeroed");
  SpriteTemporalState wrap; wrap.temporal = 15;
  advanceSpriteTemporal(wrap, 0.07, 1.0, 16);
  check(wrap.temporal == 0 && wrap.finished && wrap.accumulator > 0.0099 && wrap.accumulator < 0.0101,
        "loop wrap 15 -> 0 sets the finished flag");
  advanceSpriteTemporal(wrap, 0.0, 1.0, 16);
  check(!wrap.finished, "finished flag is cleared on the next advance (+0x200 = 0)");
  SpriteTemporalState once; once.temporal = 15; once.loop = false;
  advanceSpriteTemporal(once, 0.07, 1.0, 16);
  check(once.temporal == 15 && once.finished, "non-looping clip holds count-1");
  SpriteTemporalState fast;
  advanceSpriteTemporal(fast, 0.05, 2.0, 16);
  check(fast.temporal == 1 && fast.accumulator > 0.039 && fast.accumulator < 0.041, "clip rate scales dt: 0.05*2 = 0.1");
  SpriteTemporalState idle;
  advanceSpriteTemporal(idle, 0.0, 1.0, 16);
  check(idle.temporal == 0 && idle.accumulator == 0.0, "dt 0 (paused / not in play) changes nothing");
  SpriteTemporalState bad; bad.mode = 0;
  advanceSpriteTemporal(bad, 1.0, 1.0, 16);
  check(bad.temporal == 0, "only the constructor mode 1 is implemented");
}
}  // namespace

int main() {
  testCrt();
  testLatch();
  testAdvance();
  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? 1 : 0;
}
