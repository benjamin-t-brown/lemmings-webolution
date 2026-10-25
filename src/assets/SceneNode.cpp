#include "SceneNode.h"

#include <cstring>
#include <format>
#include <limits>

namespace webo {
namespace {
constexpr std::uint32_t kMaxCount = 100'000;
constexpr unsigned kMaxDepth = 128;
constexpr std::size_t kMaxNodes = 10'000;
constexpr std::size_t kMinNodeBytes = 4 + 12 * 8 + 4 + 16 + 4;
constexpr std::size_t kMinMaterialBytes = 4 + 4;
constexpr std::size_t kMinMeshBlockBytes = 4 + 4 + 11 + 12;
constexpr std::size_t kMaxVectorBytes = 256 * 1024;
constexpr std::size_t kMaxEstimatedOutputBytes = 32 * 1024 * 1024;

struct Reader {
  std::span<const std::byte> bytes;
  std::size_t p = 0, nodes = 0;
  std::size_t estimatedOutputBytes = 0;
  std::string error;

  bool charge(std::size_t count, std::size_t elementBytes, const char* what) {
    if (elementBytes && count > std::numeric_limits<std::size_t>::max() / elementBytes) {
      error = std::format("{} allocation size overflows", what); return false;
    }
    const std::size_t amount = count * elementBytes;
    if (amount > kMaxVectorBytes) {
      error = std::format("{} output allocation {} exceeds per-vector limit {}", what, amount,
                          kMaxVectorBytes); return false;
    }
    if (amount > kMaxEstimatedOutputBytes - estimatedOutputBytes) {
      error = std::format("scene output estimate exceeds {} bytes", kMaxEstimatedOutputBytes); return false;
    }
    estimatedOutputBytes += amount;
    return true;
  }

  bool require(std::size_t n, const char* what) {
    if (p > bytes.size() || n > bytes.size() - p) {
      error = std::format("truncated {} at 0x{:x} (need {} bytes, have {})", what, p, n,
                          p <= bytes.size() ? bytes.size() - p : 0);
      return false;
    }
    return true;
  }
  bool u32(std::uint32_t& v, const char* what) {
    if (!require(4, what)) return false;
    std::memcpy(&v, bytes.data() + p, 4); p += 4; return true;
  }
  bool u8(std::uint8_t& v, const char* what) {
    if (!require(1, what)) return false;
    v = std::to_integer<std::uint8_t>(bytes[p++]); return true;
  }
  bool f64(double& v, const char* what) {
    if (!require(8, what)) return false;
    std::memcpy(&v, bytes.data() + p, 8); p += 8; return true;
  }
  bool string(std::string& s, const char* what) {
    std::uint32_t n;
    if (!u32(n, "string length")) return false;
    if (n > kMaxCount) { error = std::format("{} length {} exceeds limit", what, n); return false; }
    if (!require(n, what)) return false;
    if (!charge(n, 1, what)) return false;
    s.assign(reinterpret_cast<const char*>(bytes.data() + p), n); p += n; return true;
  }
  bool count(std::uint32_t& n, const char* what) {
    if (!u32(n, what)) return false;
    if (n > kMaxCount) { error = std::format("{} {} exceeds limit at 0x{:x}", what, n, p-4); return false; }
    return true;
  }
  bool vec3s(std::vector<std::array<double, 3>>& out, const char* what) {
    std::uint32_t n;
    if (!count(n, what)) return false;
    if (n > (bytes.size() - p) / 24) { error = std::format("{} count {} exceeds remaining bytes", what, n); return false; }
    if (!charge(n, sizeof(std::array<double, 3>), what)) return false;
    out.resize(n);
    for (auto& v : out) for (double& x : v) if (!f64(x, what)) return false;
    return true;
  }
  bool node(SceneNode& out, unsigned depth) {
    if (depth > kMaxDepth) { error = "scene-node recursion limit exceeded"; return false; }
    if (++nodes > kMaxNodes) { error = "scene-node count limit exceeded"; return false; }
    out.sourceOffset = p;
    if (!u32(out.type, "node type")) return false;
    if (out.type == 1) {
      out.positionsOffset = p + 4;
      if (!vec3s(out.positions, "position count")) return false;
      out.normalsOffset = p + 4;
      if (!vec3s(out.normals, "normal count")) return false;
      std::uint32_t n;
      if (!count(n, "material count")) return false;
      if (n > (bytes.size() - p) / kMinMaterialBytes) {
        error = std::format("material count {} cannot fit in remaining input", n); return false;
      }
      if (!charge(n, sizeof(SceneMaterial), "material table")) return false;
      for (std::uint32_t i = 0; i < n; ++i) {
        SceneMaterial m; m.sourceOffset = p;
        if (!string(m.name, "material name") || !u32(m.blendSelector, "material blend selector")) return false;
        out.materials.push_back(std::move(m));
      }
      std::uint32_t nb;
      if (!count(nb, "mesh block count")) return false;
      if (nb > (bytes.size() - p) / kMinMeshBlockBytes) {
        error = std::format("mesh block count {} cannot fit in remaining input", nb); return false;
      }
      if (!charge(nb, sizeof(SceneMeshBlock), "mesh block table")) return false;
      for (std::uint32_t bi = 0; bi < nb; ++bi) {
        SceneMeshBlock b; b.sourceOffset = p;
        std::string tag; std::uint32_t count, slot;
        if (!u32(b.allocationWord, "mesh allocation word") || !string(tag, "mesh tag") ||
            tag != "PolyList 09") {
          if (error.empty()) error = std::format("unsupported mesh tag '{}' at 0x{:x}", tag, b.sourceOffset);
          return false;
        }
        if (!u32(b.kind, "mesh kind")) return false;
        // Read the remaining two header words separately; keeping the native
        // 12-byte block header distinct from legacy PolyList's 16-byte header.
        if (!u32(count, "mesh record count") || !u32(slot, "material slot")) return false;
        b.materialSlot = slot;
        if (b.kind != 5 && b.kind != 6 && b.kind != 7) {
          error = std::format("unsupported scene mesh kind {} at 0x{:x}", b.kind, b.sourceOffset); return false;
        }
        if (slot >= out.materials.size()) { error = std::format("material slot {} outside node table at 0x{:x}", slot, b.sourceOffset); return false; }
        const std::size_t stride = b.kind == 5 ? 160 : b.kind == 6 ? 116 : 212;
        if (count > (bytes.size() - p) / stride) { error = std::format("mesh record count {} exceeds remaining bytes", count); return false; }
        if (!charge(count, sizeof(SceneRecord), "mesh records")) return false;
        for (std::uint32_t ri = 0; ri < count; ++ri) {
          SceneRecord r; r.sourceOffset = p;
          if (!u32(r.normalIndex, "record normal index")) return false;
          if (r.normalIndex >= out.normals.size()) { error = std::format("normal index {} outside node normals at 0x{:x}", r.normalIndex, r.sourceOffset); return false; }
          const unsigned corners = b.kind == 5 ? 3 : 4;
          if (!charge(corners, sizeof(SceneCorner), "record corners")) return false;
          r.corners.resize(corners);
          double shared[4]{};
          if (b.kind == 6) for (double& x : shared) if (!f64(x, "kind-6 shared colour/alpha")) return false;
          for (auto& c : r.corners) {
            c.sourceOffset = p;
            if (!u32(c.vertexIndex, "corner vertex index")) return false;
            if (c.vertexIndex >= out.positions.size()) { error = std::format("vertex index {} outside node positions at 0x{:x}", c.vertexIndex, r.sourceOffset); return false; }
            if (!f64(c.u, "corner U") || !f64(c.v, "corner V")) return false;
            if (b.kind == 6) { c.r=shared[0]; c.g=shared[1]; c.b=shared[2]; c.a=shared[3]; }
            else if (!f64(c.r, "corner red") || !f64(c.g, "corner green") || !f64(c.b, "corner blue") || !f64(c.a, "corner alpha")) return false;
          }
          b.records.push_back(std::move(r));
        }
        out.meshes.push_back(std::move(b));
      }
    } else if (out.type == 2) {
      out.type2.sourceOffset = p;
      for (double& x : out.type2.leading) if (!f64(x, "type-2 leading double")) return false;
      if (!u8(out.type2.visibility, "type-2 visibility") || !u8(out.type2.branch, "type-2 branch")) return false;
      if (out.type2.branch == 0) {
        if (!string(out.type2.branchName, "type-2 branch name")) return false;
        for (double& x : out.type2.branchDoubles) if (!f64(x, "type-2 branch double")) return false;
      } else {
        error = std::format("unsupported nonzero type-2 branch {} at 0x{:x}", out.type2.branch, p-1); return false;
      }
      if (!u32(out.type2.trailingWord, "type-2 trailing word")) return false;
    } else if (out.type != 0) {
      error = std::format("unsupported scene node type {} at 0x{:x}", out.type, out.sourceOffset); return false;
    }
    out.commonHeaderOffset = p;
    for (double& x : out.matrix) if (!f64(x, "node matrix")) return false;
    out.nameOffset = p;
    if (!string(out.name, "node name")) return false;
    out.headDoublesOffset = p;
    for (double& x : out.headDoubles) if (!f64(x, "node head double")) return false;
    std::uint32_t childCount;
    if (!count(childCount, "child count")) return false;
    if (childCount > kMaxNodes - nodes) { error = "scene child count exceeds node limit"; return false; }
    if (childCount > (bytes.size() - p) / kMinNodeBytes) {
      error = std::format("child count {} cannot fit in remaining input", childCount); return false;
    }
    if (!charge(childCount, sizeof(SceneNode), "child table")) return false;
    for (std::uint32_t i = 0; i < childCount; ++i) {
      SceneNode child;
      if (!node(child, depth + 1)) return false;
      out.children.push_back(std::move(child));
    }
    out.endOffset = p;
    return true;
  }
};
}  // namespace

std::expected<SceneNodeRead, std::string> readSceneNode(std::span<const std::byte> bytes,
                                                        std::size_t startOffset) {
  if (startOffset > bytes.size()) return std::unexpected(std::format("node start 0x{:x} past input size 0x{:x}", startOffset, bytes.size()));
  Reader r{bytes, startOffset, 0, 0, {}};
  SceneNodeRead result;
  if (!r.node(result.root, 0)) return std::unexpected(std::move(r.error));
  result.endOffset = r.p;
  return result;
}
}  // namespace webo
