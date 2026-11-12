// scene-branch-research-002: read-only measurement of the installed MM_DIG_000
// Secondary scenery tree including type-2 branch-1 records. Source-built; reads
// the effective archive overlay (patch over base) and the reference executable's
// bytes (never executes it). Production readers are not changed: the branch-1
// traversal below is a measurement-only reader whose layout comes from the
// address-backed contract in docs/scratch/scene-branch-research-002.md
// (reader 0x0047B44C, branch-1 reader 0x00468BF2, common base 0x0046BB73).
// Writes only under build/scene-branch-research-002 (SCENE_BRANCH_DIR).
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Sprite.h"
#include "lemmings/compiled_level.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;
namespace {
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
  std::uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
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

// ---- measurement-only scene tree reader (types 0/1/2, branch 0/1) ----------
struct Node {
  std::size_t begin = 0, end = 0, parent = SIZE_MAX, depth = 0;
  std::uint32_t type = 0;
  // type 1
  std::size_t positions = 0, normals = 0, materials = 0, meshBlocks = 0, meshRecords = 0;
  std::vector<std::string> materialNames;
  // type 2
  std::size_t t2Begin = 0, branchByteOffset = 0;
  double A = 0, B = 0, H = 0;
  std::uint8_t visibility = 0, branch = 0;
  std::string branch0Name;
  std::array<double, 4> branch0Uv{};
  std::uint32_t f1 = 0, f2 = 0, f3 = 0, f4 = 0;
  std::size_t fieldsOffset = 0, nameOffset1 = 0;
  std::string spriteName;
  std::uint32_t trailing = 0;
  std::size_t trailingOffset = 0;
  // common base
  std::array<double, 12> matrix{};
  std::string name;
  std::array<double, 2> head{};
  std::uint32_t childCount = 0;
  std::size_t commonOffset = 0;
};

struct Tree {
  std::span<const std::byte> b;
  std::size_t p = 0;
  std::vector<Node> nodes;
  std::string err;
  bool need(std::size_t n, const char* w) {
    if (p > b.size() || n > b.size() - p) { err = std::format("truncated {} at 0x{:x}", w, p); return false; }
    return true;
  }
  bool u32(std::uint32_t& v, const char* w) { if (!need(4, w)) return false; std::memcpy(&v, b.data() + p, 4); p += 4; return true; }
  bool u8(std::uint8_t& v, const char* w) { if (!need(1, w)) return false; v = std::to_integer<std::uint8_t>(b[p++]); return true; }
  bool f64(double& v, const char* w) { if (!need(8, w)) return false; std::memcpy(&v, b.data() + p, 8); p += 8; return true; }
  bool str(std::string& s, const char* w, std::uint32_t cap) {
    std::uint32_t n; if (!u32(n, w)) return false;
    if (n > cap) { err = std::format("{} length {} exceeds capacity {} at 0x{:x}", w, n, cap, p - 4); return false; }
    if (!need(n, w)) return false;
    s.assign(reinterpret_cast<const char*>(b.data() + p), n); p += n; return true;
  }
  bool node(std::size_t parent, std::size_t depth) {
    if (depth > 64 || nodes.size() > 5000) { err = "tree limit"; return false; }
    const std::size_t me = nodes.size();
    nodes.emplace_back();
    nodes[me].begin = p; nodes[me].parent = parent; nodes[me].depth = depth;
    std::uint32_t type; if (!u32(type, "type")) return false;
    nodes[me].type = type;
    if (type == 1) {
      std::uint32_t n;
      if (!u32(n, "pos count") || n > (b.size() - p) / 24) { if (err.empty()) err = "pos count"; return false; }
      nodes[me].positions = n; p += std::size_t(n) * 24;
      if (!u32(n, "normal count") || n > (b.size() - p) / 24) { if (err.empty()) err = "normal count"; return false; }
      nodes[me].normals = n; p += std::size_t(n) * 24;
      std::uint32_t mc; if (!u32(mc, "material count") || mc > 100000) { if (err.empty()) err = "material count"; return false; }
      nodes[me].materials = mc;
      for (std::uint32_t i = 0; i < mc; ++i) {
        std::string s; std::uint32_t sel;
        if (!str(s, "material name", 100000) || !u32(sel, "material selector")) return false;
        nodes[me].materialNames.push_back(s);
      }
      std::uint32_t nb; if (!u32(nb, "mesh block count") || nb > 100000) { if (err.empty()) err = "mesh block count"; return false; }
      nodes[me].meshBlocks = nb;
      for (std::uint32_t bi = 0; bi < nb; ++bi) {
        std::uint32_t alloc, kind, count, slot; std::string tag;
        if (!u32(alloc, "alloc") || !str(tag, "mesh tag", 100000) || tag != "PolyList 09") { if (err.empty()) err = "mesh tag " + tag; return false; }
        if (!u32(kind, "kind") || !u32(count, "count") || !u32(slot, "slot")) return false;
        if (kind != 5 && kind != 6 && kind != 7) { err = std::format("mesh kind {} at 0x{:x}", kind, p); return false; }
        const std::size_t stride = kind == 5 ? 160 : kind == 6 ? 116 : 212;
        if (count > (b.size() - p) / stride) { err = "mesh record count"; return false; }
        p += std::size_t(count) * stride; nodes[me].meshRecords += count;
      }
    } else if (type == 2) {
      nodes[me].t2Begin = p;
      if (!f64(nodes[me].A, "A") || !f64(nodes[me].B, "B") || !f64(nodes[me].H, "H")) return false;
      if (!u8(nodes[me].visibility, "vis")) return false;
      nodes[me].branchByteOffset = p;
      if (!u8(nodes[me].branch, "branch")) return false;
      if (nodes[me].branch == 0) {
        if (!str(nodes[me].branch0Name, "branch0 name", 260)) return false;
        for (double& x : nodes[me].branch0Uv) if (!f64(x, "uv")) return false;
      } else {
        nodes[me].fieldsOffset = p;
        // 0x00468BF2: four u32 (stored at sub+0x1B0, +0x1C8, +0x1CC, +0x1B8), then a
        // length-prefixed string with capacity 0xFF (0x00468C51 push 0xff).
        if (!u32(nodes[me].f1, "f1") || !u32(nodes[me].f2, "f2") || !u32(nodes[me].f3, "f3") || !u32(nodes[me].f4, "f4")) return false;
        nodes[me].nameOffset1 = p;
        if (!str(nodes[me].spriteName, "sprite name", 0xFF)) return false;
      }
      nodes[me].trailingOffset = p;
      if (!u32(nodes[me].trailing, "trailing")) return false;
    } else if (type != 0) {
      err = std::format("unsupported node type {} at 0x{:x}", type, nodes[me].begin); return false;
    }
    nodes[me].commonOffset = p;
    for (double& x : nodes[me].matrix) if (!f64(x, "matrix")) return false;
    if (!str(nodes[me].name, "node name", 100000)) return false;
    for (double& x : nodes[me].head) if (!f64(x, "head")) return false;
    if (!u32(nodes[me].childCount, "child count") || nodes[me].childCount > 5000) { if (err.empty()) err = "child count"; return false; }
    const auto count = nodes[me].childCount;
    for (std::uint32_t i = 0; i < count; ++i) if (!node(me, depth + 1)) return false;
    nodes[me].end = p;
    return true;
  }
};

// ---- PE image reader for constants (bytes only) -----------------------------
struct Pe {
  std::vector<std::uint8_t> data;
  struct Sec { std::uint32_t va, vsize, raw, rawsize; std::string name; };
  std::vector<Sec> secs; std::uint32_t base = 0;
  bool parse() {
    if (data.size() < 0x200) return false;
    std::uint32_t lfa; std::memcpy(&lfa, &data[0x3c], 4);
    if (lfa + 0x18 > data.size()) return false;
    std::uint16_t ns, opt; std::memcpy(&ns, &data[lfa + 6], 2); std::memcpy(&opt, &data[lfa + 20], 2);
    std::memcpy(&base, &data[lfa + 24 + 28], 4);
    std::size_t s = lfa + 24 + opt;
    for (int i = 0; i < ns; ++i, s += 40) {
      Sec x; x.name.assign(reinterpret_cast<const char*>(&data[s]), strnlen(reinterpret_cast<const char*>(&data[s]), 8));
      std::memcpy(&x.vsize, &data[s + 8], 4); std::memcpy(&x.va, &data[s + 12], 4);
      std::memcpy(&x.rawsize, &data[s + 16], 4); std::memcpy(&x.raw, &data[s + 20], 4);
      secs.push_back(x);
    }
    return true;
  }
  // returns file offset or -1 when the VA has no raw bytes (e.g. .bss tail)
  long off(std::uint32_t va) const {
    const std::uint32_t rva = va - base;
    for (const auto& s : secs) if (rva >= s.va && rva < s.va + s.vsize) {
      if (rva - s.va >= s.rawsize) return -1;
      return long(s.raw + (rva - s.va));
    }
    return -2;
  }
  std::string secOf(std::uint32_t va) const {
    const std::uint32_t rva = va - base;
    for (const auto& s : secs) if (rva >= s.va && rva < s.va + s.vsize) return s.name;
    return "?";
  }
  bool dbl(std::uint32_t va, double& v) const { auto o = off(va); if (o < 0 || std::size_t(o) + 8 > data.size()) return false; std::memcpy(&v, &data[o], 8); return true; }
  bool dw(std::uint32_t va, std::uint32_t& v) const { auto o = off(va); if (o < 0 || std::size_t(o) + 4 > data.size()) return false; std::memcpy(&v, &data[o], 4); return true; }
  std::string cstr(std::uint32_t va) const { auto o = off(va); if (o < 0) return "<no file bytes>"; std::string s; for (std::size_t i = o; i < data.size() && data[i] && s.size() < 80; ++i) s += char(data[i]); return s; }
};

std::string idOf(const std::string& name) {
  auto s = name.find_last_of("\\/"); auto base = s == std::string::npos ? name : name.substr(s + 1);
  auto dot = base.rfind('.'); return dot == std::string::npos ? base : base.substr(0, dot);
}
std::uint32_t textureFlags(const std::string& sheet) {  // texture_name_flags, decomp-lemmings/src/texture_creation.cpp:21
  const auto slash = sheet.find_last_of('\\');
  const char c = sheet.empty() ? '\0' : sheet[slash == std::string::npos ? 0 : slash + 1];
  switch (c) { case '_': return 1; case '$': return 2; case '~': return 4; case '^': return 8; case '+': return 16; default: return 0; }
}
int fails = 0;
void check(bool ok, const std::string& what) { if (!ok) { ++fails; std::cerr << "FAIL: " << what << '\n'; } }
}  // namespace

int main() try {
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cout << "SKIP: WEBOLUTION_GAME_DIR unset\n"; return 0; }
  auto lib = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!lib) { std::cerr << "FAIL open archives: " << lib.error() << '\n'; return 1; }
  const fs::path outDir = SCENE_BRANCH_DIR; fs::create_directories(outDir);
  std::ostringstream out;

  // ---- level identity --------------------------------------------------------
  std::string levelName;
  for (const auto& n : levelFileNames(*lib)) if (idOf(n) == "MM_DIG_000") levelName = n;
  check(!levelName.empty(), "MM_DIG_000 level entry present");
  if (levelName.empty()) return 1;
  auto bytes = lib->read(levelName);
  if (!bytes) { std::cerr << "FAIL read level: " << bytes.error() << '\n'; return 1; }
  const bool inPatch = lib->patch() && lib->patch()->contains(levelName);
  out << "LEVEL entry=" << levelName << " source=" << (inPatch ? "patch.box" : "lemmings.box")
      << " size=" << bytes->size() << " sha256="
      << sha256Hex(reinterpret_cast<const std::uint8_t*>(bytes->data()), bytes->size()) << '\n';
  const auto level = lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes->data()), bytes->size()});
  check(level.objects.second_object_source_extent.has_value(), "Secondary extent present");
  if (!level.objects.second_object_source_extent) return 1;
  const auto ext = *level.objects.second_object_source_extent;
  out << std::format("SECONDARY typedRoot=0x{:x} treeEnd=0x{:x} markerMatches={}\n", ext.typed_root_begin, ext.tree_end, level.objects.second_object_marker_matches);

  // ---- traverse --------------------------------------------------------------
  Tree t; t.b = *bytes; t.p = ext.typed_root_begin;
  const bool ok = t.node(SIZE_MAX, 0);
  out << std::format("TRAVERSAL ok={} end=0x{:x} nodes={} error=\"{}\"\n", ok, t.p, t.nodes.size(), t.err);
  check(ok, "measurement traversal parses the whole Secondary tree: " + t.err);
  check(ok && t.p == ext.tree_end, "traversal end equals retained tree end");
  std::size_t nT0 = 0, nT1 = 0, nT2b0 = 0, nT2b1 = 0, nested = 0, hidden = 0;
  for (const auto& n : t.nodes) {
    nT0 += n.type == 0; nT1 += n.type == 1; nT2b0 += n.type == 2 && n.branch == 0; nT2b1 += n.type == 2 && n.branch != 0;
    if (n.type == 2 && n.branch != 0) hidden += n.visibility == 0;
    if (n.parent != SIZE_MAX && t.nodes[n.parent].parent != SIZE_MAX) ++nested;
  }
  std::size_t b1WithChildren = 0, t1WithChildren = 0, rootChildren = 0;
  for (const auto& n : t.nodes) {
    if (n.type == 2 && n.branch != 0 && n.childCount) ++b1WithChildren;
    if (n.type == 1 && n.childCount) ++t1WithChildren;
    if (n.parent == 0) ++rootChildren;
  }
  out << std::format("CENSUS type0={} type1={} type2branch0={} type2branch1={} hiddenBranch1={} depth>=2Nodes={} rootDirectChildren={} branch1WithChildren={} type1WithChildren={}\n",
                     nT0, nT1, nT2b0, nT2b1, hidden, nested, rootChildren, b1WithChildren, t1WithChildren);
  check(nT2b1 > 0, "at least one branch-1 record measured");

  // ---- records ---------------------------------------------------------------
  out << "RECORDS (file order; offsets are absolute file offsets into the level)\n";
  std::set<std::string> names;
  for (std::size_t i = 0; i < t.nodes.size(); ++i) {
    const auto& n = t.nodes[i];
    out << std::format("NODE[{}] type={} depth={} parent={} begin=0x{:x} end=0x{:x} name=\"{}\" children={} matrixTranslation=({:.6g},{:.6g},{:.6g}) head=({:.6g},{:.6g})",
                       i, n.type, n.depth, n.parent == SIZE_MAX ? -1 : long(n.parent), n.begin, n.end, n.name, n.childCount,
                       n.matrix[9], n.matrix[10], n.matrix[11], n.head[0], n.head[1]);
    if (n.type == 1) out << std::format(" positions={} normals={} materials={} meshBlocks={} meshRecords={}", n.positions, n.normals, n.materials, n.meshBlocks, n.meshRecords);
    if (n.type == 2) {
      out << std::format(" T2[A={:.9g} B={:.9g} H={:.9g} vis={} branch={} branchByte@0x{:x} trailing={}@0x{:x}", n.A, n.B, n.H, n.visibility, n.branch, n.branchByteOffset, n.trailing, n.trailingOffset);
      if (n.branch == 0) out << std::format(" name=\"{}\" uv=({:.6g},{:.6g},{:.6g},{:.6g})", n.branch0Name, n.branch0Uv[0], n.branch0Uv[1], n.branch0Uv[2], n.branch0Uv[3]);
      else { out << std::format(" f1={} f2={} f3={} f4={} fields@0x{:x} sprite=\"{}\"@0x{:x}", n.f1, n.f2, n.f3, n.f4, n.fieldsOffset, n.spriteName, n.nameOffset1); names.insert(n.spriteName); }
      out << ']';
    }
    out << '\n';
  }

  // ---- sprite resolution -----------------------------------------------------
  out << "RESOLUTION (registry name -> SPRITES\\<name>.SPR via fmt 0x4C33C4 'Sprites\\%s.spr', 0x484EB0)\n";
  std::map<std::string, SpriteSheet> sheets; std::map<std::string, std::string> srcOf;
  std::size_t unresolved = 0, missingTex = 0;
  for (const auto& nm : names) {
    const std::string path = "SPRITES\\" + nm + ".SPR";
    if (!lib->contains(path)) { out << std::format("SPRITE \"{}\" -> {} MISSING in effective archive\n", nm, path); ++unresolved; continue; }
    const bool p = lib->patch() && lib->patch()->contains(path);
    auto raw = lib->read(path);
    auto sh = loadSpriteSheet(*lib, path);
    if (!raw || !sh) { out << std::format("SPRITE \"{}\" -> {} PARSE-FAIL {}\n", nm, path, !raw ? raw.error() : sh.error()); ++unresolved; continue; }
    out << std::format("SPRITE \"{}\" -> {} source={} size={} sha256={} clips={} palette={}\n", nm, path, p ? "patch.box" : "lemmings.box", raw->size(),
                       sha256Hex(reinterpret_cast<const std::uint8_t*>(raw->data()), raw->size()), sh->sprites.size(), sh->palette.size());
    for (std::size_t c = 0; c < sh->sprites.size(); ++c) {
      const auto& s = sh->sprites[c];
      out << std::format("  CLIP[{}] name=\"{}\" rate={:.9g} temporalEntries={}", c, s.name, s.rate, s.animations.size());
      std::map<std::size_t, std::size_t> dirHist;
      for (const auto& a : s.animations) ++dirHist[a.frames.size()];
      out << " directionalCountHistogram={";
      for (const auto& [k, v] : dirHist) out << std::format("{}x{} ", k, v);
      out << "}\n";
    }
    sheets.emplace(nm, std::move(*sh));
  }
  out << "RECORD-TO-FRAME (one line per branch-1 node)\n";
  std::map<std::string, std::size_t> usedTextures;
  std::size_t badClip = 0, badTemporal = 0, hasDir = 0, multiDir = 0, count1 = 0;
  std::map<std::size_t, std::size_t> dirCountHist, tempHist;
  for (std::size_t i = 0; i < t.nodes.size(); ++i) {
    const auto& n = t.nodes[i];
    if (n.type != 2 || n.branch == 0) continue;
    out << std::format("REC node[{}]@0x{:x} sprite=\"{}\" mirror(f1)={} clip(f2)={} temporal(f3,serialized)={} dir(f4,serialized)={} vis={} A={:.6g} B={:.6g} H={:.6g} blendSelector={}",
                       i, n.begin, n.spriteName, n.f1, n.f2, n.f3, n.f4, n.visibility, n.A, n.B, n.H, n.trailing);
    auto it = sheets.find(n.spriteName);
    if (it == sheets.end()) { out << " UNRESOLVED\n"; continue; }
    const auto& sh = it->second;
    if (n.f2 >= sh.sprites.size()) { out << std::format(" clip-index-OUT-OF-RANGE(clips={})\n", sh.sprites.size()); ++badClip; continue; }
    const auto& clip = sh.sprites[n.f2];
    const auto cnt = clip.animations.size();
    tempHist[cnt]++;
    // 0x4B03B0 is 32767.0 (measured below), so r = rand()/32767.0 is in [0,1] and
    // trunc((count-1)*r) is in [0,count-1]; count-1 occurs only when rand() == 32767.
    out << std::format(" clipName=\"{}\" rate={:.9g} temporalEntries={} firstDrawRange=[0,{}]", clip.name, clip.rate, cnt, cnt ? cnt - 1 : 0);
    if (n.f3 >= cnt) ++badTemporal;
    const auto& a = clip.animations[std::min<std::size_t>(n.f3, cnt ? cnt - 1 : 0)];
    if (cnt == 0) { out << " NO-TEMPORAL-ENTRIES\n"; continue; }
    out << std::format(" entry[f3].directionalImages={}", a.frames.size());
    dirCountHist[a.frames.size()]++;
    hasDir += !a.frames.empty(); multiDir += a.frames.size() > 1; count1 += cnt == 1;
    if (!a.frames.empty()) {
      const auto& fr = a.frames[0];
      const std::uint32_t fl = textureFlags(fr.sheet);
      const auto tex = sheetAssetName(fr.sheet);
      const bool texOk = lib->contains(tex);
      if (!texOk) ++missingTex;
      usedTextures[fr.sheet + (texOk ? "" : " (MISSING)")]++;
      out << std::format(" image0{{sheet=\"{}\" texFlags={} texture={} exists={} rectLRTB=({},{},{},{}) uv=({:.6g},{:.6g},{:.6g},{:.6g}) offs/size=({},{},{},{})}}",
                         fr.sheet, fl, tex, texOk, fr.left, fr.right, fr.top, fr.bottom, fr.uLeft, fr.uRight, fr.uTop, fr.uBottom, fr.unknown0, fr.unknown1, fr.width, fr.height);
    }
    out << '\n';
  }
  out << std::format("SUMMARY branch1Records={} uniqueSprites={} unresolvedSprites={} clipOutOfRange={} serializedTemporalOutOfRange={} entriesWithDirectionalImages={} entriesWithMoreThanOneDirectionalImage={} clipsWithSingleTemporalEntry={} missingTextures={}\n",
                     nT2b1, names.size(), unresolved, badClip, badTemporal, hasDir, multiDir, count1, missingTex);
  out << "TEMPORAL-ENTRY-COUNT HISTOGRAM:"; for (auto [k, v] : tempHist) out << std::format(" {}x{}", k, v); out << '\n';
  out << "DIRECTIONAL-COUNT HISTOGRAM:"; for (auto [k, v] : dirCountHist) out << std::format(" {}x{}", k, v); out << '\n';
  out << "SHEET TEXTURES (image0 of each record):\n"; for (auto& [k, v] : usedTextures) out << std::format("  {} x{} flags={}\n", k, v, textureFlags(k));
  check(unresolved == 0, "every branch-1 sprite name resolves to an installed SPRITES\\*.SPR");
  check(badClip == 0, "every branch-1 clip index is inside its sheet");

  // ---- reference executable constants (bytes only; never executed) ------------
  {
    Pe pe; const fs::path exe = fs::path(gameDir) / "Lemmings Revolution.exe";
    std::ifstream f(exe, std::ios::binary);
    if (!f) { out << "EXE not readable at " << exe.string() << '\n'; ++fails; }
    else {
      pe.data.assign(std::istreambuf_iterator<char>(f), {});
      out << "EXE sha256=" << sha256Hex(pe.data.data(), pe.data.size()) << " size=" << pe.data.size() << '\n';
      check(sha256Hex(pe.data.data(), pe.data.size()) == "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f", "reference executable identity");
      check(pe.parse(), "PE parse");
      for (const auto& s : pe.secs) out << std::format("  SECTION {} va=0x{:x} vsize=0x{:x} raw=0x{:x} rawsize=0x{:x}\n", s.name, pe.base + s.va, s.vsize, s.raw, s.rawsize);
      const std::uint32_t dv[] = {0x4B02C8, 0x4B02D0, 0x4B0338, 0x4B03B0, 0x4B03F8, 0x4B0D68, 0x4B1928, 0x4B1940, 0x4B1948, 0x4B1950,
                                    // frame-clock constants: ms->seconds divisor, paused/non-play fixed step, step cap, burst cap, seed override
                                    0x4B1758, 0x4B17B8, 0x4C08E8, 0x4B1800, 0x4E33D0};
      for (auto va : dv) { double v = 0; const bool g = pe.dbl(va, v); out << std::format("  DOUBLE @0x{:x} [{}] = {}\n", va, pe.secOf(va), g ? std::format("{:.17g} (bits 0x{:016x})", v, std::bit_cast<std::uint64_t>(v)) : "<no file bytes>"); }
      const std::uint32_t wv[] = {0x4C4570, 0x4C24E0, 0x4C227C, 0x4C24E4, 0x58D53C, 0x58CB38, 0x5CD5F4,
                                          0x4B15F8, 0x4E33C8, 0x4E33D0, 0x4E33D4, 0x4C36A8};
      for (auto va : wv) { std::uint32_t v = 0; const bool g = pe.dw(va, v); out << std::format("  DWORD  @0x{:x} [{}] = {}\n", va, pe.secOf(va), g ? std::format("0x{:08x} ({})", v, v) : "<no file bytes (zero-initialised)>"); }
      out << "  STRING @0x4C33C4 = \"" << pe.cstr(0x4C33C4) << "\"\n  STRING @0x4C33D4 = \"" << pe.cstr(0x4C33D4) << "\"\n";
      // Calls to srand-style writers: the only static writer of 0x4C4570 is 0x49C844 (disassembly grep), so the
      // initial image value is the effective initial PRNG state unless that writer runs.
    }
  }
  // Worked example only (NOT a claim about the original's runtime state): the CRT LCG at 0x0049C84A
  // (state' = state*0x343FD + 0x269EC3; value = (int32(state') >> 16) & 0x7FFF) started from the
  // executable's initial image state 1 with no earlier rand()/srand() call, mapped through the
  // first-draw formula f3 = trunc((count-1) * value/32767.0) for the 16-entry clip, in file order.
  {
    std::uint32_t state = 1; out << "WORKED-EXAMPLE initial-state-1 first-draw f3 for 19 records (count=16):";
    for (std::size_t i = 0; i < nT2b1; ++i) {
      state = state * 0x343FDu + 0x269EC3u;
      const std::uint32_t v = (std::uint32_t(std::int32_t(state) >> 16)) & 0x7FFFu;
      out << ' ' << static_cast<int>(15.0 * (static_cast<double>(v) / 32767.0));
    }
    out << '\n';
  }
  out << "FAILS=" << fails << '\n';
  std::ofstream(outDir / "records.txt") << out.str();
  std::cout << out.str();
  if (fails) return 1;
  std::cout << "PASS scenebranch_measure\n";
  return 0;
} catch (const std::exception& e) { std::cerr << "FAIL scenebranch_measure: " << e.what() << '\n'; return 1; }
