// fluid-surface-impl-001: visible linked fluid on the installed MM_W_012, source-built clone only.
//
// Production components exercised: BackdropScene::build (kind-4 admission, receiver base `Fluid`
// roots), BackdropScene::renderStage(Stage::FluidGate) fed from the live sim::World gate state,
// scheduleSceneFrame (tube -> fluid -> secondary), the accepted shared-dt World::advanceElapsedMs
// path and, for the ordinary route, app::runApplication(--level MM_W_012 --sim).
//
// Label key (observations file):
//   REAL       installed MM_W_012 data through the public loaders, no injected actors
//   SYNTHETIC  a walker placed beside the real lever (183,60) so contact happens within the run
//              (a fixture, not natural play; the natural input route belongs to the final assessment)
//   UNIT       a hand-built SceneNode exercising the animation update with literal expectations that
//              come from the retained measurement dump (gate-meshes-MM_W_012.txt), not from the code
//   PUBLIC     runApplication frames observed through the read-only per-frame view
//
// Nothing here is original-fidelity proof: depth state is the named PROVISIONAL clone policy
// (BackdropScene::kProvisionalFluidDepthPolicy); the original is never run.
#include "app/AppEntry.h"
#include "app/BackdropScene.h"
#include "app/SceneFrame.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/Bitmap.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "lemmings/compiled_level.hpp"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace webo;
using Policy = app::BackdropScene::FluidDepthPolicy;

int g_failures = 0;
int g_checks = 0;
std::ostringstream g_log;

void check(bool cond, const std::string& what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
    g_log << "FAIL " << what << "\n";
  }
}
void note(const std::string& line) { g_log << line << "\n"; }

Image toImage(const app::Raster& raster) {
  const auto& f = raster.framebuffer();
  Image im;
  im.width = f.width;
  im.height = f.height;
  for (std::size_t i = 0; i + 2 < f.rgb.size(); i += 3)
    im.rgba.insert(im.rgba.end(), {f.rgb[i], f.rgb[i + 1], f.rgb[i + 2], 255});
  return im;
}

std::vector<std::size_t> diffPixels(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b) {
  std::vector<std::size_t> out;
  if (a.size() != b.size()) return out;
  for (std::size_t i = 0; i + 2 < a.size(); i += 3)
    if (a[i] != b[i] || a[i + 1] != b[i + 1] || a[i + 2] != b[i + 2]) out.push_back(i / 3);
  return out;
}

// Independent projection (row-vector convention of the accepted camera basis).
bool project(const app::CameraBasis& c, double x, double y, double z, double& sx, double& sy) {
  const auto& m = c.worldToView;
  const double vx = x * m[0] + y * m[3] + z * m[6] + m[9];
  const double vy = x * m[1] + y * m[4] + z * m[7] + m[10];
  const double vz = x * m[2] + y * m[5] + z * m[8] + m[11];
  if (!(vz > 0.0)) return false;
  sx = c.cx + c.focalX * vx / vz;
  sy = c.cy - c.focalY * vy / vz;
  return true;
}

// SYNTHETIC walker beside lever (183,60): copied placement logic of test_levergates.
std::size_t placeWalker(sim::World& w, sim::Facing facing, bool inBox) {
  const sim::Lever lev = w.setup().levers.at(2);
  sim::Lemming l;
  l.x = inBox ? lev.x + 1 : lev.x + lev.width + 1;
  l.facing = facing;
  l.state = sim::State::Walker;
  l.y = lev.y + lev.height;
  for (int y = lev.y; y <= lev.y + lev.height; ++y) {
    if (w.terrain().solid(l.x, y + 1)) {
      l.y = y;
      break;
    }
  }
  w.place(l);
  return w.lemmings().size() - 1;
}

const sim::FluidGate* gateById(const sim::World& w, int id) {
  for (const auto& g : w.setup().gates)
    if (g.id == id) return &g;
  return nullptr;
}
std::size_t gateIndexById(const sim::World& w, int id) {
  for (std::size_t i = 0; i < w.setup().gates.size(); ++i)
    if (w.setup().gates[i].id == id) return i;
  return SIZE_MAX;
}

bool pushQuit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
  return SDL_PushEvent(&e) == 1;
}

struct PublicFrame {
  std::vector<std::uint8_t> rgba;
  int width = 0, height = 0;
  app::CameraParams pose;
  std::vector<app::BackdropScene::FluidGateState> gates;
  std::vector<int> ids;
};

std::vector<PublicFrame> runPublic(const std::string& gameDir, const std::string& cameraFile, bool sceneOff, int lastFrame, int& status) {
  std::vector<std::string> args{"fluidsurface_impl", "--game-dir", gameDir, "--level", "MM_W_012", "--sim",
                                "--native", "--camera", cameraFile, "--frames", "40", "--no-audio"};
  if (sceneOff) args.push_back("--scene-off");
  std::vector<char*> argv;
  for (auto& a : args) argv.push_back(a.data());
  std::vector<PublicFrame> frames;
  bool queued = false;
  status = app::runApplication(
      static_cast<int>(argv.size()), argv.data(), {},
      [&](const app::LevelFrameView& f) {
        PublicFrame p;
        p.rgba = f.image.rgba;
        p.width = f.image.width;
        p.height = f.image.height;
        p.pose = f.displayedPose;
        for (const auto& g : f.world.setup().gates) {
          p.gates.push_back({g.lane, g.height, g.wavePhase, g.animated});
          p.ids.push_back(g.id);
        }
        frames.push_back(std::move(p));
        if (f.frame >= lastFrame && !queued) queued = pushQuit();
      },
      app::ApplicationRunOptions{.frameSeconds = 0.25});
  return frames;
}

}  // namespace

int main(int argc, char** argv) {
  const std::filesystem::path outDir = argc > 1 ? argv[1] : "build/fluid-surface-impl-001";
  std::filesystem::create_directories(outDir);

  // ---- UNIT: animation update, literal expectations from the retained dump (gate 12, lane 20).
  {
    SceneNode g;
    g.type = 1;
    g.positions.assign(6, {0, 0, 0});
    g.normals = {{0, 1, 0}, {0.778976, 0, -0.627054}, {0.778976, 0, -0.627054}};
    SceneMeshBlock sides, top;
    sides.kind = 4;
    top.kind = 5;
    const std::array<std::array<std::uint32_t, 3>, 2> topCorners{{{2, 4, 3}, {3, 4, 5}}};
    for (const auto& tri : topCorners) {
      SceneRecord rec;
      rec.normalIndex = 0;
      for (auto v : tri) {
        SceneCorner c;
        c.vertexIndex = v;
        c.a = 128;
        rec.corners.push_back(c);
      }
      top.records.push_back(rec);
    }
    g.meshes = {sides, top};
    app::BackdropScene::FluidRecord rec;
    rec.posOff = {{{41.5573, 4, -41.5573}, {48.7752, 4, -32.5906}, {41.7078, 4, -27.8682}, {35.5469, 4, -35.5469}}};
    for (auto& a : rec.amp) a = {0, 1, 0};
    rec.baseColour = {0, 0, 192};
    app::BackdropScene::FluidGateState st;
    st.lane = 20;
    st.height = 24.5;
    st.wavePhase = 0.0;
    st.animated = true;
    SceneNode settled = g;
    app::BackdropScene::applyFluidState(settled, rec, st);
    check(std::abs(settled.positions[2][1] - 29.0) < 1e-4 && std::abs(settled.positions[3][1] - 28.5) < 1e-4 &&
              std::abs(settled.positions[4][1] - 29.0) < 1e-4 && std::abs(settled.positions[5][1] - 28.5) < 1e-4,
          "UNIT: h=24.5 phase 0 lane 20 gives outer-edge Y (29, 28.5) as in the installed gate-12 dump");
    check(std::abs(settled.positions[2][0] - 41.5573) < 1e-9 && std::abs(settled.positions[5][2] + 27.8682) < 1e-9,
          "UNIT: X/Z of the animated vertices stay at the serialized pos_off");
    const auto& r0 = settled.meshes[1].records[0].corners;
    const auto& r1 = settled.meshes[1].records[1].corners;
    // The colour phase uses the traced promoted-f32 pi, so cos(2*pi*(0-0.25)) is -4.4e-8 rather than 0
    // and the 0.25 lane weight lands ~5.6e-6 below 63.75. The tolerance is 1e-3 colour units, far
    // below one 8-bit output step; the literals themselves are the exact-pi arithmetic values.
    constexpr double kColourTol = 1e-3;
    check(std::abs(r0[0].r - 63.75) < kColourTol && std::abs(r0[0].b - 207.75) < kColourTol && std::abs(r0[1].r - 63.75) < kColourTol &&
              std::abs(r0[2].r - 255.0) < kColourTol && std::abs(r1[0].b - 255.0) < kColourTol && std::abs(r1[1].r - 63.75) < kColourTol &&
              std::abs(r1[2].g - 255.0) < kColourTol && r0[0].a == 128.0,
          "UNIT: lane-1/lane-2 top colours (63.75,63.75,207.75)/(255,255,255), alpha untouched");
    check(settled.normals[0][1] == 1.0, "UNIT: stored normals are kept while h != 0");
    st.height = 0.0;
    SceneNode drained = g;
    app::BackdropScene::applyFluidState(drained, rec, st);
    bool flat = true;
    for (std::size_t s = 2; s < 6; ++s) flat = flat && drained.positions[s][1] == 4.0;
    check(flat, "UNIT: h=0 collapses vertices 2..5 onto pos_off (Y=4), wave forced to 0");
    check(drained.normals[0][1] == -1.0 && drained.normals[1][0] == -0.778976 && drained.normals[2][2] == 0.627054,
          "UNIT: h=0 negates normals 0..2");
    check(drained.meshes[1].records[0].corners[0].r == 0.0 && drained.meshes[1].records[0].corners[0].b == 192.0,
          "UNIT: h=0 top colour (0,0,192)");
    st.animated = false;
    st.height = 24.5;
    SceneNode raw = g;
    app::BackdropScene::applyFluidState(raw, rec, st);
    check(raw.positions[2][1] == 0.0, "UNIT: before the first update the stored vertices are untouched");
    // Wave phase: a half period flips the lane-1 sample from +0.5 to -0.5.
    st.animated = true;
    st.wavePhase = 1.0;
    SceneNode half = g;
    app::BackdropScene::applyFluidState(half, rec, st);
    check(std::abs(half.positions[2][1] - 28.0) < 1e-4, "UNIT: phase 1.0 (half the 2.0 period) gives w = -0.5");
  }

  const char* dirC = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dirC || !*dirC || !std::filesystem::exists(std::filesystem::path(dirC) / "lemmings.box")) {
    std::cout << "fluid surface: UNIT checks ran, installed checks skipped (WEBOLUTION_GAME_DIR has no lemmings.box)\n";
    return g_failures == 0 ? 0 : 1;
  }
  const std::filesystem::path root(dirC);
  auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  auto resolved = sim::resolveRetailLevel(*lib, "MM_W_012");
  if (!resolved) {
    std::cerr << resolved.error() << "\n";
    return 1;
  }
  auto blob = lib->read(resolved->file);
  if (!blob) {
    std::cerr << blob.error() << "\n";
    return 1;
  }
  auto mesh = findCylinderMesh(*blob);
  auto index = indexLevel(*blob);
  if (!mesh || !index) {
    std::cerr << "tube input unavailable\n";
    return 1;
  }
  auto tube = app::TubeScene::build(*lib, *mesh, *blob, index->grid0, true);
  auto built = app::BackdropScene::build(*lib, *blob, "MM_W_012");
  if (!tube || !built) {
    std::cerr << "scene build failed\n";
    return 1;
  }
  app::BackdropScene& backdrop = *built;
  // The production default, read before any diagnostic selects another policy.
  check(app::BackdropScene::kProvisionalFluidDepthPolicy == Policy::LessWriteOn && backdrop.fluidDepthPolicy() == Policy::LessWriteOn,
        "the default policy is the named provisional LessWriteOn");

  // ---- REAL: admission of the installed receivers.
  {
    const auto indices = backdrop.fluidEntityIndices();
    check(indices.size() == 19, std::format("REAL: all 19 kind-0 receivers admitted as animated Fluid roots, got {}", indices.size()));
    bool ordered = std::is_sorted(indices.begin(), indices.end());
    check(ordered, "REAL: receivers are submitted in source (stream) order");
    for (const auto& d : backdrop.diagnostics())
      check(d.find("FluidGate") == std::string::npos, "REAL: no fluid admission diagnostic: " + d);
    note(std::format("REAL admission: {} fluid roots; stream indices of lever ids 5/6 are 12/11", indices.size()));
  }

  const auto loadWorld = [&]() {
    auto setup = sim::loadRetailLevel(*lib, "MM_W_012");
    if (!setup) {
      check(false, "load MM_W_012: " + setup.error());
      return sim::World{};
    }
    return sim::World(std::move(*setup));
  };

  struct Shot {
    app::Raster raster{640, 480};
    app::BackdropStats stats;
  };
  const auto shoot = [&](const sim::World* w, const app::CameraBasis& cam, Policy policy) {
    Shot s;
    s.raster.clear(0, 0, 0);
    if (w) {
      std::vector<app::BackdropScene::FluidGateState> st;
      for (const auto& g : w->setup().gates) st.push_back({g.lane, g.height, g.wavePhase, g.animated});
      backdrop.setFluidState(std::move(st));
    } else {
      backdrop.clearFluidState();
    }
    backdrop.setFluidDepthPolicy(policy);
    std::vector<std::uint8_t> mask(640 * 480, 1);
    app::SceneRasterObserver obs;
    obs.mask = &mask;
    app::SceneOptions options;
    options.drawCellRoute = true;
    options.drawWall = false;
    options.drawOuter = false;
    options.drawInner = false;
    using S = app::BackdropScene::Stage;
    app::scheduleSceneFrame(
        [&] { backdrop.renderStage(s.raster, cam, *lib, s.stats, S::Primary); },
        [&] { backdrop.renderStage(s.raster, cam, *lib, s.stats, S::LegacyUnattributed); },
        [&] { tube->render(s.raster, cam, options, &*lib); },
        [&] { backdrop.renderStage(s.raster, cam, *lib, s.stats, S::FluidGate, &obs); },
        [&] { backdrop.renderStage(s.raster, cam, *lib, s.stats, S::Secondary); });
    backdrop.clearFluidState();
    return s;
  };
  const auto submission = [](const Shot& s, std::size_t entity) -> const app::BackdropStats::FluidSubmission* {
    for (const auto& f : s.stats.fluidSubmissions)
      if (f.entityIndex == entity) return &f;
    return nullptr;
  };
  const auto step = [](sim::World& w, int ms) { w.advanceElapsedMs(ms); };

  constexpr int kIdA = 5, kIdB = 6;  // lever (183,60) receivers
  // Settled reference world only to choose the stable camera.
  sim::World reference = loadWorld();
  const std::size_t ia = gateIndexById(reference, kIdA), ib = gateIndexById(reference, kIdB);
  check(ia == 12 && ib == 11, "REAL: ids 5/6 are stream indices 12/11");
  placeWalker(reference, sim::Facing::Left, false);
  for (int i = 0; i < 13; ++i) step(reference, 16);
  for (int i = 0; i < 200; ++i) step(reference, 100);
  check(gateById(reference, kIdA)->height == 24.5 && gateById(reference, kIdB)->height == 24.5, "SYNTHETIC: reference world settled at 24.5");

  // Stable camera: the yaw (0.. 252 step 4, the production params otherwise) from which the settled
  // pair of receivers writes the most pixels. Selection is by actual raster stores, not by geometry.
  const int terrainHeight = reference.terrain().height();
  app::CameraParams chosen;
  std::size_t bestScore = 0;
  for (int yaw = 0; yaw < 256; yaw += 4) {
    app::CameraParams p;
    p.yawUnits = yaw;
    p.levelEyeHeight = terrainHeight;
    const auto s = shoot(&reference, app::buildCamera(p), app::BackdropScene::kProvisionalFluidDepthPolicy);
    const auto* a = submission(s, ia);
    const auto* b = submission(s, ib);
    const std::size_t score = (a ? a->acceptedColorWrites : 0) + (b ? b->acceptedColorWrites : 0);
    if (score > bestScore) {
      bestScore = score;
      chosen = p;
    }
  }
  note(std::format("camera: yawUnits={} levelEyeHeight={} settled-pair accepted stores={}", chosen.yawUnits, chosen.levelEyeHeight, bestScore));
  check(bestScore > 0, "REAL: some camera yaw sees the settled receiver pair of lever (183,60) (fluid pixels written)");
  const app::CameraBasis cam = app::buildCamera(chosen);

  // ---- Baseline: the production scene with NO fluid state (what the clone drew before this task).
  sim::World initialWorld = loadWorld();
  placeWalker(initialWorld, sim::Facing::Left, false);
  for (int i = 0; i < 12; ++i) step(initialWorld, 16);  // 12 updates: before the walker touches the lever
  check(initialWorld.setup().levers[2].state == 1, "SYNTHETIC: lever untouched in the initial stage");
  const Shot baseline = shoot(nullptr, cam, Policy::LessWriteOn);
  const Shot initial = shoot(&initialWorld, cam, Policy::LessWriteOn);
  check(baseline.stats.fluidSubmissions.empty(), "baseline: no fluid submissions without live state");
  check(writePng(outDir / "baseline-no-fluid.png", toImage(baseline.raster)).has_value(), "write baseline PNG");
  check(writePng(outDir / "initial.png", toImage(initial.raster)).has_value(), "write initial PNG");
  {
    const auto* a = submission(initial, ia);
    const auto* b = submission(initial, ib);
    check(a && b && a->height == 0.0 && b->height == 0.0, "initial: ids 5/6 at height 0");
    check(a && b && a->triangles == 0 && b->triangles == 0 && a->acceptedColorWrites == 0 && b->acceptedColorWrites == 0,
          "initial: drained receivers draw nothing (h=0 normals negated; provenance: research 7.2)");
    check(a && !a->raw && initialWorld.setup().gates[ia].animated, "initial: first update has run (animated geometry, not the raw mesh)");
    // Raised receivers (stream indices 8..10 = ids 12/11/10 start at 22.5 per gate-records-MM_W_012.txt)
    // show water in the initial frame.
    std::size_t raisedWrites = 0;
    for (const auto& g : initialWorld.setup().gates)
      if (g.height > 0.0) {
        const auto* sub = submission(initial, static_cast<std::size_t>(&g - initialWorld.setup().gates.data()));
        raisedWrites += sub ? sub->acceptedColorWrites : 0;
      }
    note(std::format("initial: fluid pixels changed vs baseline stage={} accepted stores of loaded-raised receivers={}", initial.stats.fluidPixelsChanged, raisedWrites));
    if (raisedWrites > 0)
      check(!diffPixels(baseline.raster.framebuffer().rgb, initial.raster.framebuffer().rgb).empty(),
            "initial: loaded-raised receivers change pixels relative to the no-fluid baseline");
  }

  // ---- Matched-time controls: control (no pull) versus activated, identical dt history.
  sim::World control = loadWorld();
  sim::World active = loadWorld();
  placeWalker(active, sim::Facing::Left, false);
  for (int i = 0; i < 13; ++i) {
    step(control, 16);
    step(active, 16);
  }
  check(active.setup().levers[2].state == -1 && gateById(active, kIdA)->request == 0, "SYNTHETIC: pull sent type 4 to ids 5/6");
  check(gateById(control, kIdA)->request == 1, "control: no pull, ids 5/6 stay drained");

  struct Sample {
    const char* name;
    int ms;
  };
  const std::array<Sample, 3> samples{{{"rising-a", 3000}, {"rising-b", 4000}, {"settled", 10000}}};
  std::vector<std::size_t> changedCounts;
  for (const auto& sample : samples) {
    step(control, sample.ms);
    step(active, sample.ms);
    const Shot c = shoot(&control, cam, Policy::LessWriteOn);
    const Shot a = shoot(&active, cam, Policy::LessWriteOn);
    check(writePng(outDir / (std::string("control-") + sample.name + ".png"), toImage(c.raster)).has_value(), "write control PNG");
    check(writePng(outDir / (std::string("active-") + sample.name + ".png"), toImage(a.raster)).has_value(), "write active PNG");
    const double h = gateById(active, kIdA)->height;
    const auto changed = diffPixels(c.raster.framebuffer().rgb, a.raster.framebuffer().rgb);
    changedCounts.push_back(changed.size());
    note(std::format("{}: active h(id5)={:.4f} h(id6)={:.4f} control h={:.1f} phase={:.4f}/{:.4f} changed pixels={}", sample.name, h,
                     gateById(active, kIdB)->height, gateById(control, kIdA)->height, gateById(active, kIdA)->wavePhase,
                     gateById(control, kIdA)->wavePhase, changed.size()));
    check(gateById(control, kIdA)->height == 0.0 && gateById(control, kIdB)->height == 0.0, std::string(sample.name) + ": control receivers drained");
    check(h > 0.0, std::string(sample.name) + ": activated receivers are rising/settled");
    check(gateById(active, kIdA)->wavePhase == gateById(control, kIdA)->wavePhase, std::string(sample.name) + ": identical wave phase (matched time)");
    // Control: nothing of the drained pair is drawn; activated: the surface rasterizes.
    const auto* ca = submission(c, ia);
    const auto* aa = submission(a, ia);
    const auto* ab = submission(a, ib);
    check(ca && ca->triangles == 0, std::string(sample.name) + ": control draws no surface for id 5");
    check(aa && ab && aa->triangles > 0 && ab->triangles > 0 && aa->height == h, std::string(sample.name) + ": activated ids 5/6 submit surface triangles at the live height");
    check(aa && aa->acceptedColorWrites > 0 && ab->acceptedColorWrites > 0, std::string(sample.name) + ": activated ids 5/6 write pixels");
    // Receivers whose live state is identical in both worlds must submit identically. The injected
    // walker may touch another lever during the run, so such receivers (if any) are listed and
    // excluded from the equality and added to the footprint instead of being assumed unchanged.
    std::set<std::size_t> differing{ia, ib};
    for (std::size_t i = 0; i < active.setup().gates.size(); ++i) {
      const auto& ga = active.setup().gates[i];
      const auto& gc = control.setup().gates[i];
      if (ga.height != gc.height || ga.request != gc.request || ga.done != gc.done) differing.insert(i);
    }
    bool others = true;
    for (std::size_t i = 0; i < active.setup().gates.size(); ++i) {
      if (differing.count(i)) continue;
      const auto* sc = submission(c, i);
      const auto* sa = submission(a, i);
      others = others && sc && sa && sc->triangles == sa->triangles && sc->predicateRejected == sa->predicateRejected && sc->height == sa->height;
    }
    check(others, std::string(sample.name) + ": receivers with identical live state submit identically in control and activated worlds");
    note(std::format("{}: receivers whose state differs between worlds: {}", sample.name, differing.size()));
    // Pixel correlation: differences lie inside the projected footprint of receivers 5/6.
    double x0 = 1e9, y0 = 1e9, x1 = -1e9, y1 = -1e9;
    bool projectable = true;
    const lemmings::CompiledLevel level = lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size()});
    for (std::size_t gi : differing) {
      const auto& rec = level.gadgets.entities_42acac[gi];
      const double gh = active.setup().gates[gi].height;
      for (std::size_t k = 0; k < 4; ++k) {
        const double px = std::bit_cast<double>(rec.triples[2 * k][0]);
        const double pz = std::bit_cast<double>(rec.triples[2 * k][2]);
        for (double y : {4.0, 4.0 + gh - 0.5, 4.0 + gh + 0.5}) {
          double sx = 0, sy = 0;
          if (!project(cam, px, y, pz, sx, sy)) {
            projectable = false;
            continue;
          }
          x0 = std::min(x0, sx);
          y0 = std::min(y0, sy);
          x1 = std::max(x1, sx);
          y1 = std::max(y1, sy);
        }
      }
    }
    if (projectable) {
      std::size_t outside = 0;
      for (std::size_t p : changed) {
        const double px = static_cast<double>(p % 640), py = static_cast<double>(p / 640);
        if (px < x0 - 3 || px > x1 + 3 || py < y0 - 3 || py > y1 + 3) ++outside;
      }
      check(outside == 0, std::format("{}: all {} differing pixels lie inside the independently projected receiver 5/6 footprint ({} outside)", sample.name, changed.size(), outside));
    } else {
      note(std::string(sample.name) + ": footprint not projectable from this camera (check skipped)");
    }
  }
  check(changedCounts[0] > 0 && changedCounts[1] > 0 && changedCounts[2] > 0, "pixels differ between control and activated at every sample");
  check(changedCounts[0] != changedCounts[1] || changedCounts[1] != changedCounts[2], "differing-pixel counts change as the surface rises");

  // ---- Provisional depth policy diagnostic at the matched settled time.
  {
    const Shot less = shoot(&active, cam, Policy::LessWriteOn);
    const Shot lessNoWrite = shoot(&active, cam, Policy::LessWriteOff);
    const Shot disabled = shoot(&active, cam, Policy::DepthDisabled);
    note(std::format("depth diagnostic (settled, matched time): fluid pixels changed LessWriteOn={} LessWriteOff={} DepthDisabled={}; "
                     "LessWriteOn vs LessWriteOff differing pixels={}, vs DepthDisabled={}",
                     less.stats.fluidPixelsChanged, lessNoWrite.stats.fluidPixelsChanged, disabled.stats.fluidPixelsChanged,
                     diffPixels(less.raster.framebuffer().rgb, lessNoWrite.raster.framebuffer().rgb).size(),
                     diffPixels(less.raster.framebuffer().rgb, disabled.raster.framebuffer().rgb).size()));
    check(disabled.stats.fluidPixelsChanged >= less.stats.fluidPixelsChanged, "depth diagnostic: disabling the test never hides fluid pixels that Less shows");
    check(writePng(outDir / "settled-depth-less-write-on.png", toImage(less.raster)).has_value() &&
              writePng(outDir / "settled-depth-less-write-off.png", toImage(lessNoWrite.raster)).has_value() &&
              writePng(outDir / "settled-depth-disabled.png", toImage(disabled.raster)).has_value(),
          "write depth diagnostic PNGs");
    backdrop.setFluidDepthPolicy(app::BackdropScene::kProvisionalFluidDepthPolicy);  // leave the diagnostic selection
  }

  // ---- Repeat pulls after settle, pause and reload.
  {
    placeWalker(active, sim::Facing::Right, true);
    step(active, 16);
    placeWalker(active, sim::Facing::Left, true);
    step(active, 16);
    check(gateById(active, kIdA)->height == 24.5 && gateById(active, kIdB)->height == 24.5, "repeat pulls after settle keep 24.5");
    const Shot p1 = shoot(&active, cam, Policy::LessWriteOn);
    const Shot p2 = shoot(&active, cam, Policy::LessWriteOn);
    check(diffPixels(p1.raster.framebuffer().rgb, p2.raster.framebuffer().rgb).empty(), "pause: a frame with no world update is repeatable byte for byte");

    // Reload: a fresh load with the same post-reset dt history reproduces the initial frame exactly.
    sim::World reloaded = loadWorld();
    placeWalker(reloaded, sim::Facing::Left, false);
    for (int i = 0; i < 12; ++i) step(reloaded, 16);
    const Shot again = shoot(&reloaded, cam, Policy::LessWriteOn);
    check(diffPixels(initial.raster.framebuffer().rgb, again.raster.framebuffer().rgb).empty(), "reload: byte-identical to the initial frame at identical inputs");
    check(gateById(reloaded, kIdA)->height == 0.0 && gateById(reloaded, kIdA)->request == 1 && gateById(reloaded, 10)->height == 22.5 &&
              gateById(reloaded, 11)->height == 22.5 && gateById(reloaded, 12)->height == 22.5,
          "reload: ids 5/6 drained, loaded-raised receivers (ids 10/11/12) at 22.5");
  }

  // ---- Fast-forward synchronisation: wave phase depends on total elapsed time, not on segmentation.
  {
    sim::World fine = loadWorld();
    sim::World coarse = loadWorld();
    for (int i = 0; i < 312; ++i) step(fine, 16);
    step(fine, 8);
    for (int i = 0; i < 50; ++i) step(coarse, 100);
    bool same = fine.setup().gates.size() == coarse.setup().gates.size();
    for (std::size_t i = 0; same && i < fine.setup().gates.size(); ++i)
      same = std::abs(fine.setup().gates[i].wavePhase - coarse.setup().gates[i].wavePhase) < 1e-9 &&
             fine.setup().gates[i].height == coarse.setup().gates[i].height;
    note(std::format("fast-forward: phase fine={:.9f} coarse={:.9f}", fine.setup().gates[0].wavePhase, coarse.setup().gates[0].wavePhase));
    check(same, "fast-forward: 5.000 s as 16 ms steps and as 100 ms steps give the same phase (mod 2.0) and heights");
    check(std::abs(coarse.setup().gates[0].wavePhase - 1.0) < 1e-9, "5.0 s of dt wraps to phase 1.0 at the 2.0 period");
  }

  // ---- Unrelated scene regression: levels without a fluid stage render identically with or without state.
  {
    auto other = sim::resolveRetailLevel(*lib, "MM_CLIMB_002");
    if (other) {
      auto otherBlob = lib->read(other->file);
      auto built2 = app::BackdropScene::build(*lib, *otherBlob, "MM_CLIMB_002");
      check(otherBlob.has_value() && built2.has_value(), "MM_CLIMB_002 builds");
      if (built2) {
        const auto idx = built2->fluidEntityIndices();
        app::Raster r1(640, 480), r2(640, 480);
        r1.clear(0, 0, 0);
        r2.clear(0, 0, 0);
        app::BackdropStats s1, s2;
        const app::CameraBasis c2 = app::buildCamera(app::CameraParams{});
        built2->render(r1, c2, *lib, s1);
        built2->setFluidState(std::vector<app::BackdropScene::FluidGateState>(8));
        built2->render(r2, c2, *lib, s2);
        note(std::format("regression MM_CLIMB_002: fluid roots={} meshTriangles={}", idx.size(), s1.meshTriangles));
        if (idx.empty()) check(diffPixels(r1.framebuffer().rgb, r2.framebuffer().rgb).empty(), "MM_CLIMB_002 has no receivers: output unchanged by fluid state");
        check(s1.meshTriangles > 0, "MM_CLIMB_002 scenery still draws");
      }
    }
  }

  // ---- PUBLIC: ordinary application route, with and without scenery.
  {
    const char* video = std::getenv("SDL_VIDEODRIVER");
    const char* audio = std::getenv("SDL_AUDIODRIVER");
    if (!video || std::string(video) != "dummy" || !audio || std::string(audio) != "dummy") {
      note("PUBLIC: skipped, SDL dummy drivers are not set");
    } else {
      // The standalone default camera (yaw 32, eye height 96) barely sees the raised receivers: the
      // first supervised run measured 1930 fluid-stage pixels in 8 frames, none visible in the final
      // image. Both public runs therefore use the stable camera of the stages above, handed to the
      // ordinary route through its documented --camera parameter file.
      const std::filesystem::path cameraFile = outDir / "public-camera.json";
      {
        std::ofstream cf(cameraFile);
        cf << std::format("{{\"viewport\": [{}, {}], \"yaw_units\": {}, \"level_eye_height\": {}}}\n", chosen.width, chosen.height,
                          chosen.yawUnits, chosen.levelEyeHeight);
      }
      note(std::format("PUBLIC camera file: yaw_units={} level_eye_height={}", chosen.yawUnits, chosen.levelEyeHeight));
      int statusOn = -1, statusOff = -1;
      const auto on = runPublic(dirC, cameraFile.string(), false, 7, statusOn);
      const auto off = runPublic(dirC, cameraFile.string(), true, 7, statusOff);
      check(statusOn == 0 && statusOff == 0, std::format("PUBLIC: both runs exit 0 ({} / {})", statusOn, statusOff));
      check(on.size() == off.size() && on.size() >= 5, std::format("PUBLIC: frames observed {} / {}", on.size(), off.size()));
      std::size_t maskTotal = 0, changedTotal = 0;
      for (std::size_t f = 0; f < std::min(on.size(), off.size()); ++f) {
        const auto& fr = on[f];
        // Live state seen by the ordinary route: ids 5/6 drained, ids 10/11/12 loaded raised, first update done.
        if (f >= 1) {
          for (std::size_t g = 0; g < fr.ids.size(); ++g) {
            if (fr.ids[g] == kIdA || fr.ids[g] == kIdB) check(fr.gates[g].height == 0.0 && fr.gates[g].animated, "PUBLIC: ids 5/6 drained and animated in the ordinary route");
            if (fr.ids[g] >= 10 && fr.ids[g] <= 12) check(fr.gates[g].height == 22.5, std::format("PUBLIC: gate id {} loaded raised (22.5)", fr.ids[g]));
          }
        }
        if (fr.width != 640 || fr.height != 480 || off[f].rgba.size() != fr.rgba.size()) continue;
        // Fluid-only render on black at the frame's own pose and live gate snapshot = the mask.
        backdrop.setFluidState(fr.gates);
        app::Raster only(640, 480);
        only.clear(0, 0, 0);
        app::BackdropStats st;
        backdrop.renderStage(only, app::buildCamera(fr.pose), *lib, st, app::BackdropScene::Stage::FluidGate);
        backdrop.clearFluidState();
        const auto& rgb = only.framebuffer().rgb;
        for (std::size_t p = 0; p < 640u * 480u; ++p) {
          if (!(rgb[3 * p] || rgb[3 * p + 1] || rgb[3 * p + 2])) continue;
          ++maskTotal;
          if (fr.rgba[4 * p] != off[f].rgba[4 * p] || fr.rgba[4 * p + 1] != off[f].rgba[4 * p + 1] || fr.rgba[4 * p + 2] != off[f].rgba[4 * p + 2]) ++changedTotal;
        }
      }
      note(std::format("PUBLIC: fluid-stage mask pixels over frames={} differing from the --scene-off run={}", maskTotal, changedTotal));
      check(maskTotal > 0, "PUBLIC: the fluid stage writes pixels in the ordinary route frames");
      check(maskTotal == 0 || changedTotal * 2 >= maskTotal, "PUBLIC: at least half of the fluid-stage pixels differ from the scene-off run");
    }
  }

  std::ofstream log(outDir / "observations.txt");
  log << "fluid-surface-impl-001 observations (source-built clone; not original-fidelity proof)\n"
      << "depth policy: PROVISIONAL " << "LessWriteOn" << " (original ZENABLE/ZWRITEENABLE/ZFUNC/ZBIAS unresolved)\n"
      << g_log.str() << "checks=" << g_checks << " failures=" << g_failures << "\n";
  std::cout << (g_failures == 0 ? "PASS" : "FAIL") << ": fluid surface impl, " << g_checks << " checks, " << g_failures << " failures\n";
  return g_failures == 0 ? 0 : 1;
}
