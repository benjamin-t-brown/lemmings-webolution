// Measure nested Secondary trees from the installed archive overlay. Use
// read_compiled_level and readSceneNode, then follow the original traversal:
// 0x0046B73F root slot 8 sorts direct children, 0x0046B6CC traverses base
// children, 0x0046C55D is type-1 slot 4, 0x0047D0C0 is type-2 slot 4, and
// 0x0046B590 constructs the base. Write to SCENE_NESTED_DIR.
#include "app/Camera.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneMeshSource.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/SceneNode.h"
#include "assets/Sprite.h"
#include "lemmings/compiled_level.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;
using namespace webo::app;
namespace {
using Matrix = std::array<double, 12>;
Matrix identity() { return {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}; }
Matrix compose(const Matrix& a, const Matrix& b) {
  Matrix o{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) for (int k = 0; k < 3; ++k) o[3 * i + j] += a[3 * i + k] * b[3 * k + j];
    o[9 + i] = b[9 + i];
    for (int k = 0; k < 3; ++k) o[9 + i] += a[9 + k] * b[3 * k + i];
  }
  return o;
}
BillboardAffine3x4 affine(const Matrix& m) { return {{{m[0], m[3], m[6], m[9], m[1], m[4], m[7], m[10], m[2], m[5], m[8], m[11]}}}; }
BillboardAffine3x4 multiply(const BillboardAffine3x4& a, const BillboardAffine3x4& b) {
  BillboardAffine3x4 o;
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) for (int k = 0; k < 3; ++k) o.values[4 * r + c] += a.values[4 * r + k] * b.values[4 * k + c];
    o.values[4 * r + 3] = a.values[4 * r + 3];
    for (int k = 0; k < 3; ++k) o.values[4 * r + 3] += a.values[4 * r + k] * b.values[4 * k + 3];
  }
  return o;
}
Matrix nativeAffine(const BillboardAffine3x4& a) {
  return {a.values[0], a.values[4], a.values[8], a.values[1], a.values[5], a.values[9], a.values[2], a.values[6], a.values[10], a.values[3], a.values[7], a.values[11]};
}

std::string sha256Hex(const std::uint8_t* data, std::size_t n) {
  static constexpr std::uint32_t k[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  auto rotr = [](std::uint32_t x, int s) { return (x >> s) | (x << (32 - s)); };
  std::uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  std::vector<std::uint8_t> msg(data, data + n);
  const std::uint64_t bitLen = static_cast<std::uint64_t>(n) * 8;
  msg.push_back(0x80);
  while ((msg.size() % 64) != 56) msg.push_back(0);
  for (int i = 7; i >= 0; --i) msg.push_back(static_cast<std::uint8_t>((bitLen >> (i * 8)) & 0xFF));
  for (std::size_t off = 0; off < msg.size(); off += 64) {
    std::uint32_t w[64];
    for (std::size_t i = 0; i < 16; ++i)
      w[i] = (std::uint32_t(msg[off + i * 4]) << 24) | (std::uint32_t(msg[off + i * 4 + 1]) << 16) |
             (std::uint32_t(msg[off + i * 4 + 2]) << 8) | msg[off + i * 4 + 3];
    for (std::size_t i = 16; i < 64; ++i) {
      const auto s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      const auto s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (std::size_t i = 0; i < 64; ++i) {
      const auto t1 = hh + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ (~e & g)) + k[i] + w[i];
      const auto t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
      hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
  }
  std::string out;
  for (auto v : h) out += std::format("{:08x}", v);
  return out;
}
std::string idOf(const std::string& name) {
  auto s = name.find_last_of("\\/");
  auto base = s == std::string::npos ? name : name.substr(s + 1);
  auto dot = base.rfind('.');
  return dot == std::string::npos ? base : base.substr(0, dot);
}

// The production Secondary admission predicate (BackdropScene.cpp "nested/non-leaf"), verbatim.
bool productionNestedPredicate(const SceneNode& root) {
  return root.type != 0 || std::any_of(root.children.begin(), root.children.end(), [](const auto& n) {
    return !n.children.empty() || (n.type != 1 && n.type != 2);
  });
}

// 0x0046B73F..0x0046BAD1 inserts child i by descending key in file order.
// 0x0046B8F4..0x0046BA72 narrows lo/hi with key[i]<key[idx[mid]].
// Insert after hi if key[i]<key[idx[hi]], before lo if key[i]>key[idx[lo]],
// and after lo otherwise. Call child slot 4 in array order.
std::vector<std::size_t> originalOrder(const std::vector<double>& key) {
  std::vector<std::size_t> idx;
  if (key.empty()) return idx;
  idx.push_back(0);
  for (std::size_t i = 1; i < key.size(); ++i) {
    long lo = 0, hi = static_cast<long>(idx.size()) - 1;
    while (hi - lo > 1) {
      const long mid = (hi + lo) >> 1;
      if (key[i] < key[idx[mid]]) lo = mid; else hi = mid;
    }
    long pos;
    if (key[i] < key[idx[hi]]) pos = hi + 1;
    else if (key[i] > key[idx[lo]]) pos = lo;
    else pos = lo + 1;
    idx.insert(idx.begin() + pos, i);
  }
  return idx;
}
std::vector<std::size_t> stdSortOrder(const std::vector<double>& key) {
  std::vector<std::size_t> idx(key.size());
  for (std::size_t i = 0; i < idx.size(); ++i) idx[i] = i;
  std::sort(idx.begin(), idx.end(), [&](std::size_t a, std::size_t b) { return key[a] > key[b]; });
  return idx;
}

struct TreeStats {
  std::size_t nodes = 0, maxDepth = 0;
  std::array<std::size_t, 4> byType{};              // 0,1,2, other
  std::array<std::size_t, 4> withChildrenByType{};  // nodes that have children, by type
  std::array<std::size_t, 4> nestedByType{};        // depth >= 2
  std::map<std::string, std::size_t> pairs;         // "parentType>childType"
  std::size_t hiddenType2 = 0, type2Branch0 = 0, type2Branch1 = 0, cutme = 0, cutmeNested = 0, type1WithMesh = 0;
  std::size_t directChildren = 0, directWithChildren = 0, directNotType12 = 0;
};
std::size_t ti(std::uint32_t t) { return t < 3 ? t : 3; }
void gather(const SceneNode& n, std::size_t depth, const SceneNode* parent, TreeStats& s) {
  ++s.nodes;
  s.maxDepth = std::max(s.maxDepth, depth);
  ++s.byType[ti(n.type)];
  if (!n.children.empty()) ++s.withChildrenByType[ti(n.type)];
  if (depth >= 2) ++s.nestedByType[ti(n.type)];
  if (parent) ++s.pairs[std::format("{}>{}", parent->type, n.type)];
  if (n.type == 2) {
    if (n.type2.visibility == 0) ++s.hiddenType2;
    if (n.type2.branch == 0) ++s.type2Branch0; else ++s.type2Branch1;
  }
  if (n.type == 1 && n.name.find("cutme") != std::string::npos) { ++s.cutme; if (depth >= 2) ++s.cutmeNested; }
  for (const auto& c : n.children) gather(c, depth + 1, &n, s);
}

struct NodeLine {
  std::size_t idx, depth, parent;
  const SceneNode* node;
};
void flatten(const SceneNode& n, std::size_t depth, std::size_t parent, std::vector<NodeLine>& out) {
  const std::size_t me = out.size();
  out.push_back({me, depth, parent, &n});
  for (const auto& c : n.children) flatten(c, depth + 1, me, out);
}

std::string describe(const AssetLibrary& lib, const SceneNode& n) {
  std::string s;
  if (n.type == 1) {
    std::size_t recs = 0;
    std::array<std::size_t, 8> kinds{};
    for (const auto& b : n.meshes) { recs += b.records.size(); if (b.kind < 8) kinds[b.kind] += b.records.size(); }
    s += std::format(" positions={} normals={} materials={} blocks={} records={} k5={} k6={} k7={}", n.positions.size(), n.normals.size(),
                     n.materials.size(), n.meshes.size(), recs, kinds[5], kinds[6], kinds[7]);
    for (const auto& m : n.materials) {
      const bool has = m.name.empty() || lib.contains("TEXTURES\\" + m.name + ".BMP");
      s += std::format(" mat[{}{}sel={}]", m.name.empty() ? "<null>" : m.name, has ? "" : "(MISSING)", m.blendSelector);
    }
  } else if (n.type == 2) {
    s += std::format(" vis={} branch={} A={:.6g} B={:.6g} H={:.6g} trailing={}", int(n.type2.visibility), int(n.type2.branch), n.type2.leading[0], n.type2.leading[1], n.type2.leading[2], n.type2.trailingWord);
    if (n.type2.branch == 0) {
      s += std::format(" res='{}' uv=({:.6g},{:.6g},{:.6g},{:.6g})", n.type2.branchName, n.type2.branchDoubles[0], n.type2.branchDoubles[1], n.type2.branchDoubles[2], n.type2.branchDoubles[3]);
      if (!n.type2.branchName.empty() && !lib.contains("TEXTURES\\" + n.type2.branchName + ".BMP")) s += "(MISSING)";
    } else {
      s += std::format(" sprite='{}' f=({},{},{},{})", n.type2.branchName, n.type2.branchWords[0], n.type2.branchWords[1], n.type2.branchWords[2], n.type2.branchWords[3]);
    }
  }
  return s;
}

// Report the first BackdropScene::build failure after the nested guard.
std::string nonNestedFailure(const AssetLibrary& lib, const SceneNode& root, const BillboardSourceInputs& chain, std::size_t& missingTextures) {
  std::string err;
  const auto sprites = [&](const auto& self, const SceneNode& n) -> void {
    if (!err.empty()) return;
    if (n.type == 2 && n.type2.branch != 0) {
      const auto no = [&](const std::string& why) { err = std::format("unsupported type-2 branch 1 sprite '{}': {}", n.type2.branchName, why); };
      const std::string asset = "SPRITES\\" + n.type2.branchName + ".SPR";
      if (!lib.contains(asset)) { no("sprite file missing"); return; }
      auto sheet = loadSpriteSheet(lib, asset);
      if (!sheet) { no("does not parse"); return; }
      const auto ci = n.type2.branchWords[1];
      if (ci >= sheet->sprites.size()) { no("clip outside sheet"); return; }
      if (sheet->sprites[ci].animations.empty()) { no("clip empty"); return; }
      for (const auto& a : sheet->sprites[ci].animations) {
        if (a.frames.size() != 1) { no(std::format("{} directional images", a.frames.size())); return; }
        if (!lib.contains(sheetAssetName(a.frames[0].sheet))) { no("texture missing"); return; }
      }
    }
    for (const auto& c : n.children) self(self, c);
  };
  sprites(sprites, root);
  if (!err.empty()) return err;
  auto meshes = buildSceneMeshSource(root, identity(), identity());
  if (!meshes) return "mesh adapter: " + meshes.error();
  const auto walk = [&](const auto& self, const SceneNode& n, const Matrix& ancestors) -> void {
    if (n.type == 1) for (const auto& m : n.materials) if (!m.name.empty() && !lib.contains("TEXTURES\\" + m.name + ".BMP")) ++missingTextures;
    if (n.type == 2 && n.type2.branch == 0) {
      auto in = chain;
      in.parentCamera = multiply(chain.parentCamera, affine(ancestors));
      auto b = makeSceneBillboardSource(n, in);
      if (!b) { if (err.empty()) err = std::format("billboard adapter at 0x{:x}: {}", n.sourceOffset, b.error()); }
      else if (!b->textureName.empty() && !lib.contains("TEXTURES\\" + b->textureName + ".BMP")) ++missingTextures;
    }
    const Matrix next = compose(n.matrix, ancestors);
    for (const auto& c : n.children) self(self, c, next);
  };
  walk(walk, root, identity());
  return err;
}

struct RootDepth { std::string role; std::size_t nodes = 0, maxDepth = 0, withChildren = 0; };

struct LevelResult {
  std::string id;
  std::size_t payloadBytes = 0;
  std::string payloadSha;
  bool hasSecondary = false;
  std::size_t secBegin = 0, secEnd = 0;
  TreeStats st;
  bool affected = false;
  std::string nonNested;
  std::size_t missingTextures = 0;
  std::size_t exactTies = 0;          // pairs of direct children with bit-identical sort keys
  bool orderDiffers = false;          // original binary insertion order != std::sort(key>) order
  std::vector<RootDepth> otherRoots;
};
}  // namespace

int main() try {
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cout << "SKIP: WEBOLUTION_GAME_DIR unset\n"; return 0; }
  const fs::path dir = SCENE_NESTED_DIR;
  fs::create_directories(dir / "trees");

  // Reference identity (bytes only).
  {
    std::ifstream f(fs::path(gameDir) / "Lemmings Revolution.exe", std::ios::binary);
    std::vector<char> exe((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    const auto sha = sha256Hex(reinterpret_cast<const std::uint8_t*>(exe.data()), exe.size());
    std::cout << "reference exe bytes=" << exe.size() << " sha256=" << sha << '\n';
    if (sha != "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f") { std::cerr << "FAIL reference executable hash mismatch\n"; return 1; }
  }
  auto lib = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!lib) { std::cerr << "FAIL open archives: " << lib.error() << '\n'; return 1; }

  const auto camera = buildCamera(CameraParams{});
  const auto built = buildBillboardCameraChain(camera);
  BillboardSourceInputs chain;
  chain.parentCamera.values = built.parentCamera;
  chain.sourceTransform.values = built.sourceTransform;
  chain.projectionTransform.values = built.projectionTransform;
  chain.center = {camera.cx, camera.cy};
  const Matrix parentCamera = nativeAffine(BillboardAffine3x4{built.parentCamera});
  const Matrix sourceRotation = nativeAffine(BillboardAffine3x4{built.sourceTransform});

  const auto names = levelFileNames(*lib);
  std::vector<LevelResult> results;
  std::map<std::string, SceneNode> keepTrees;  // trees of affected + reference levels, for dumping
  for (const auto& name : names) {
    auto bytes = lib->read(name);
    if (!bytes) { std::cerr << "FAIL read " << name << ": " << bytes.error() << '\n'; return 1; }
    LevelResult r;
    r.id = idOf(name);
    r.payloadBytes = bytes->size();
    lemmings::CompiledLevel level;
    try { level = lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes->data()), bytes->size()}); }
    catch (const std::exception& e) { std::cerr << "FAIL " << r.id << " container: " << e.what() << '\n'; return 1; }
    const auto depthOf = [&](const std::string& role, const lemmings::ObjectSourceExtent& e) {
      auto p = readSceneNode(*bytes, e.typed_root_begin);
      RootDepth d; d.role = role;
      if (p) { TreeStats s; gather(p->root, 0, nullptr, s); d.nodes = s.nodes; d.maxDepth = s.maxDepth; for (auto c : s.withChildrenByType) d.withChildren += c; }
      else d.role += " (parse: " + p.error() + ")";
      r.otherRoots.push_back(d);
    };
    depthOf("Primary", level.objects.source_extent);
    if (level.objects.second_object_source_extent) {
      const auto& e = *level.objects.second_object_source_extent;
      r.hasSecondary = true;
      r.secBegin = e.typed_root_begin; r.secEnd = e.tree_end;
      auto parsed = readSceneNode(*bytes, e.typed_root_begin);
      if (!parsed) { std::cerr << "FAIL " << r.id << " Secondary parse: " << parsed.error() << '\n'; return 1; }
      if (parsed->endOffset != e.tree_end) { std::cerr << "FAIL " << r.id << " Secondary tree end mismatch\n"; return 1; }
      gather(parsed->root, 0, nullptr, r.st);
      r.st.directChildren = parsed->root.children.size();
      for (const auto& c : parsed->root.children) { if (!c.children.empty()) ++r.st.directWithChildren; if (c.type != 1 && c.type != 2) ++r.st.directNotType12; }
      r.affected = productionNestedPredicate(parsed->root);
      // Sort keys of the direct children under the default camera (0x0046B805..0x0046B87B).
      std::vector<double> keys;
      const Matrix parent = compose(parsed->root.matrix, parentCamera);
      for (const auto& c : parsed->root.children) {
        const Matrix composed = compose(c.matrix, parent);
        keys.push_back((composed[9] * sourceRotation[2] + composed[10] * sourceRotation[5]) + composed[11] * sourceRotation[8] + c.headDoubles[1]);
      }
      for (std::size_t i = 0; i < keys.size(); ++i) for (std::size_t j = i + 1; j < keys.size(); ++j) if (keys[i] == keys[j]) ++r.exactTies;
      r.orderDiffers = originalOrder(keys) != stdSortOrder(keys);
      r.nonNested = nonNestedFailure(*lib, parsed->root, chain, r.missingTextures);
      if (r.affected || r.id == "MM_CLIMB_002" || r.id == "MM_DIG_000") {
        r.payloadSha = sha256Hex(reinterpret_cast<const std::uint8_t*>(bytes->data()), bytes->size());
        keepTrees.emplace(r.id, std::move(parsed->root));
      }
    }
    for (std::size_t i = 0; i < level.gadgets.entities_425e50.size(); ++i)
      if (level.gadgets.entities_425e50[i].model && level.gadgets.entities_425e50[i].model->object) depthOf(std::format("Legacy 425e50[{}]", i), level.gadgets.entities_425e50[i].model->object->source_extent);
    for (std::size_t i = 0; i < level.gadgets.entities_426c6b.size(); ++i)
      if (level.gadgets.entities_426c6b[i].model && level.gadgets.entities_426c6b[i].model->object) depthOf(std::format("Legacy 426c6b[{}]", i), level.gadgets.entities_426c6b[i].model->object->source_extent);
    results.push_back(std::move(r));
  }
  std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) { return a.id < b.id; });

  std::ostringstream out;
  out << "scene-nested-research-001 measurement (source-built, read-only; not native admission)\n";
  out << "effectiveLevels=" << results.size() << " camera=default buildCamera(CameraParams{}) (sort keys only)\n";
  std::vector<std::string> affected;
  for (const auto& r : results) {
    out << std::format("LEVEL {} bytes={} secondary={} nodes={} maxDepth={} type0/1/2/other={}/{}/{}/{} withChildren(t0/t1/t2)={}/{}/{} nestedNodes(t0/t1/t2)={}/{}/{} hiddenType2={} branch0={} branch1={} cutme={} cutmeNested={} directChildren={} directWithChildren={} directNotType12={} exactKeyTies={} insertionVsStdSortDiffers={} affected={} firstNonNestedFailure=\"{}\" missingTextures={}",
                       r.id, r.payloadBytes, r.hasSecondary, r.st.nodes, r.st.maxDepth, r.st.byType[0], r.st.byType[1], r.st.byType[2], r.st.byType[3],
                       r.st.withChildrenByType[0], r.st.withChildrenByType[1], r.st.withChildrenByType[2], r.st.nestedByType[0], r.st.nestedByType[1], r.st.nestedByType[2],
                       r.st.hiddenType2, r.st.type2Branch0, r.st.type2Branch1, r.st.cutme, r.st.cutmeNested, r.st.directChildren, r.st.directWithChildren, r.st.directNotType12,
                       r.exactTies, r.orderDiffers, r.affected, r.nonNested, r.missingTextures);
    std::string pairs;
    for (const auto& [k, v] : r.st.pairs) pairs += std::format(" {}={}", k, v);
    out << " pairs[parent>child]:" << pairs << '\n';
    for (const auto& d : r.otherRoots) out << std::format("  ROOT {} nodes={} maxDepth={} nodesWithChildren={}\n", d.role, d.nodes, d.maxDepth, d.withChildren);
    if (r.affected) affected.push_back(r.id);
  }
  out << "AFFECTED " << affected.size() << ':';
  for (const auto& a : affected) out << ' ' << a;
  out << '\n';

  // Per-affected detail: identity, trees, sort orders.
  struct Cand { std::string id; std::size_t nested; };
  std::vector<Cand> eligible;
  for (const auto& r : results) {
    if (!r.affected && r.id != "MM_CLIMB_002" && r.id != "MM_DIG_000") continue;
    const SceneNode& root = keepTrees.at(r.id);
    out << std::format("DETAIL {} payloadSha256={} secondaryTypedRoot=0x{:x} treeEnd=0x{:x}\n", r.id, r.payloadSha, r.secBegin, r.secEnd);
    std::vector<NodeLine> flat;
    flatten(root, 0, SIZE_MAX, flat);
    std::ostringstream tree;
    tree << std::format("{} Secondary tree typedRoot=0x{:x} treeEnd=0x{:x} payloadSha256={}\n", r.id, r.secBegin, r.secEnd, r.payloadSha);
    for (const auto& l : flat) {
      const SceneNode& n = *l.node;
      tree << std::format("{:4} d{} p{} [0x{:x},0x{:x}) type={} name='{}' children={} radius(+100)={:.9g} sortBias(+108)={:.9g} t=({:.6g},{:.6g},{:.6g})",
                          l.idx, l.depth, l.parent == SIZE_MAX ? -1 : static_cast<long>(l.parent), n.sourceOffset, n.endOffset, n.type, n.name, n.children.size(),
                          n.headDoubles[0], n.headDoubles[1], n.matrix[9], n.matrix[10], n.matrix[11])
           << describe(*lib, n) << '\n';
    }
    // Direct-child sort keys and orders.
    std::vector<double> keys;
    const Matrix parent = compose(root.matrix, parentCamera);
    for (const auto& c : root.children) {
      const Matrix composed = compose(c.matrix, parent);
      keys.push_back((composed[9] * sourceRotation[2] + composed[10] * sourceRotation[5]) + composed[11] * sourceRotation[8] + c.headDoubles[1]);
    }
    const auto orig = originalOrder(keys), std_ = stdSortOrder(keys);
    tree << "DIRECT-CHILD ORDER (index in file order: key) original-binary-insertion:";
    for (auto i : orig) tree << std::format(" {}:{:.9g}", i, keys[i]);
    tree << "\nstd::sort(key>) differs=" << (orig != std_) << '\n';
    std::ofstream(dir / "trees" / (r.id + ".txt")) << tree.str();

    // 0x0046B73F sorts root children. Descendants follow in depth-first file order.
    // +0x110==0 skips a subtree. A type-2 parent pushes its matrix twice.
    std::ostringstream plan;
    plan << r.id << " traversal plan (planned order of node slot-4 calls)\n";
    std::size_t visited = 0, skippedSubtrees = 0;
    const auto visit = [&](const auto& self, const SceneNode& n, std::size_t depth, const Matrix& top) -> void {
      const bool vis = n.type != 2 || n.type2.visibility != 0;  // type 0/1 keep the constructor default 1
      plan << std::format("{}VISIT 0x{:x} type={} name='{}' vis={}\n", std::string(depth * 2, ' '), n.sourceOffset, n.type, n.name, vis);
      if (!vis) { ++skippedSubtrees; return; }
      ++visited;
      const Matrix pushed = compose(n.matrix, top);                        // 0x43BC10 in slot 4
      const Matrix childTop = n.type == 2 ? compose(n.matrix, pushed) : pushed;  // 0x47D85C -> 0x46B6CC pushes again
      for (const auto& c : n.children) self(self, c, depth + 1, childTop);
    };
    for (auto i : orig) visit(visit, root.children[i], 0, parent);
    plan << std::format("visited={} skippedHiddenSubtrees={}\n", visited, skippedSubtrees);
    std::ofstream(dir / "trees" / (r.id + ".plan.txt")) << plan.str();
    if (r.affected && r.nonNested.empty() && r.missingTextures == 0) eligible.push_back({r.id, r.st.nestedByType[0] + r.st.nestedByType[1] + r.st.nestedByType[2]});
  }
  std::sort(eligible.begin(), eligible.end(), [](const auto& a, const auto& b) { return a.nested != b.nested ? a.nested < b.nested : a.id < b.id; });
  out << "ELIGIBLE (affected, no other first-failure, 0 missing textures):";
  for (const auto& c : eligible) out << ' ' << c.id << '(' << c.nested << ')';
  out << '\n';
  if (!eligible.empty()) out << "REPRESENTATIVE " << eligible.front().id << " (fewest nested nodes among eligible)\n";
  std::ofstream(dir / "summary.txt") << out.str();
  std::cout << out.str();

  // Require exactly the 18 nested cases and preserve the reference-level counts.
  const std::vector<std::string> expected = {"E11_KF_1", "E22_KF_1", "H05_KF_1", "H09_KF_1", "H20_KF_1", "H24_KF_1", "M04_KF_1", "M11_KF_1", "M16_MG_1",
                                             "M21_KF_1", "MM_BLOCK_BOMB_A", "MM_FLOAT_CLIMB2", "MM_H_003", "VH04_JC1", "VH05_JC1", "VH18_JC", "VH19_JC", "VH21_JC"};
  if (results.size() != 102) { std::cerr << "FAIL effective level count " << results.size() << " != 102\n"; return 1; }
  if (affected != expected) { std::cerr << "FAIL affected set differs from the accepted 18-level census\n"; return 1; }
  if (eligible.empty()) { std::cerr << "FAIL no eligible representative among the 18\n"; return 1; }
  std::cout << "PASS scenenested_measure\n";
  return 0;
} catch (const std::exception& e) {
  std::cerr << "FAIL scenenested_measure: " << e.what() << '\n';
  return 1;
}
