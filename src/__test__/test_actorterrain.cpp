#include "app/ActorTerrain.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

bool nearAbs(double a, double b, double tol) {
  return std::abs(a - b) <= tol;
}

void checkSample(const webo::ActorTerrainSample& got, double x, double y, double z,
                 std::string_view label) {
  check(nearAbs(got.x, x, 1e-9), std::string(label) + " x");
  check(nearAbs(got.y, y, 1e-9), std::string(label) + " y");
  check(nearAbs(got.z, z, 1e-9), std::string(label) + " z");
}

}  // namespace

int main() {
  // Phase-2 live hatch words (MM_CLIMB_002 row): column 32, file Y word 42.
  const auto climb = webo::terrainSampleFromHatchSpawn({32, 42});
  checkSample(climb, 32.0, 42.0, 0.5, "hatch spawn MM_CLIMB_002");

  // Sign extension path uses FILD on signed int32 (`0x00425C0B` / `0x00425C21`).
  const auto negativeX = webo::terrainSampleFromHatchSpawn({-1, 0});
  checkSample(negativeX, -1.0, 0.0, 0.5, "hatch spawn negative X");

  // Isolated-term gate: X and Y must come from distinct hatch words (not swapped or dropped).
  check(climb.x != climb.y, "hatch spawn X/Y independence");
  check(!nearAbs(climb.z, 0.75, 1e-9), "hatch spawn Z is not motion slab 0.75");

  if (g_failures != 0) {
    std::cerr << g_failures << " failure(s) in " << g_checks << " checks\n";
    return EXIT_FAILURE;
  }
  std::cout << "test_actorterrain: " << g_checks << " checks OK\n";
  return EXIT_SUCCESS;
}
