#include "app/SceneMeshSource.h"

#include <format>
#include <unordered_map>

namespace webo {
namespace {
using Matrix = std::array<double, 12>;
using Attributes = std::array<double, 6>;

std::array<double, 3> transform(const std::array<double, 3>& p, const Matrix& m) {
  std::array<double, 3> out{};
  for (std::size_t j = 0; j < 3; ++j) {
    for (std::size_t i = 0; i < 3; ++i) out[j] += p[i] * m[3 * i + j];
    out[j] += m[9 + j];
  }
  return out;
}

// Result applies first, then second, under row-vector convention.
Matrix compose(const Matrix& first, const Matrix& second) {
  Matrix out{};
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t j = 0; j < 3; ++j)
      for (std::size_t k = 0; k < 3; ++k)
        out[3 * i + j] += first[3 * i + k] * second[3 * k + j];
    out[9 + i] = second[9 + i];
    for (std::size_t k = 0; k < 3; ++k) out[9 + i] += first[9 + k] * second[3 * k + i];
  }
  return out;
}

Attributes attrs(const SceneCorner& c) { return {c.u, c.v, c.r, c.g, c.b, c.a}; }

std::expected<void, std::string> visit(
    const SceneNode& node, const Matrix& parent, const Matrix& view,
    std::vector<SceneMeshRecordSource>& result) {
  if (node.type > 2) return std::unexpected(std::format("unsupported hand-built scene node type {} at 0x{:x}", node.type, node.sourceOffset));
  const Matrix world = compose(node.matrix, parent);
  const Matrix toView = compose(world, view);
  if (node.type == 1) {
    for (const auto& block : node.meshes) {
      if (block.kind != 5 && block.kind != 6 && block.kind != 7)
        return std::unexpected(std::format("unsupported hand-built mesh kind {} at 0x{:x}", block.kind, block.sourceOffset));
      if (block.materialSlot >= node.materials.size())
        return std::unexpected(std::format("material slot {} outside node material table at 0x{:x}", block.materialSlot, block.sourceOffset));
      const auto& material = node.materials[block.materialSlot];
      for (const auto& record : block.records) {
        if (record.normalIndex >= node.normals.size())
          return std::unexpected(std::format("normal index {} outside node normals at 0x{:x}", record.normalIndex, record.sourceOffset));
        std::unordered_map<std::uint32_t, Attributes> winner;
        for (const auto& c : record.corners) {
          if (c.vertexIndex >= node.positions.size())
            return std::unexpected(std::format("vertex index {} outside node positions at 0x{:x}", c.vertexIndex, c.sourceOffset));
          winner[c.vertexIndex] = attrs(c);
        }
        SceneMeshRecordSource out;
        out.nodeSourceOffset = node.sourceOffset;
        out.blockSourceOffset = block.sourceOffset;
        out.recordSourceOffset = record.sourceOffset;
        out.kind = block.kind;
        out.materialSlot = block.materialSlot;
        out.materialName = material.name;
        out.blendSelector = material.blendSelector;
        out.corners.reserve(record.corners.size());
        for (const auto& c : record.corners) {
          SceneMeshCornerSource corner;
          corner.sourceOffset = c.sourceOffset;
          corner.vertexIndex = c.vertexIndex;
          corner.localPosition = node.positions[c.vertexIndex];
          corner.worldPosition = transform(corner.localPosition, world);
          corner.viewPosition = transform(corner.localPosition, toView);
          corner.localNormal = node.normals[record.normalIndex];
          corner.normalIndex = record.normalIndex;
          corner.sourceAttributes = attrs(c);
          corner.attributes = winner.at(c.vertexIndex);
          out.corners.push_back(corner);
        }
        result.push_back(std::move(out));
      }
    }
  }
  for (const auto& child : node.children) {
    auto status = visit(child, world, view, result);
    if (!status) return status;
  }
  return {};
}
}  // namespace

std::expected<std::vector<SceneMeshRecordSource>, std::string> buildSceneMeshSource(
    const SceneNode& root, const std::array<double, 12>& parentWorld,
    const std::array<double, 12>& view) {
  std::vector<SceneMeshRecordSource> result;
  auto status = visit(root, parentWorld, view, result);
  if (!status) return std::unexpected(status.error());
  return result;
}
}  // namespace webo
