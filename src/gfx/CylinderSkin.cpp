#include "CylinderSkin.h"

#include "Renderer.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"

#include <format>

namespace webo::gfx {

std::expected<void, std::string> CylinderSkin::load(const AssetLibrary& lib, Renderer& renderer,
                                                    const CylinderMesh& mesh) {
  tiles_.clear();
  missing_.clear();
  columns_ = static_cast<int>(CylinderMesh::kTileColumns);
  rows_ = static_cast<int>(CylinderMesh::kTileRows);

  for (const auto& name : mesh.tileTextures) {
    if (name.empty())
      continue;
    const TilePosition pos = parseTilePosition(name);
    if (!pos.valid) {
      missing_.push_back(std::format("{}: cannot read a position from the name", name));
      continue;
    }
    if (!pos.outer)
      continue;  // the camera is outside the tube
    if (pos.column >= columns_ || pos.row >= rows_) {
      missing_.push_back(std::format("{}: position {},{} is outside the {}x{} grid", name,
                                     pos.column, pos.row, columns_, rows_));
      continue;
    }

    auto image = loadTexture(lib, levelTextureAssetName(name));
    if (!image) {
      missing_.push_back(std::format("{}: {}", name, image.error()));
      continue;
    }
    const unsigned texture = renderer.uploadImage(*image);
    if (!texture) {
      missing_.push_back(std::format("{}: upload failed", name));
      continue;
    }
    tiles_.push_back(Tile{texture, pos.column, pos.row});
  }

  if (tiles_.empty())
    return std::unexpected("no outer skin tiles could be loaded");
  return {};
}

void CylinderSkin::draw(Renderer& renderer, float worldWidth, float worldHeight,
                        int segments) const {
  if (tiles_.empty() || columns_ <= 0 || rows_ <= 0)
    return;
  const float tileWidth = worldWidth / static_cast<float>(columns_);
  const float tileHeight = worldHeight / static_cast<float>(rows_);

  for (const Tile& tile : tiles_) {
    const float x = tileWidth * static_cast<float>(tile.column);
    const float y = tileHeight * static_cast<float>(tile.row);
    renderer.drawCylinderPatch(tile.texture, x, y, tileWidth, tileHeight, segments);
  }
}

}  // namespace webo::gfx
