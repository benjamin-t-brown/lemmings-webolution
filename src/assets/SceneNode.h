#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

struct SceneCorner {
  std::size_t sourceOffset = 0;
  std::uint32_t vertexIndex = 0;
  double u = 0, v = 0;
  double r = 0, g = 0, b = 0, a = 0;
};

struct SceneRecord {
  std::size_t sourceOffset = 0;
  std::uint32_t normalIndex = 0;
  std::vector<SceneCorner> corners;
};

struct SceneMeshBlock {
  std::size_t sourceOffset = 0;
  std::uint32_t allocationWord = 0;
  std::uint32_t kind = 0, materialSlot = 0;
  std::vector<SceneRecord> records;
};

struct SceneMaterial {
  std::size_t sourceOffset = 0;
  std::string name;
  std::uint32_t blendSelector = 0;
};

struct SceneType2Payload {
  std::size_t sourceOffset = 0;
  std::array<double, 3> leading{};
  std::uint8_t visibility = 0, branch = 0;
  std::string branchName;
  std::array<double, 4> branchDoubles{};
  std::array<std::uint32_t, 4> branchWords{};
  std::uint32_t trailingWord = 0;
};

struct SceneNode {
  std::size_t sourceOffset = 0, positionsOffset = 0, normalsOffset = 0;
  std::size_t commonHeaderOffset = 0, nameOffset = 0, headDoublesOffset = 0, endOffset = 0;
  std::uint32_t type = 0;
  std::vector<std::array<double, 3>> positions, normals;
  std::vector<SceneMaterial> materials;
  std::vector<SceneMeshBlock> meshes;
  SceneType2Payload type2;
  std::array<double, 12> matrix{};
  std::string name;
  std::array<double, 2> headDoubles{};
  std::vector<SceneNode> children;
};

struct SceneNodeRead {
  SceneNode root;
  std::size_t endOffset = 0;
};

// Reads one node at exactly startOffset. No tag scanning or trailer inference.
std::expected<SceneNodeRead, std::string> readSceneNode(
    std::span<const std::byte> bytes, std::size_t startOffset);

}  // namespace webo
