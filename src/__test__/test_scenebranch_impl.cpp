// scene-branch-impl-001: authentic frozen (supervisor task-entry SceneNode/BackdropScene) and
// current loaders in separate executables over EVERY effective installed level, plus the
// MM_DIG_000 type-2 branch-1 sprite demonstration (current side only).
// Fixed source camera, injected generator state; source-built rendering evidence only. No
// original code runs; no claim of original admission, flame-phase agreement, x87 equivalence,
// clipping or full-frame fidelity; C3 stays FAIL.
#include "app/BackdropScene.h"
#include "app/SceneFrame.h"
#include "app/TubeScene.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "scene_lever_baseline.hpp"
#ifndef SCENE_FROZEN
#include "app/AppEntry.h"
#include "app/CrtRand.h"
#include "assets/Sprite.h"
#include "lemmings/texture_creation.hpp"
#include "sceneadmission_census.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace webo;
using namespace webo::app;
using Bytes = std::vector<std::byte>;
using Clock = std::chrono::steady_clock;
namespace {
void require(bool pass, const std::string& message) {
  if (!pass) throw std::runtime_error(message);
}
double seconds(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double>(b - a).count(); }
void binary(const fs::path& path, const void* data, std::size_t size) {
  std::ofstream out(path, std::ios::binary);
  out.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
  require(bool(out), "write " + path.string());
}
Image image(const Raster& raster) {
  const auto& f = raster.framebuffer(); Image im; im.width = f.width; im.height = f.height;
  for (std::size_t i = 0; i < f.rgb.size(); i += 3) im.rgba.insert(im.rgba.end(), {f.rgb[i], f.rgb[i + 1], f.rgb[i + 2], 255});
  return im;
}
std::size_t changed(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b, std::size_t stride = 3) {
  require(a.size() == b.size(), "equal pixel storage");
  std::size_t n = 0;
  for (std::size_t i = 0; i < a.size(); i += stride)
    if (!std::equal(a.begin() + i, a.begin() + i + 3, b.begin() + i)) ++n;
  return n;
}
void artifact(const fs::path& dir, const std::string& name, Raster& raster, bool raw) {
  require(bool(writePng(dir / (name + ".png"), image(raster))), "PNG " + name);
  if (!raw) return;
  const auto& rgb = raster.framebuffer().rgb;
  binary(dir / (name + ".rgb"), rgb.data(), rgb.size());
  const auto& depth = raster.mutableDepthBuffer();
  binary(dir / (name + ".depth"), depth.data(), depth.size() * sizeof(double));
}
struct Hashes { std::uint64_t rgb, depth; };
Hashes hashes(Raster& raster) {
  Hashes h{1469598103934665603ull, 1469598103934665603ull};
  for (auto v : raster.framebuffer().rgb) h.rgb = (h.rgb ^ v) * 1099511628211ull;
  for (double d : raster.mutableDepthBuffer()) {
    auto bits = std::bit_cast<std::uint64_t>(d);
    for (int i = 0; i < 8; ++i) h.depth = (h.depth ^ ((bits >> (8 * i)) & 0xff)) * 1099511628211ull;
  }
  return h;
}
std::string hex(std::uint64_t v) { std::ostringstream s; s << std::hex << std::setw(16) << std::setfill('0') << v; return s.str(); }
constexpr std::array<BackdropScene::Stage, 3> stages = {BackdropScene::Stage::Primary, BackdropScene::Stage::LegacyUnattributed, BackdropScene::Stage::Secondary};
constexpr std::array<const char*, 3> stageNames = {"Primary", "LegacyUnattributed", "Secondary"};
struct Rendered {
  Raster raster{640, 480};
  leverbaseline::Pixels leverPixels;
  std::array<BackdropStats, 3> stats{};
  std::array<std::size_t, 3> writes{}, depthWrites{}, candidates{};
  std::set<std::size_t> spritePixels;  // pixels accepted from sprite submissions
};
Rendered render(const BackdropScene& backdrop, const TubeScene* tube, const AssetLibrary& lib, const CameraBasis& cam,
                bool scenery, bool compose, const std::set<std::size_t>* spriteOffsets = nullptr) {
  Rendered out; out.raster.clear(0, 0, 0); out.leverPixels.bind(backdrop);
  std::vector<std::uint8_t> mask(640 * 480, 1);
  SceneOptions options; options.drawCellRoute = true; options.drawWall = false;
  options.drawOuter = false; options.drawInner = false;
  const auto stage = [&](std::size_t i) {
    if (!scenery) return;
    SceneRasterObserver observer; observer.mask = &mask;
    observer.candidate = [&](auto, auto, auto, auto) { ++out.candidates[i]; };
    observer.accepted = [&](const SceneRasterWriteEvent& e) {
      ++out.writes[i]; out.depthWrites[i] += e.depthAssigned;
      out.leverPixels.accepted(e);
      if (spriteOffsets && spriteOffsets->count(static_cast<std::size_t>(e.source))) out.spritePixels.insert(e.pixel);
    };
    backdrop.renderStage(out.raster, cam, lib, out.stats[i], stages[i], &observer);
  };
  scheduleSceneFrame([&] { stage(0); }, [&] { stage(1); }, [&] { if (compose && tube) tube->render(out.raster, cam, options, &lib); }, [&] { stage(2); });
  for (std::size_t i = 0; i < 3; ++i) {
    require(out.stats[i].rejected == out.stats[i].projectionRejected + out.stats[i].rasterRejected, "rejection attribution");
    require(out.stats[i].acceptedColorWrites == out.writes[i] && out.stats[i].acceptedDepthWrites == out.depthWrites[i], "accepted-event counters");
  }
  return out;
}
std::string idOf(const std::string& name) {
  auto s = name.find_last_of("\\/"); auto base = s == std::string::npos ? name : name.substr(s + 1);
  auto dot = base.rfind('.'); return dot == std::string::npos ? base : base.substr(0, dot);
}
// "<level> <role> <owner>[i] ...: unsupported <form> at 0x..; scenery not admitted..." -> "<role>: <form>"
std::string firstReason(const std::string& d) {
  const auto form = d.find("unsupported");
  if (form == std::string::npos) return d.substr(0, 120);
  const auto roleBegin = d.find(' ') + 1, roleEnd = d.find(' ', roleBegin);
  auto text = d.substr(form, d.find(';', form) - form);
  if (const auto at = text.find(" at 0x"); at != std::string::npos) text.resize(at);
  return d.substr(roleBegin, roleEnd - roleBegin) + ": " + text.substr(0, 160);
}
std::map<std::string, std::string> readLines(const fs::path& path) {
  std::map<std::string, std::string> out; std::ifstream in(path); std::string line;
  while (std::getline(in, line)) { const auto sp = line.find(' '); if (sp != std::string::npos) out[line.substr(0, sp)] = line.substr(sp + 1); }
  return out;
}

#ifndef SCENE_FROZEN
// ---- MM_DIG_000 fixed measured record (docs/scratch/scene-branch-research-002.md section 8) ----
constexpr std::array<std::size_t, 19> kDigNodes = {0xA7555, 0xBC6EA, 0xBC7C7, 0xBC8A4, 0xBC981, 0xBCA5E, 0xBCB3B, 0xBCC18, 0xBCCF5, 0xBCDD2,
                                                   0xC1DE8, 0xC1EC5, 0xC1FA2, 0xC207F, 0xC215C, 0xC2239, 0xC2316, 0xC23F3, 0xC24D0};
// 19 draws from CRT state 1 mapped through trunc(15*rand/32767): the hand-computed worked example.
constexpr std::array<std::uint32_t, 19> kWorkedFrames = {0, 8, 2, 12, 8, 7, 5, 13, 12, 11, 2, 12, 10, 7, 4, 0, 1, 5, 2};

BackdropScene buildDig(const AssetLibrary& lib, const Bytes& bytes, CrtRand& rng) {
  auto built = BackdropScene::build(lib, bytes, "MM_DIG_000");
  require(bool(built) && !built->empty() && built->diagnostics().empty(), "MM_DIG_000 admitted without diagnostics");
  built->setRandom(&rng);
  return std::move(*built);
}

void application(const std::string& gameDir, const fs::path& dir, std::ostream& log) {
  require(std::string(std::getenv("SDL_VIDEODRIVER") ? std::getenv("SDL_VIDEODRIVER") : "") == "dummy", "dummy video required");
  require(std::string(std::getenv("SDL_AUDIODRIVER") ? std::getenv("SDL_AUDIODRIVER") : "") == "dummy", "dummy audio required");
  const auto cameraFile = dir / "fixed-camera.json";
  std::ofstream(cameraFile) << R"({"viewport":[640,480],"center":[319,239],"aspect":1.3333333333333333,"angle_degrees":19.03,"yaw_units":32,"yaw_offset":128,"yaw_units_per_turn":256,"zoom_shift":0,"level_eye_height":96,"height_bias":17.392,"eye_distance":377.516,"fixed_pitch_turns":-0.03135770272046,"pose_pi":3.1415927410125732,"lens_pi":3.1415927410125732,"angle_units_per_turn":360,"elevation":0})" << '\n';
  std::array<Image, 2> first;
  for (int off = 0; off < 2; ++off) {
    std::vector<std::string> args = {"scene-branch-impl", "--game-dir", gameDir, "--level", "MM_DIG_000", "--sim", "--native", "--geom", "route",
                                     "--camera", cameraFile.string(), "--no-audio", "--frames", "40"};
    if (off) args.emplace_back("--scene-off");
    std::vector<char*> argv; for (auto& a : args) argv.push_back(a.data());
    int callbacks = 0; bool queued = false; Image last;
    const auto before = BackdropScene::telemetry();
    const auto observer = [&](const LevelFrameView& view) {
      require(view.levelName == "MM_DIG_000" && view.frame == callbacks, "public level route and frame order");
      if (callbacks == 0) first[off] = view.image;
      last = view.image; ++callbacks;
      if (callbacks == 6) { SDL_Event quit{}; quit.type = SDL_QUIT; queued = SDL_PushEvent(&quit) == 1; }
    };
    ApplicationRunOptions options; options.frameSeconds = 0.1;  // owned test clock: 100 ms per frame
    const auto t0 = Clock::now();
    const auto result = runApplication(static_cast<int>(argv.size()), argv.data(), {}, observer, options);
    const auto elapsed = seconds(t0, Clock::now());
    const auto after = BackdropScene::telemetry();
    require(result == 0 && queued && callbacks == 6, "public --sim route exits normally through the observer-queued SDL_QUIT after 6 frames");
    log << "application,level=MM_DIG_000,sceneOff=" << off << ",return=" << result << ",frames=" << callbacks << ",queuedQuit=" << queued
        << ",seconds=" << elapsed << ",advanceCalls=" << after.advanceCalls - before.advanceCalls
        << ",advancesWithTime=" << after.advancesWithTime - before.advancesWithTime << ",temporalSteps=" << after.temporalSteps - before.temporalSteps
        << ",firstDrawLatches=" << after.firstDrawLatches - before.firstDrawLatches << '\n';
    if (!off) {
      require(after.advancesWithTime - before.advancesWithTime >= 5, "temporal advance ran with positive dt on at least 5 frames");
      require(after.temporalSteps - before.temporalSteps > 0, "sprite temporal indices actually stepped");
      require(after.firstDrawLatches - before.firstDrawLatches == 19, "each of the 19 sprite nodes latched its first-draw frame exactly once");
      require(bool(writePng(dir / "application-first-frame.png", first[0])) && bool(writePng(dir / "application-last-frame.png", last)), "public frame artifacts");
      require(changed(first[0].rgba, last.rgba, 4) > 0, "public frames differ between the first and sixth frame");
    } else {
      require(after.advancesWithTime == before.advancesWithTime && after.firstDrawLatches == before.firstDrawLatches, "--scene-off builds no scenery state");
    }
  }
  const auto pixels = changed(first[0].rgba, first[1].rgba, 4);
  require(pixels > 0, "public route scenery changes rendered pixels");
  log << "applicationChangedPixels,MM_DIG_000=" << pixels << '\n';
}

void digDemonstration(const AssetLibrary& lib, const Bytes& bytes, const CameraBasis& cam, const fs::path& dir, std::ostream& log) {
  // --- parser/geometry record: orientation on the measured fire record (first node 0xA7555) ---
  CrtRand rngA(1);
  auto sceneA = buildDig(lib, bytes, rngA);
  require(sceneA.spriteCount() == 19, "19 branch-1 sprites");
  std::set<std::size_t> spriteOffsets(kDigNodes.begin(), kDigNodes.end());
  {
    std::size_t found = 0;
    sceneA.visitRoots([&](const auto&, const SceneNode& root) {
      for (const auto& n : root.children) found += spriteOffsets.count(n.sourceOffset) && n.type == 2 && n.type2.branch == 1 && n.type2.branchName == "fire";
    });
    require(found == 19, "the 19 measured branch-1 offsets are the 'fire' nodes of the parsed Secondary tree");
  }
  auto sheet = loadSpriteSheet(lib, "SPRITES\\fire.SPR");
  require(bool(sheet) && sheet->sprites.size() == 1 && sheet->sprites[0].animations.size() == 16 && sheet->sprites[0].rate == 1.0, "fire.SPR: 1 clip, 16 temporal entries, rate 1.0");
  for (const auto& a : sheet->sprites[0].animations) require(a.frames.size() == 1, "every temporal entry has exactly one directional image");
  const auto& img0 = sheet->sprites[0].animations[0].frames[0];
  require(lemmings::texture_name_flags(img0.sheet.c_str()) == 1, "texture_name_flags('" + img0.sheet + "') == 1 (not the caret flags-8 row)");
  require(sheetAssetName(img0.sheet) == "TEXTURES\\SHEETS\\_FIRE0.BMP" && lib.contains(sheetAssetName(img0.sheet)), "sheet texture TEXTURES\\SHEETS\\_FIRE0.BMP exists");
  require(img0.unknown0 == 9 && img0.unknown1 == 0 && img0.width == 19 && img0.height == 25, "measured fire image ints X+0x30..0x3C = 9,0,19,25");
  {
    const SceneNode* node = nullptr;
    sceneA.visitRoots([&](const auto&, const SceneNode& root) { for (const auto& n : root.children) if (n.sourceOffset == 0xA7555) node = &n; });
    require(node != nullptr, "first branch-1 node");
    const auto chain = buildBillboardCameraChain(cam);
    BillboardSourceInputs in; in.parentCamera.values = chain.parentCamera; in.sourceTransform.values = chain.sourceTransform;
    in.projectionTransform.values = chain.projectionTransform; in.center = {cam.cx, cam.cy};
    for (bool mirror : {false, true}) {
      auto q = makeSceneSpriteQuad(*node, in, img0, mirror);
      require(bool(q), "first fire record geometry");
      const auto& v = q->vertices;
      // Orientation computed from the literal formulas (no assumption): where the image's
      // left/right/top/bottom texture edges land in screen space (pixel y grows downward).
      const double uL = img0.uLeft, uR = img0.uRight, uT = img0.uTop, uB = img0.uBottom;
      double xLeft = 0, xRight = 0, yTop = 0, yBottom = 0; int nl = 0, nr = 0, nt = 0, nb = 0;
      for (const auto& p : v) {
        if (p.u == uL) { xLeft += p.x; ++nl; } else if (p.u == uR) { xRight += p.x; ++nr; }
        if (p.v == uT) { yTop += p.y; ++nt; } else if (p.v == uB) { yBottom += p.y; ++nb; }
      }
      require(nl == 2 && nr == 2 && nt == 2 && nb == 2, "each texture edge is carried by exactly two vertices");
      xLeft /= nl; xRight /= nr; yTop /= nt; yBottom /= nb;
      const bool leftEdgeAtScreenLeft = xLeft < xRight, topEdgeAtScreenTop = yTop < yBottom;
      log << "orientation,mirror=" << mirror << ",a0..a4=" << q->args[0] << ',' << q->args[1] << ',' << q->args[2] << ',' << q->args[3] << ',' << q->args[4]
          << ",a2sign=" << (q->args[2] < 0 ? -1 : q->args[2] > 0) << ",a4sign=" << (q->args[4] < 0 ? -1 : q->args[4] > 0)
          << ",vertices(x,y,z,u,v)=";
      for (const auto& p : v) log << '(' << p.x << ',' << p.y << ',' << p.z << ',' << p.u << ',' << p.v << ')';
      log << ",imageLeftEdgeAtScreen=" << (leftEdgeAtScreenLeft ? "left" : "right") << ",imageTopEdgeAtScreen=" << (topEdgeAtScreenTop ? "up" : "down")
          << ",result=" << (leftEdgeAtScreenLeft && topEdgeAtScreenTop ? "upright" : !leftEdgeAtScreenLeft && topEdgeAtScreenTop ? "horizontally-flipped" : leftEdgeAtScreenLeft ? "vertically-flipped" : "rotated-180") << '\n';
      // Internal consistency of the computed signs with the literal vertex formulas.
      require((v[0].x < v[3].x) == (q->args[2] > 0) && (v[0].y < v[1].y) == (q->args[4] > 0), "vertex x/y order follows the signs of a2 and a4");
      require(v[0].z > 0 && v[1].z > 0 && v[2].z > 0 && v[3].z > 0, "positive depth");
    }
  }

  // --- injected-state renders: per-role calls, 19 submissions, artifacts ---
  const auto cameraTube = [&]() {
    auto mesh = findCylinderMesh(bytes); auto index = indexLevel(bytes);
    require(bool(mesh) && bool(index), "MM_DIG_000 cylinder/index");
    auto tube = TubeScene::build(lib, *mesh, bytes, index->grid0, true);
    require(bool(tube), "MM_DIG_000 tube builds");
    return std::move(*tube);
  }();
  const auto t0 = Clock::now();
  auto only = render(sceneA, &cameraTube, lib, cam, true, false, &spriteOffsets);
  auto on = render(sceneA, &cameraTube, lib, cam, true, true, &spriteOffsets);
  auto off = render(sceneA, &cameraTube, lib, cam, false, true);
  const auto renderSeconds = seconds(t0, Clock::now());
  require(sceneA.firstSpriteDrawRandomState() == std::optional<std::uint32_t>(1u), "recorded generator state at the first draw is the injected state 1");
  const auto& sec = only.stats[2];
  for (std::size_t i = 0; i < 3; ++i) {
    const auto& s = only.stats[i];
    log << "role," << stageNames[i] << ",candidates=" << s.stageMeshRecords << '/' << s.stageBillboards << '/' << s.stageSprites
        << ",rasterCalls=" << s.rasterCalls << ",spriteRasterCalls=" << s.spriteRasterCalls << ",meshRasterCalls=" << s.rasterCalls - s.spriteRasterCalls
        << ",acceptedColorStores=" << only.writes[i] << ",spriteColorStores=" << s.spriteAcceptedColorWrites
        << ",meshColorStores=" << only.writes[i] - s.spriteAcceptedColorWrites << ",acceptedDepthStores=" << only.depthWrites[i]
        << ",spriteDepthStores=" << s.spriteAcceptedDepthWrites << ",rejected=" << s.rejected << ",missingTextures=" << s.missingTextures << '\n';
    require(s.missingTextures == 0, "no missing sprite/texture in " + std::string(stageNames[i]));
  }
  require(sec.stageSprites == 19 && sec.spriteSubmissions.size() == 19 && sec.sprites == 19, "19 branch-1 submissions in the Secondary stage");
  {
    std::size_t culled = 0;
    for (const auto& sub : sec.spriteSubmissions) culled += sub.culled;
    require(sec.spriteRasterCalls + culled == 38 && sec.spriteRasterCalls <= 38, "19 sprites x 2 triangles = 38 before culling: rasterized + culled");
  }
  require(only.stats[0].spriteRasterCalls == 0 && only.stats[1].spriteRasterCalls == 0, "sprites appear only in the Secondary stage");
  require(sec.spriteAcceptedColorWrites > 0 && sec.spriteAcceptedDepthWrites > 0, "sprite triangles stored accepted colour and depth");
  std::set<std::size_t> seen;
  for (std::size_t i = 0; i < sec.spriteSubmissions.size(); ++i) {
    const auto& sub = sec.spriteSubmissions[i];
    log << "submission," << i << ",offset=0x" << std::hex << sub.sourceOffset << std::dec << ",temporal=" << sub.temporal << ",mirror=" << sub.mirror
        << ",rasterCalls=" << sub.rasterCalls << ",culled=" << sub.culled << ",rejected=" << sub.rejected << ",reason=" << (sub.reason.empty() ? "none" : sub.reason) << '\n';
    require(spriteOffsets.count(sub.sourceOffset) && seen.insert(sub.sourceOffset).second, "each submission is a distinct measured node offset");
    require(sub.temporal == kWorkedFrames[i], "state-1 first-draw frames follow the hand-computed worked sequence in draw order");
    require(!sub.mirror, "f1 == 0 for every measured record (no mirror)");
  }
  // Mesh submissions are the same Secondary stage's non-sprite calls; both are reported above.
  require(sec.rasterCalls - sec.spriteRasterCalls > 0, "Secondary mesh submissions listed apart from sprites");

  // scene-on/off and scene-only artifacts, changed pixels.
  artifact(dir, "MM_DIG_000-scene-only", only.raster, true);
  artifact(dir, "MM_DIG_000-scene-on", on.raster, true);
  artifact(dir, "MM_DIG_000-scene-off", off.raster, true);
  const auto changedPixels = changed(on.raster.framebuffer().rgb, off.raster.framebuffer().rgb);
  require(changedPixels > 0, "scene-on differs from scene-off");
  const auto spritePixelCount = only.spritePixels.size();
  require(spritePixelCount > 0, "sprite triangles changed pixels");
  log << "artifacts,sceneOnlySpritePixels=" << spritePixelCount << ",composedChangedPixels=" << changedPixels << ",renderSeconds(3 renders)=" << renderSeconds << '\n';

  // Determinism: a fresh scene with the same injected state renders byte-identical RGB and depth.
  CrtRand rngA2(1);
  auto sceneA2 = buildDig(lib, bytes, rngA2);
  auto only2 = render(sceneA2, &cameraTube, lib, cam, true, false, &spriteOffsets);
  auto on2 = render(sceneA2, &cameraTube, lib, cam, true, true);
  const auto h1 = hashes(only.raster), h2 = hashes(only2.raster);
  require(only.raster.framebuffer().rgb == only2.raster.framebuffer().rgb && only.raster.mutableDepthBuffer() == only2.raster.mutableDepthBuffer() &&
          on.raster.framebuffer().rgb == on2.raster.framebuffer().rgb && on.raster.mutableDepthBuffer() == on2.raster.mutableDepthBuffer(),
          "two renders under the same injected state are byte-identical (RGB and depth)");
  log << "determinism,sceneOnlyRgb=0x" << hex(h1.rgb) << ",sceneOnlyDepth=0x" << hex(h1.depth) << ",identical=1\n";

  // Mutation: a different injected state changes sprite pixels only.
  CrtRand rngB(12345);
  auto sceneB = buildDig(lib, bytes, rngB);
  auto onlyB = render(sceneB, &cameraTube, lib, cam, true, false, &spriteOffsets);
  require(onlyB.raster.framebuffer().rgb != only.raster.framebuffer().rgb, "a different injected generator state changes the render");
  std::size_t differing = 0, outsideSprites = 0;
  const auto& ra = only.raster.framebuffer().rgb; const auto& rb = onlyB.raster.framebuffer().rgb;
  const auto& da = only.raster.mutableDepthBuffer(); const auto& db = onlyB.raster.mutableDepthBuffer();
  for (std::size_t p = 0; p < ra.size() / 3; ++p) {
    const bool diff = !std::equal(ra.begin() + 3 * p, ra.begin() + 3 * p + 3, rb.begin() + 3 * p) || da[p] != db[p];
    if (!diff) continue;
    ++differing;
    if (!only.spritePixels.count(p) && !onlyB.spritePixels.count(p)) ++outsideSprites;
  }
  require(differing > 0 && outsideSprites == 0, "mutation: every pixel that differs between generator states lies in sprite submissions");
  log << "mutation,stateA=1,stateB=12345,differingPixels=" << differing << ",outsideSpritePixels=" << outsideSprites
      << ",firstFramesB=";
  for (const auto& sub : onlyB.stats[2].spriteSubmissions) log << sub.temporal << ' ';
  log << '\n';
  artifact(dir, "MM_DIG_000-scene-only-stateB", onlyB.raster, false);

  // Temporal advance changes the picture (a frame later) deterministically.
  CrtRand rngC(1);
  auto sceneC = buildDig(lib, bytes, rngC);
  auto beforeAdvance = render(sceneC, &cameraTube, lib, cam, true, false, &spriteOffsets);
  sceneC.advanceScenery(100.0, true, true);
  const auto pausedState = *sceneC.spriteState(0xA7555);
  sceneC.advanceScenery(100.0, false, false);
  require(pausedState.temporal == sceneC.spriteState(0xA7555)->temporal && pausedState.accumulator == sceneC.spriteState(0xA7555)->accumulator, "paused / not-in-level frames do not advance the sprites");
  const auto temporalBefore = sceneC.spriteState(0xA7555)->temporal;
  sceneC.advanceScenery(100.0, true, false);
  require(sceneC.spriteState(0xA7555)->temporal == (temporalBefore + 1) % 16, "100 ms at rate 1.0 advances exactly one image (0.1 > 0.06, remainder 0.04)");
  sceneC.advanceScenery(5000.0, true, false);  // capped at 100 ms
  auto afterAdvance = render(sceneC, &cameraTube, lib, cam, true, false, &spriteOffsets);
  require(afterAdvance.raster.framebuffer().rgb != beforeAdvance.raster.framebuffer().rgb, "advancing the temporal state changes the sprite pixels");
  log << "advance,pausedNoChange=1,oneFrameStep=1,cappedAt100ms=1,changedAfterAdvance=1\n";
}
#endif
}  // namespace

int main() try {
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR"); require(gameDir && *gameDir, "installed assets required");
  auto lib = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box"); require(bool(lib), "open installed archives");
#ifdef SCENE_FROZEN
  const std::string side = "before";
#else
  const std::string side = "after";
#endif
  const fs::path root = SCENE_BRANCH_IMPL_DIR, dir = root / side; fs::create_directories(dir);
  std::ofstream log(dir / "measurements.txt"); log << std::setprecision(17);
  const auto cam = buildCamera(CameraParams{});  // explicit fixed source camera for every level and side

  // ---- Phase 1: build every effective installed level ----
  const auto names = levelFileNames(*lib);
  log << "side=" << side << " effectiveLevels=" << names.size() << '\n';
  std::map<std::string, BackdropScene> admitted; std::map<std::string, Bytes> admittedBytes;
  std::map<std::string, std::size_t> emptyReasons; std::size_t emptyCount = 0;
  std::ofstream levels(dir / "levels.txt");
  const auto buildStart = Clock::now();
  for (const auto& name : names) {
    const auto id = idOf(name);
    auto bytes = lib->read(name); require(bool(bytes), "read actual " + id);
    const auto t0 = Clock::now();
    auto backdrop = BackdropScene::build(*lib, *bytes, id);
    const auto ms = seconds(t0, Clock::now()) * 1000;
    require(bool(backdrop), "no installed level may hard-error: " + id + (backdrop ? "" : ": " + backdrop.error()));
#ifndef SCENE_FROZEN
    const auto outcome = sceneadmission::structuralOutcome(*bytes, &*lib);
    require(!outcome.malformed && backdrop->empty() == !outcome.supported, "production admission equals the independent structural outcome: " + id);
#endif
    if (backdrop->empty()) {
      ++emptyCount;
      require(!backdrop->diagnostics().empty() && backdrop->diagnostics().front().find("empty partial scene") != std::string::npos, "empty level carries a diagnostic: " + id);
      const auto& d = backdrop->diagnostics().front();
      ++emptyReasons[firstReason(d)];
      levels << id << " admitted=0 diagnostic=" << d << '\n';
      log << "LEVEL " << id << " admitted=0 buildMs=" << ms << " diagnostic=" << d << '\n';
    } else {
      levels << id << " admitted=1\n";
      log << "LEVEL " << id << " admitted=1 buildMs=" << ms << '\n';
      admittedBytes[id] = std::move(*bytes); admitted.emplace(id, std::move(*backdrop));
    }
  }
  levels.close();
  const auto buildSeconds = seconds(buildStart, Clock::now());
  log << "admittedCount=" << admitted.size() << " emptyCount=" << emptyCount << " buildAllSeconds=" << buildSeconds << '\n';
  for (const auto& [k, v] : emptyReasons) log << "emptyReason \"" << k.substr(0, 200) << "\" levels=" << v << '\n';
  log << "admittedIds="; for (const auto& [id, _] : admitted) log << id << ','; log << '\n';

#ifdef SCENE_FROZEN
  // Entry state: ten structurally admitted levels (scene-admission-001 accepted outcome).
  require(admitted.size() == 10 && emptyCount == names.size() - 10, "frozen task-entry admission is the ten accepted levels");
  require(admitted.count("MM_CLIMB_002") && !admitted.count("MM_DIG_000"), "frozen: MM_CLIMB_002 admitted, MM_DIG_000 not");
  {
    auto dig = lib->read("COMPILED\\MM_DIG_000.LVL"); require(bool(dig), "read MM_DIG_000");
    auto frozen = BackdropScene::build(*lib, *dig, "MM_DIG_000");
    require(frozen && frozen->empty() && frozen->diagnostics().size() == 1 && frozen->diagnostics()[0].find("MM_DIG_000") != std::string::npos &&
            frozen->diagnostics()[0].find("Secondary") != std::string::npos && frozen->diagnostics()[0].find("unsupported nonzero type-2 branch 1 at 0xa7572") != std::string::npos,
            "frozen MM_DIG_000 is the diagnosed empty partial scene with exactly one diagnostic");
    log << "frozenMM_DIG_000," << frozen->diagnostics()[0] << '\n';
  }
#else
  require(admitted.count("MM_DIG_000") == 1, "current admits MM_DIG_000");
#endif

  // ---- Phase 2: render every admitted level, record RGB/depth hashes (and raw MM_CLIMB_002) ----
  std::ofstream hashFile(dir / "hashes.txt");
  double renderSeconds = 0;
#ifndef SCENE_FROZEN
  // Levels admitted by the frozen run get the full scene-only / composed on / composed off comparison;
  // levels admitted only now (a tube-free scene-only render keeps this driver inside its time guard).
  std::set<std::string> frozenAdmitted;
  for (const auto& [id, line] : readLines(root / "before" / "levels.txt")) if (line.find("admitted=1") == 0) frozenAdmitted.insert(id);
  require(frozenAdmitted.size() == 10, "frozen levels file lists ten admitted levels");
#endif
  for (const auto& [id, backdrop] : admitted) {
    const auto& bytes = admittedBytes.at(id);
    const auto r0 = Clock::now();
#ifndef SCENE_FROZEN
    if (!frozenAdmitted.count(id)) {
      auto scene = render(backdrop, nullptr, *lib, cam, true, false);
      const auto h = hashes(scene.raster);
      hashFile << id << ":scene-only rgb=" << hex(h.rgb) << " depth=" << hex(h.depth) << '\n';
      std::size_t nonblack = 0; const auto& v = scene.raster.framebuffer().rgb;
      for (std::size_t i = 0; i < v.size(); i += 3) nonblack += (v[i] | v[i + 1] | v[i + 2]) != 0;
      log << "NEWLY " << id << " sprites=" << backdrop.spriteCount() << " rasterCalls(P/L/S)=" << scene.stats[0].rasterCalls << '/' << scene.stats[1].rasterCalls << '/'
          << scene.stats[2].rasterCalls << " spriteCalls=" << scene.stats[2].spriteRasterCalls << " missingTextures="
          << scene.stats[0].missingTextures + scene.stats[1].missingTextures + scene.stats[2].missingTextures << " sceneOnlyNonblack=" << nonblack << '\n';
      renderSeconds += seconds(r0, Clock::now());
      continue;
    }
#endif
    auto mesh = findCylinderMesh(bytes); auto index = indexLevel(bytes);
    std::expected<TubeScene, std::string> tube = std::unexpected(std::string("no tube input"));
    if (mesh && index) tube = TubeScene::build(*lib, *mesh, bytes, index->grid0, true);
    require(bool(tube), "production tube builds for admitted level " + id);
    auto scene = render(backdrop, &*tube, *lib, cam, true, false);
    auto on = render(backdrop, &*tube, *lib, cam, true, true);
    auto off = render(backdrop, &*tube, *lib, cam, false, true);
    for (const auto& [mode, r] : {std::pair<const char*, Rendered*>{"scene-only", &scene}, {"scene-on", &on}, {"scene-off", &off}}) {
      const auto h = hashes(r->raster);
      hashFile << id << ':' << mode << " rgb=" << hex(h.rgb) << " depth=" << hex(h.depth) << '\n';
      // Retain authentic frozen pixels and the precise provenance of the new
      // lever stores; whole-frame equality used to encode invisible levers.
      artifact(dir, id + "-" + mode, r->raster, true);
      r->leverPixels.save(dir / (id + "-" + mode + ".lever-mask"));
    }
    if (id == "MM_CLIMB_002") {
      artifact(dir, id + "-scene-only", scene.raster, true);
      require(scene.stats[0].stageMeshRecords == 923 && scene.stats[0].stageBillboards == 0 && scene.stats[1].stageMeshRecords == 172 &&
              scene.stats[1].stageBillboards == 0 && scene.stats[2].stageMeshRecords == 82 && scene.stats[2].stageBillboards == 17, "MM_CLIMB_002 retains 923/0, 172/0, 82/17");
      require(scene.stats[0].stageMeshRecords + scene.stats[1].stageMeshRecords + scene.stats[2].stageMeshRecords == 1177, "1177 records");
      Raster primary(640, 480); primary.clear(0, 0, 0); BackdropStats st; backdrop.renderStage(primary, cam, *lib, st, stages[0]);
      std::uint64_t h = 1469598103934665603ull;
      const auto add = [&](std::uint8_t v) { h = (h ^ v) * 1099511628211ull; };
      for (auto v : primary.framebuffer().rgb) add(v);
      for (double d : primary.mutableDepthBuffer()) { auto bits = std::bit_cast<std::uint64_t>(d); for (int i = 0; i < 8; ++i) add(bits >> (8 * i)); }
      require(h == 0x1fd0bfb82ab39e95ull, "MM_CLIMB_002 primary RGB/depth hash 0x1FD0BFB82AB39E95");
    }
    renderSeconds += seconds(r0, Clock::now());
  }
  hashFile.close();
  log << "renderAllSeconds=" << renderSeconds << " admittedRendered=" << admitted.size() << '\n';

#ifndef SCENE_FROZEN
  // ---- Phase 3: compare with the frozen run ----
  const auto before = readLines(root / "before" / "hashes.txt"), now = readLines(dir / "hashes.txt");
  const auto beforeLevels = readLines(root / "before" / "levels.txt"), nowLevels = readLines(dir / "levels.txt");
  require(!before.empty() && before.size() == 30, "frozen hashes available (10 levels x 3 modes)");
  std::size_t compared = 0, exact = 0;
  for (const auto& [key, value] : before) {
    const bool identical = now.count(key) && now.at(key) == value;
    require(now.count(key) && (identical || leverbaseline::unchangedOutsideLeverStores(
        root / "before", dir, key, log)), "admitted-level RGB/depth equals frozen outside accepted lever stores: " + key);
    exact += identical;
    ++compared;
  }
  log << "frozenCurrentBaselineChecks=" << compared << " exactHashes=" << exact
      << " (10 admitted levels x scene-only/scene-on/scene-off, RGB and depth)\n";
  std::vector<std::string> newly;
  for (const auto& [id, line] : nowLevels) if (line.find("admitted=1") == 0 && beforeLevels.at(id).find("admitted=0") == 0) newly.push_back(id);
  log << "newlyAdmitted=" << newly.size() << ':'; for (const auto& id : newly) log << ' ' << id; log << '\n';
  require(std::find(newly.begin(), newly.end(), "MM_DIG_000") != newly.end(), "MM_DIG_000 is newly admitted versus the frozen run");
  for (const auto& id : newly) require(now.count(id + ":scene-only"), "newly admitted level rendered scene-only: " + id);
  require(newly.size() == admitted.size() - 10, "every admitted level is either frozen-admitted or newly admitted");

  // ---- Phase 4: MM_DIG_000 demonstration ----
  auto dig = lib->read("COMPILED\\MM_DIG_000.LVL"); require(bool(dig), "read MM_DIG_000");
  digDemonstration(*lib, *dig, cam, dir, log);
  // ---- Phase 5: bounded public route ----
  application(gameDir, dir, log);
#endif
  log.close();
  std::ifstream report(dir / "measurements.txt"); std::cout << report.rdbuf();
  std::cout << "\nPASS scene-branch-impl " << side << '\n';
  return 0;
} catch (const std::exception& e) { std::cerr << "FAIL scene-branch-impl: " << e.what() << '\n'; return 1; }
