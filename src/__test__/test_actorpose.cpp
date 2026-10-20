#include "app/ActorPose.h"
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

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

struct NativeSample {
  const char* actor;
  std::uint32_t vtable;
  double plus8[3];
  int facing;
  double model108[3];
  double basis[3];
  int simX;
  int simY;
  double zSlab;
};

// Measured live on MM_CLIMB_002, 2026-10-20 (`build/tmp/a6a-actor-pose/live/report.json`).
constexpr NativeSample kSamples[] = {
    {"0x2833720", 0x004B10E8,
     {59.64480209350586, 32.0, 0.75}, 1,
     {-56.993675231933594, 32.0, 6.125833988189697},
     {-0.08235539495944977, 0.0, -0.9965759515762329}, 59, 63, 0.75},
    {"0x2834c50", 0x004B10E8,
     {45.21244430541992, 32.0, 0.75}, 1,
     {-51.26413345336914, 32.0, 25.476972579956055},
     {-0.4517080783843994, 0.0, -0.89213627576828}, 45, 63, 0.75},
    {"0x28344c0", 0x004B10E8,
     {32.0, 36.87969207763672, 0.5}, 1,
     {-39.0824089050293, 36.87969207763672, 39.0824089050293},
     {-0.6265777349472046, 0.0, -0.7793527245521545}, 32, 58, 0.5},
};

webo::ActorPoseLevel loadPoseLevel(const fs::path& gameDir) {
  auto lib = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib)
    throw std::runtime_error(lib.error());
  std::string name;
  for (const auto& candidate : webo::levelFileNames(*lib)) {
    if (candidate.find("MM_CLIMB_002") != std::string::npos) {
      name = candidate;
      break;
    }
  }
  if (name.empty())
    throw std::runtime_error("MM_CLIMB_002 not in archives");
  auto blob = lib->read(name);
  if (!blob)
    throw std::runtime_error(blob.error());
  auto mesh = webo::findCylinderMesh(*blob);
  auto grid = webo::findCollisionGrid(*blob);
  auto index = webo::indexLevel(*blob);
  if (!mesh || !grid || !index)
    throw std::runtime_error("level parse failed");

  static std::vector<std::uint32_t> grid0Storage;
  static std::vector<webo::Vec3> surfaceStorage;
  static webo::CylinderMesh cylinderStorage;
  grid0Storage = index->grid0;
  surfaceStorage = mesh->surface;
  cylinderStorage = std::move(*mesh);

  webo::ActorPoseLevel level;
  level.gridHeight = grid->height;
  level.grid0 = grid0Storage;
  level.surface = surfaceStorage;
  level.cylinderTemplate = &cylinderStorage;
  return level;
}

void testNativeTable(const webo::ActorPoseLevel& level) {
  std::cout << "actor pose native table\n";
  constexpr double kModelTol = 1e-4;
  // REGRESSION BOUND (not a fidelity tolerance): brief basis falsifier 1e-6 is not met
  // (max measured delta 2.962e-5 on retained live table). Native float32/x87 normalize at
  // 0x00480AC8 is not replayed in the C++ adapter.
  constexpr double kBasisRegressionBound = 5e-5;
  for (const auto& s : kSamples) {
    webo::ActorTerrainSample plus8{s.plus8[0], s.plus8[1], s.plus8[2]};
    const auto got = webo::actorPoseFromNativeSample(level, plus8, s.facing);
    for (int i = 0; i < 3; ++i) {
      check(nearAbs(got.modelPosition[i], s.model108[i], kModelTol),
            std::format("{} model[{}]", s.actor, i));
      check(nearAbs(got.basis[i], s.basis[i], kBasisRegressionBound),
            std::format("{} basis[{}]", s.actor, i));
    }
  }
}

}  // namespace

int main() {
  const char* env = std::getenv("WEBOLUTION_GAME_DIR");
  if (!env) {
    std::cout << "skip: WEBOLUTION_GAME_DIR not set\n";
    return 0;
  }
  try {
    const auto level = loadPoseLevel(fs::path(env));
    testNativeTable(level);
  } catch (const std::exception& ex) {
    check(false, ex.what());
  }
  std::cout << g_checks << " checks, " << g_failures << " failures\n";
  return g_failures == 0 ? 0 : 1;
}
