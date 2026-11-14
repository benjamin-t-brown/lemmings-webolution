// scene-nested-impl-001: authentic frozen (supervisor task-entry BackdropScene/SceneDispatch/SceneNode)
// and current loaders in separate executables over EVERY effective installed level, plus the
// E11_KF_1 nested type-1 windmill demonstration (current side only).
// Fixed source camera; source-built rendering evidence only. No original code runs; no claim of
// original admission, pixel agreement, x87 equivalence, clipping or full-frame fidelity; C3 stays FAIL.
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
#include <iterator>
#include <map>
#include <memory>
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
Bytes slurp(const fs::path& path) {
  std::ifstream in(path, std::ios::binary); require(bool(in), "read " + path.string());
  std::vector<char> raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  Bytes out(raw.size()); for (std::size_t i = 0; i < raw.size(); ++i) out[i] = static_cast<std::byte>(raw[i]);
  return out;
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
  // Secondary-stage probes: node draw order (first candidate), records/triangles per node, accepted stores per record.
  std::vector<std::size_t> nodeOrder;
  std::map<std::size_t, std::size_t> nodeRecords, nodeTriangles, writesBySource;
};
Rendered render(const BackdropScene& backdrop, const TubeScene* tube, const AssetLibrary& lib, const CameraBasis& cam,
                bool scenery, bool compose) {
  Rendered out; out.raster.clear(0, 0, 0); out.leverPixels.bind(backdrop);
  std::vector<std::uint8_t> mask(640 * 480, 1);
  SceneOptions options; options.drawCellRoute = true; options.drawWall = false;
  options.drawOuter = false; options.drawInner = false;
  const auto stage = [&](std::size_t i) {
    if (!scenery) return;
    SceneRasterObserver observer; observer.mask = &mask;
    observer.candidate = [&, i](std::uint64_t, std::uint64_t node, std::uint64_t, std::uint64_t triangles) {
      ++out.candidates[i];
      if (i != 2) return;
      if (!out.nodeRecords.count(node)) out.nodeOrder.push_back(node);
      ++out.nodeRecords[node]; out.nodeTriangles[node] += triangles;
    };
    observer.accepted = [&, i](const SceneRasterWriteEvent& e) {
      ++out.writes[i]; out.depthWrites[i] += e.depthAssigned;
      out.leverPixels.accepted(e);
      if (i == 2) ++out.writesBySource[static_cast<std::size_t>(e.source)];
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
std::size_t nonblack(const Raster& r) {
  std::size_t n = 0; const auto& v = r.framebuffer().rgb;
  for (std::size_t i = 0; i < v.size(); i += 3) n += (v[i] | v[i + 1] | v[i + 2]) != 0;
  return n;
}

#ifndef SCENE_FROZEN
// ---- E11_KF_1 constants: docs/scratch/scene-nested-research-001.md section 2 and trees/E11_KF_1.plan.txt ----
constexpr std::size_t kBody = 0xb60e7, kBladesBegin = 0xb6ed3, kBladesEnd = 0xb7576;
// Independently written literal: trees/E11_KF_1.plan.txt order (cube1_trans, its child, then the other 27 direct children).
constexpr std::array<std::size_t, 29> kPlan = {0xb60e7, 0xb6ed3, 0xbfd87, 0xbf80a, 0xbef09, 0xbe608, 0xb3465, 0xb37a9, 0xbad5f, 0xba372,
                                               0xbc3a1, 0xbb74c, 0xbdaff, 0xbcff6, 0xb809f, 0xb7576, 0xb7dd6, 0xb7754, 0xb7665, 0xb7930,
                                               0xb7842, 0xb4dea, 0xb3aed, 0xb7bfa, 0xb7ce8, 0xb7fb2, 0xb7ec4, 0xb7b0c, 0xb7a1e};

std::size_t sumWrites(const Rendered& r, std::size_t begin, std::size_t end) {
  std::size_t n = 0;
  for (const auto& [src, count] : r.writesBySource) if (src >= begin && src < end) n += count;
  return n;
}
std::vector<std::size_t> dispatchOrder(const BackdropScene& scene, const CameraBasis& cam, std::vector<SecondaryDispatchItem>* items = nullptr) {
  std::vector<std::size_t> order;
  scene.visitSecondaryDispatch(cam, [&](const SecondaryDispatchItem& item) { order.push_back(item.nodeSourceOffset); if (items) items->push_back(item); });
  return order;
}
BackdropScene buildE11(const AssetLibrary& lib, const Bytes& bytes) {
  auto built = BackdropScene::build(lib, bytes, "E11_KF_1");
  require(bool(built) && !built->empty() && built->diagnostics().empty(), "E11_KF_1 admitted without diagnostics");
  return std::move(*built);
}

void application(const std::string& gameDir, const fs::path& dir, std::ostream& log) {
  require(std::string(std::getenv("SDL_VIDEODRIVER") ? std::getenv("SDL_VIDEODRIVER") : "") == "dummy", "dummy video required");
  require(std::string(std::getenv("SDL_AUDIODRIVER") ? std::getenv("SDL_AUDIODRIVER") : "") == "dummy", "dummy audio required");
  const auto cameraFile = dir / "fixed-camera.json";
  std::ofstream(cameraFile) << R"({"viewport":[640,480],"center":[319,239],"aspect":1.3333333333333333,"angle_degrees":19.03,"yaw_units":32,"yaw_offset":128,"yaw_units_per_turn":256,"zoom_shift":0,"level_eye_height":96,"height_bias":17.392,"eye_distance":377.516,"fixed_pitch_turns":-0.03135770272046,"pose_pi":3.1415927410125732,"lens_pi":3.1415927410125732,"angle_units_per_turn":360,"elevation":0})" << '\n';
  std::array<Image, 2> first;
  for (int off = 0; off < 2; ++off) {
    std::vector<std::string> args = {"scene-nested-impl", "--game-dir", gameDir, "--level", "E11_KF_1", "--sim", "--native", "--geom", "route",
                                     "--camera", cameraFile.string(), "--no-audio", "--frames", "40"};
    if (off) args.emplace_back("--scene-off");
    std::vector<char*> argv; for (auto& a : args) argv.push_back(a.data());
    int callbacks = 0; bool queued = false; Image last;
    const auto observer = [&](const LevelFrameView& view) {
      require(view.levelName == "E11_KF_1" && view.frame == callbacks, "public level route and frame order");
      if (callbacks == 0) first[off] = view.image;
      last = view.image; ++callbacks;
      if (callbacks == 4) { SDL_Event quit{}; quit.type = SDL_QUIT; queued = SDL_PushEvent(&quit) == 1; }
    };
    ApplicationRunOptions options; options.frameSeconds = 0.1;
    const auto t0 = Clock::now();
    const auto result = runApplication(static_cast<int>(argv.size()), argv.data(), {}, observer, options);
    const auto elapsed = seconds(t0, Clock::now());
    require(result == 0 && queued && callbacks == 4, "public --sim route exits normally through the observer-queued SDL_QUIT after 4 frames");
    log << "application,level=E11_KF_1,sceneOff=" << off << ",return=" << result << ",frames=" << callbacks << ",queuedQuit=" << queued << ",seconds=" << elapsed << '\n';
    if (!off) require(bool(writePng(dir / "application-first-frame.png", first[0])) && bool(writePng(dir / "application-last-frame.png", last)), "public frame artifacts");
  }
  const auto pixels = changed(first[0].rgba, first[1].rgba, 4);
  require(pixels > 0, "public E11_KF_1 route: admitted scenery changes rendered pixels versus --scene-off");
  log << "applicationChangedPixels,E11_KF_1=" << pixels << '\n';
}

// Synthetic type-2-with-children: a real E11_KF_1 type-2 leaf (0xb7576) gets child count 1 and a copy of itself
// appended as its child. The level must become a diagnosed, successful, EMPTY partial scene.
void type2WithChildren(const AssetLibrary& lib, const Bytes& bytes, std::ostream& log) {
  auto leaf = readSceneNode(bytes, 0xb7576);
  require(bool(leaf) && leaf->root.type == 2 && leaf->root.children.empty(), "0xb7576 is a type-2 leaf");
  const std::size_t end = leaf->endOffset;
  require(end >= 4 + 0xb7576 && std::to_integer<int>(bytes[end - 4]) == 0 && std::to_integer<int>(bytes[end - 3]) == 0, "type-2 leaf ends with a zero child count");
  Bytes mutated = bytes;
  const Bytes copy(bytes.begin() + 0xb7576, bytes.begin() + static_cast<std::ptrdiff_t>(end));
  mutated[end - 4] = std::byte{1};
  mutated.insert(mutated.begin() + static_cast<std::ptrdiff_t>(end), copy.begin(), copy.end());
  auto result = BackdropScene::build(lib, mutated, "E11_KF_1");
  require(bool(result), "type-2-with-children is not a hard error");
  require(result->empty() && result->diagnostics().size() == 1, "type-2-with-children yields a diagnosed empty partial scene");
  const auto& d = result->diagnostics()[0];
  require(d.find("E11_KF_1") != std::string::npos && d.find("Secondary") != std::string::npos && d.find("unsupported type-2 node with children") != std::string::npos &&
          d.find("empty partial scene") != std::string::npos, "level-, role- and form-qualified diagnostic: " + d);
  log << "type2WithChildren," << d << '\n';
}

void e11Demonstration(const AssetLibrary& lib, const Bytes& bytes, const CameraBasis& cam, const fs::path& dir, std::ostream& log) {
  auto scene = buildE11(lib, bytes);
  auto mesh = findCylinderMesh(bytes); auto index = indexLevel(bytes);
  require(bool(mesh) && bool(index), "E11_KF_1 cylinder/index");
  auto tubeBuilt = TubeScene::build(lib, *mesh, bytes, index->grid0, true);
  require(bool(tubeBuilt), "E11_KF_1 tube builds");
  const TubeScene& tube = *tubeBuilt;

  // --- dispatch order equals the independently written literal plan; depth/parent bookkeeping ---
  std::vector<SecondaryDispatchItem> items;
  const auto order = dispatchOrder(scene, cam, &items);
  require(order.size() == kPlan.size() && std::equal(order.begin(), order.end(), kPlan.begin()), "dispatch order equals trees/E11_KF_1.plan.txt");
  require(items[0].nodeSourceOffset == kBody && items[0].depth == 1 && items[1].nodeSourceOffset == kBladesBegin && items[1].depth == 2 && items[1].parentOffset == kBody,
          "windmill child is emitted immediately after cube1_trans at depth 2 with its parent offset");
  for (std::size_t i = 2; i < items.size(); ++i) require(items[i].depth == 1, "other items are direct children");
  log << "dispatchOrder=";
  for (auto o : order) log << " 0x" << std::hex << o << std::dec;
  log << '\n';

  // --- tie order: the evidenced insertion differs from std::sort exactly where keys tie ---
  std::vector<std::size_t> fileOffsets; std::map<std::size_t, double> keyOf;
  scene.visitRoots([&](const auto& source, const SceneNode& root) {
    if (source.stage != BackdropScene::Stage::Secondary) return;
    for (const auto& c : root.children) fileOffsets.push_back(c.sourceOffset);
  });
  for (const auto& item : items) if (item.depth == 1) keyOf[item.nodeSourceOffset] = item.key;
  require(fileOffsets.size() == 28 && keyOf.size() == 28, "28 direct children");
  std::vector<double> fileKeys; for (auto o : fileOffsets) fileKeys.push_back(keyOf.at(o));
  std::vector<std::size_t> sorted(28); for (std::size_t i = 0; i < 28; ++i) sorted[i] = i;
  std::sort(sorted.begin(), sorted.end(), [&](std::size_t a, std::size_t b) { return fileKeys[a] > fileKeys[b]; });
  const auto insertion = binaryInsertionOrder(fileKeys);
  std::size_t tiedPairs = 0;
  for (std::size_t i = 0; i < 28; ++i) for (std::size_t j = i + 1; j < 28; ++j) tiedPairs += fileKeys[i] == fileKeys[j];
  std::size_t positionsDiffering = 0; for (std::size_t i = 0; i < 28; ++i) positionsDiffering += sorted[i] != insertion[i];
  require(tiedPairs >= 9 && sorted != insertion, "std::sort order differs from the evidenced insertion order on E11_KF_1 ties");
  // Both orders agree wherever keys differ: the differing positions hold bit-identical keys only.
  for (std::size_t i = 0; i < 28; ++i) require(fileKeys[sorted[i]] == fileKeys[insertion[i]], "orders agree on every key value position");
  log << "ties,exactEqualKeyPairs=" << tiedPairs << ",stdSortVsInsertionPositionsDiffering=" << positionsDiffering << '\n';

  // --- renders: scene-only, scene-on, scene-off, determinism ---
  auto only = render(scene, &tube, lib, cam, true, false);
  auto on = render(scene, &tube, lib, cam, true, true);
  auto off = render(scene, &tube, lib, cam, false, true);
  auto only2 = render(scene, &tube, lib, cam, true, false);
  auto on2 = render(scene, &tube, lib, cam, true, true);
  require(only.raster.framebuffer().rgb == only2.raster.framebuffer().rgb && only.raster.mutableDepthBuffer() == only2.raster.mutableDepthBuffer() &&
          on.raster.framebuffer().rgb == on2.raster.framebuffer().rgb && on.raster.mutableDepthBuffer() == on2.raster.mutableDepthBuffer(),
          "two renders are byte-identical (RGB and depth)");
  for (std::size_t i = 0; i < 3; ++i) {
    const auto& s = only.stats[i];
    log << "role," << stageNames[i] << ",candidates(mesh/billboard/sprite)=" << s.stageMeshRecords << '/' << s.stageBillboards << '/' << s.stageSprites
        << ",rasterCalls=" << s.rasterCalls << ",acceptedColorStores=" << only.writes[i] << ",acceptedDepthStores=" << only.depthWrites[i]
        << ",rejected=" << s.rejected << "(projection " << s.projectionRejected << ",raster " << s.rasterRejected << "),predicateRejected=" << s.predicateRejected
        << ",missingTextures=" << s.missingTextures << ",diagnostics=" << s.diagnostics.size() << '\n';
    require(s.missingTextures == 0, "no missing resources in " + std::string(stageNames[i]));
  }
  const auto& sec = only.stats[2];
  require(sec.stageMeshRecords > 0 && sec.rasterCalls > 0, "Secondary scenery is submitted");
  // The candidate (draw) order of nodes equals the literal plan restricted to the nodes that submit records.
  std::vector<std::size_t> expectedDrawn;
  for (auto o : kPlan) if (only.nodeRecords.count(o)) expectedDrawn.push_back(o);
  require(only.nodeOrder == expectedDrawn, "rasterizer submission order equals the literal plan order");
  require(only.nodeRecords.count(kBody) && only.nodeRecords.count(kBladesBegin), "body and blades both submit records");
  require(only.nodeRecords.at(kBladesBegin) == 5, "windmill_roll_0.2_trans submits its 5 kind-7 records");
  const std::size_t bodyStores = sumWrites(only, kBody, kBladesBegin), bladeStores = sumWrites(only, kBladesBegin, kBladesEnd), allStores = only.writes[2];
  log << "windmillBody,node=0xb60e7,records=" << only.nodeRecords.at(kBody) << ",candidateTriangles=" << only.nodeTriangles.at(kBody) << ",acceptedColorStores=" << bodyStores << '\n';
  log << "windmillBlades,node=0xb6ed3,records=" << only.nodeRecords.at(kBladesBegin) << ",candidateTriangles=" << only.nodeTriangles.at(kBladesBegin) << ",acceptedColorStores=" << bladeStores << '\n';
  log << "otherDirectChildren,acceptedColorStores=" << allStores - bodyStores - bladeStores << '\n';
  const auto changedPixels = changed(on.raster.framebuffer().rgb, off.raster.framebuffer().rgb);
  require(changedPixels > 0 && nonblack(only.raster) > 0, "scene-on differs from scene-off and the scene-only render is not blank");
  // Culls: ungated dispatch minus gated dispatch (type-1 radius cull and visibility).
  std::size_t gated = order.size(), ungated = 0;
  scene.visitRoots([&](const auto& source, const SceneNode& root) {
    if (source.stage != BackdropScene::Stage::Secondary) return;
    const auto chain = buildBillboardCameraChain(cam);
    const auto row = [](const std::array<double, 12>& a) { return std::array<double, 12>{a[0], a[4], a[8], a[1], a[5], a[9], a[2], a[6], a[10], a[3], a[7], a[11]}; };
    dispatchSecondaryChildren(root, row(chain.parentCamera), row(chain.sourceTransform), [&](const SecondaryDispatchItem&) { ++ungated; });
  });
  log << "culls,dispatchedWithGates=" << gated << ",dispatchedWithoutGates=" << ungated << ",culledOrHidden=" << ungated - gated << '\n';
  require(ungated == 29 && gated <= ungated, "ungated traversal visits 29 nodes");
  artifact(dir, "E11_KF_1-scene-only", only.raster, true);
  artifact(dir, "E11_KF_1-scene-on", on.raster, true);
  artifact(dir, "E11_KF_1-scene-off", off.raster, true);
  log << "artifacts,sceneOnlyNonblack=" << nonblack(only.raster) << ",composedChangedPixels=" << changedPixels << ",sceneOnlyRgb=0x" << hex(hashes(only.raster).rgb)
      << ",sceneOnlyDepth=0x" << hex(hashes(only.raster).depth) << ",identical=1\n";

  // --- mutation: hide cube1_trans (node and subtree) ---
  {
    auto m = buildE11(lib, bytes);
    m.setSecondaryHidden([](const SceneNode& n) { return n.sourceOffset == kBody; });
    const auto mo = dispatchOrder(m, cam);
    require(mo.size() == 27 && std::find(mo.begin(), mo.end(), kBody) == mo.end() && std::find(mo.begin(), mo.end(), kBladesBegin) == mo.end(), "hiding cube1_trans removes it and its child from the dispatch");
    auto r = render(m, &tube, lib, cam, true, false);
    require(!r.nodeRecords.count(kBody) && !r.nodeRecords.count(kBladesBegin) && r.stats[2].rasterCalls < only.stats[2].rasterCalls, "hiding cube1_trans removes body and blades from submission");
    require(r.raster.framebuffer().rgb != only.raster.framebuffer().rgb || (bodyStores + bladeStores) == 0, "hiding cube1_trans changes pixels when the windmill stores any");
    log << "mutation,hideBody,dispatched=" << mo.size() << ",secondaryRasterCalls=" << r.stats[2].rasterCalls << " (full " << sec.rasterCalls << "),changedPixels="
        << changed(r.raster.framebuffer().rgb, only.raster.framebuffer().rgb) << '\n';
  }
  // --- mutation: hide only the child ---
  {
    auto m = buildE11(lib, bytes);
    m.setSecondaryHidden([](const SceneNode& n) { return n.sourceOffset == kBladesBegin; });
    const auto mo = dispatchOrder(m, cam);
    require(mo.size() == 28 && mo[0] == kBody && std::find(mo.begin(), mo.end(), kBladesBegin) == mo.end(), "hiding the child removes only the child");
    auto r = render(m, &tube, lib, cam, true, false);
    require(r.nodeRecords.count(kBody) && !r.nodeRecords.count(kBladesBegin) && r.stats[2].rasterCalls < only.stats[2].rasterCalls, "hiding the child removes only the blades' submissions");
    require(sumWrites(r, kBladesBegin, kBladesEnd) == 0 && sumWrites(r, kBody, kBladesBegin) <= bodyStores, "no accepted blade stores remain");
    std::size_t outside = 0;  // pixels changed vs the full render must be explained by blade submissions
    const auto& a = only.raster.framebuffer().rgb; const auto& b = r.raster.framebuffer().rgb;
    for (std::size_t p = 0; p < a.size() / 3; ++p) outside += !std::equal(a.begin() + 3 * p, a.begin() + 3 * p + 3, b.begin() + 3 * p);
    require(bladeStores > 0 || outside == 0, "blade removal changes no pixel when the blades stored nothing");
    log << "mutation,hideBlades,dispatched=" << mo.size() << ",secondaryRasterCalls=" << r.stats[2].rasterCalls << " (full " << sec.rasterCalls << "),changedPixels=" << outside << '\n';
  }
  type2WithChildren(lib, bytes, log);
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
  const fs::path root = SCENE_NESTED_IMPL_DIR, dir = root / side; fs::create_directories(dir / "rgb");
  std::ofstream log(dir / "measurements.txt"); log << std::setprecision(17);
  const auto cam = buildCamera(CameraParams{});

  // ---- Phase 1: build every effective installed level ----
  const auto names = levelFileNames(*lib);
  require(names.size() == 102, "102 effective installed levels");
  log << "side=" << side << " effectiveLevels=" << names.size() << '\n';
  std::map<std::string, BackdropScene> admitted; std::map<std::string, Bytes> admittedBytes;
  std::map<std::string, std::size_t> emptyReasons; std::size_t emptyCount = 0;
  std::ofstream levels(dir / "levels.txt");
  const auto buildStart = Clock::now();
  for (const auto& name : names) {
    const auto id = idOf(name);
    auto bytes = lib->read(name); require(bool(bytes), "read actual " + id);
    auto backdrop = BackdropScene::build(*lib, *bytes, id);
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
      log << "LEVEL " << id << " admitted=0 diagnostic=" << d << '\n';
    } else {
      levels << id << " admitted=1\n";
      log << "LEVEL " << id << " admitted=1\n";
      admittedBytes[id] = std::move(*bytes); admitted.emplace(id, std::move(*backdrop));
    }
  }
  levels.close();
  log << "admittedCount=" << admitted.size() << " emptyCount=" << emptyCount << " buildAllSeconds=" << seconds(buildStart, Clock::now()) << '\n';
  for (const auto& [k, v] : emptyReasons) log << "emptyReason \"" << k.substr(0, 200) << "\" levels=" << v << '\n';
  log << "admittedIds="; for (const auto& [id, _] : admitted) log << id << ','; log << '\n';

  static const std::set<std::string> acceptedTen = {"E06_KF_1", "H17_MG_1", "M06_KF_1", "M08_KF_1", "MM_BASH_002", "MM_CLIMB_002", "MM_E1_3_SKILLS", "MM_E1_4SKILLS", "MM_W_012", "VH22_JC"};
#ifdef SCENE_FROZEN
  require(admitted.size() == 84 && emptyCount == 18, "frozen task-entry admission is 84 levels with 18 nested/non-leaf empty partial scenes");
  for (const auto& id : acceptedTen) require(admitted.count(id) == 1, "frozen admits the originally accepted level " + id);
  {
    auto e11 = lib->read("COMPILED\\E11_KF_1.LVL"); require(bool(e11), "read E11_KF_1");
    auto frozen = BackdropScene::build(*lib, *e11, "E11_KF_1");
    require(frozen && frozen->empty() && frozen->diagnostics().size() == 1 && frozen->diagnostics()[0].find("E11_KF_1 Secondary paired-secondary[0] source=0xb33dc: unsupported nested/non-leaf secondary dispatch") != std::string::npos,
            "frozen E11_KF_1 is the diagnosed empty partial scene with exactly one diagnostic");
    log << "frozenE11_KF_1," << frozen->diagnostics()[0] << '\n';
  }
#else
  require(admitted.count("E11_KF_1") == 1, "current admits E11_KF_1");
  require(admitted.size() + emptyCount == 102, "every level is admitted or a diagnosed empty partial scene");
#endif

  // ---- Phase 2: render every level admitted by the frozen run; the current run also renders the newly admitted ones ----
  std::ofstream hashFile(dir / "hashes.txt");
  double renderSeconds = 0;
  std::set<std::string> frozenAdmitted;
#ifndef SCENE_FROZEN
  for (const auto& [id, line] : readLines(root / "before" / "levels.txt")) if (line.find("admitted=1") == 0) frozenAdmitted.insert(id);
  require(frozenAdmitted.size() == 84, "frozen levels file lists 84 admitted levels");
#endif
  for (const auto& [id, backdrop] : admitted) {
    const auto& bytes = admittedBytes.at(id);
    const auto r0 = Clock::now();
    const bool ten = acceptedTen.count(id) != 0;
    std::unique_ptr<TubeScene> tubePtr;
    if (ten) {
      auto mesh = findCylinderMesh(bytes); auto index = indexLevel(bytes);
      std::expected<TubeScene, std::string> tube = std::unexpected(std::string("no tube input"));
      if (mesh && index) tube = TubeScene::build(*lib, *mesh, bytes, index->grid0, true);
      require(bool(tube), "production tube builds for admitted level " + id);
      tubePtr = std::make_unique<TubeScene>(std::move(*tube));
    }
    auto scene = render(backdrop, tubePtr.get(), *lib, cam, true, false);
    const auto h = hashes(scene.raster);
    hashFile << id << ":scene-only rgb=" << hex(h.rgb) << " depth=" << hex(h.depth) << '\n';
#ifdef SCENE_FROZEN
    binary(dir / "rgb" / (id + ".rgb"), scene.raster.framebuffer().rgb.data(), scene.raster.framebuffer().rgb.size());
#else
    std::size_t missing = 0; for (const auto& s : scene.stats) missing += s.missingTextures;
    if (!frozenAdmitted.count(id)) {
      log << "NEWLY " << id << " rasterCalls(P/L/S)=" << scene.stats[0].rasterCalls << '/' << scene.stats[1].rasterCalls << '/' << scene.stats[2].rasterCalls
          << " acceptedStores(P/L/S)=" << scene.writes[0] << '/' << scene.writes[1] << '/' << scene.writes[2] << " missingTextures=" << missing << " sceneOnlyNonblack=" << nonblack(scene.raster) << '\n';
      require(missing == 0, "newly admitted level has no missing resources: " + id);
    } else if (!ten) {
      const auto before = readLines(root / "before" / "hashes.txt");
      const auto it = before.find(id + ":scene-only");
      require(it != before.end(), "frozen scene-only hash present for " + id);
      if (it->second != "rgb=" + hex(h.rgb) + " depth=" + hex(h.depth)) {
        const auto frozenRgb = slurp(root / "before" / "rgb" / (id + ".rgb"));
        const auto& now = scene.raster.framebuffer().rgb;
        require(frozenRgb.size() == now.size(), "frozen RGB artifact size");
        std::size_t px = 0;
        for (std::size_t p = 0; p < now.size() / 3; ++p)
          for (int c = 0; c < 3; ++c) if (static_cast<std::uint8_t>(frozenRgb[3 * p + c]) != now[3 * p + c]) { ++px; break; }
        log << "TIECHANGED " << id << " scene-only RGB pixels differing from frozen=" << px << '\n';
      } else log << "TIEUNCHANGED " << id << '\n';
    }
#endif
    if (ten) {
      artifact(dir, id + "-scene-only", scene.raster, true);
      scene.leverPixels.save(dir / (id + "-scene-only.lever-mask"));
      auto on = render(backdrop, tubePtr.get(), *lib, cam, true, true);
      auto off = render(backdrop, tubePtr.get(), *lib, cam, false, true);
      for (const auto& [mode, r] : {std::pair<const char*, Rendered*>{"scene-on", &on}, {"scene-off", &off}}) {
        const auto hh = hashes(r->raster);
        hashFile << id << ':' << mode << " rgb=" << hex(hh.rgb) << " depth=" << hex(hh.depth) << '\n';
        artifact(dir, id + "-" + mode, r->raster, true);
        r->leverPixels.save(dir / (id + "-" + mode + ".lever-mask"));
      }
      if (id == "MM_CLIMB_002") {
        require(scene.stats[0].stageMeshRecords == 923 && scene.stats[0].stageBillboards == 0 && scene.stats[1].stageMeshRecords == 172 &&
                scene.stats[1].stageBillboards == 0 && scene.stats[2].stageMeshRecords == 82 && scene.stats[2].stageBillboards == 17, "MM_CLIMB_002 retains 923/0, 172/0, 82/17");
        require(scene.stats[0].stageMeshRecords + scene.stats[1].stageMeshRecords + scene.stats[2].stageMeshRecords == 1177, "1177 records");
        Raster primary(640, 480); primary.clear(0, 0, 0); BackdropStats st; backdrop.renderStage(primary, cam, *lib, st, stages[0]);
        std::uint64_t hh = 1469598103934665603ull;
        const auto add = [&](std::uint8_t v) { hh = (hh ^ v) * 1099511628211ull; };
        for (auto v : primary.framebuffer().rgb) add(v);
        for (double d : primary.mutableDepthBuffer()) { auto bits = std::bit_cast<std::uint64_t>(d); for (int i = 0; i < 8; ++i) add(bits >> (8 * i)); }
        require(hh == 0x1fd0bfb82ab39e95ull, "MM_CLIMB_002 primary RGB/depth hash 0x1FD0BFB82AB39E95");
      }
    }
    renderSeconds += seconds(r0, Clock::now());
  }
  hashFile.close();
  log << "renderAllSeconds=" << renderSeconds << " admittedRendered=" << admitted.size() << '\n';

#ifndef SCENE_FROZEN
  // ---- Phase 3: ten-level RGB/depth equality outside the repaired lever omission ----
  const auto before = readLines(root / "before" / "hashes.txt"), now = readLines(dir / "hashes.txt");
  std::size_t compared = 0, exact = 0;
  for (const auto& [key, value] : before) {
    const auto id = key.substr(0, key.find(':'));
    if (!acceptedTen.count(id)) continue;
    const bool identical = now.count(key) && now.at(key) == value;
    require(now.count(key) && (identical || leverbaseline::unchangedOutsideLeverStores(
        root / "before", dir, key, log)), "ten-level RGB/depth equals frozen outside accepted lever stores: " + key);
    exact += identical;
    ++compared;
  }
  require(compared == 30, "10 levels x scene-only/scene-on/scene-off compared");
  log << "frozenCurrentBaselineChecks=" << compared << " exactHashes=" << exact << '\n';
  // ---- Phase 4: E11_KF_1 demonstration and bounded public route ----
  auto e11 = lib->read("COMPILED\\E11_KF_1.LVL"); require(bool(e11), "read E11_KF_1");
  e11Demonstration(*lib, *e11, cam, dir, log);
  application(gameDir, dir, log);
#endif
  log.close();
  std::ifstream report(dir / "measurements.txt"); std::cout << report.rdbuf();
  std::cout << "\nPASS scene-nested-impl " << side << '\n';
  return 0;
} catch (const std::exception& e) { std::cerr << "FAIL scene-nested-impl: " << e.what() << '\n'; return 1; }
