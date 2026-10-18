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

  const auto loadBlock = [&](const std::vector<std::string>& names) {
    for (const auto& name : names) {
      if (name.empty())
        continue;
      const TilePosition pos = parseTilePosition(name);
      if (!pos.valid) {
        missing_.push_back(std::format("{}: cannot read a position from the name", name));
        continue;
      }
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
      tiles_.push_back(Tile{texture, pos.column, pos.row, pos.outer});
    }
  };

  loadBlock(mesh.tileTextures);
  loadBlock(mesh.innerTileTextures);

  if (tiles_.empty())
    return std::unexpected("no skin tiles could be loaded");
  return {};
}

std::size_t CylinderSkin::outerCount() const {
  std::size_t n = 0;
  for (const Tile& t : tiles_)
    n += t.outer ? 1 : 0;
  return n;
}

std::size_t CylinderSkin::innerCount() const { return tiles_.size() - outerCount(); }

void CylinderSkin::draw(Renderer& renderer, float worldWidth, float worldHeight,
                        int segments) const {
  if (tiles_.empty() || columns_ <= 0 || rows_ <= 0)
    return;
  const float tileWidth = worldWidth / static_cast<float>(columns_);
  const float tileHeight = worldHeight / static_cast<float>(rows_);

  // Inner surface first, so the outer one covers it wherever it is opaque.
  // Both are drawn at the same radius: the tube's 10-unit wall is thinner than
  // a texel at this scale, and separating them would only produce z-fighting.
  // What makes the inner skin visible at all is the outer skin's own
  // transparency, which is how the original shows the far side through gaps.
  for (int pass = 0; pass < 2; ++pass) {
    const bool wantOuter = pass == 1;
    for (const Tile& tile : tiles_) {
      if (tile.outer != wantOuter)
        continue;
      const float x = tileWidth * static_cast<float>(tile.column);
      const float y = tileHeight * static_cast<float>(tile.row);
      renderer.drawCylinderPatch(tile.texture, x, y, tileWidth, tileHeight, segments);
    }
  }
}

}  // namespace webo::gfx
