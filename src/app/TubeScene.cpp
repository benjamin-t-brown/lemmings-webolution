#include "TubeScene.h"

#include "assets/Bitmap.h"
#include "assets/LevelIndex.h"
#include "assets/PolyList.h"
#include "assets/SurfaceMesh.h"
#include "gfx/TilePosition.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>

namespace webo::app {
namespace {

Texture toTexture(const Image& image) {
  Texture t;
  t.width = image.width;
  t.height = image.height;
  t.rgb.resize(static_cast<std::size_t>(image.width) * image.height * 3);
  for (std::size_t i = 0, n = static_cast<std::size_t>(image.width) * image.height; i < n; ++i) {
    t.rgb[i * 3] = image.rgba[i * 4];
    t.rgb[i * 3 + 1] = image.rgba[i * 4 + 1];
    t.rgb[i * 3 + 2] = image.rgba[i * 4 + 2];
  }
  return t;
}

bool project(const CameraBasis& c, const Vec3& v, std::array<double, 4>& out) {
  const auto& m = c.worldToView;
  const double px = m[0] * v.x + m[3] * v.y + m[6] * v.z + m[9];
  const double py = m[1] * v.x + m[4] * v.y + m[7] * v.z + m[10];
  const double pz = m[2] * v.x + m[5] * v.y + m[8] * v.z + m[11];
  if (pz <= 0.0 || !std::isfinite(pz))
    return false;
  const double inv = 1.0 / pz;
  out = {c.cx + c.focalX * px * inv, c.cy - c.focalY * py * inv, pz, inv};
  return true;
}

}  // namespace

std::expected<TubeScene, std::string> TubeScene::build(const AssetLibrary& lib,
                                                       const CylinderMesh& mesh,
                                                       std::span<const std::byte> levelBytes,
                                                       std::span<const std::uint32_t> grid0,
                                                       bool highDetailTiles) {
  if (mesh.vertices.empty() || mesh.ringCount < 2 || mesh.angularSteps == 0)
    return std::unexpected("scene: the cylinder mesh has no usable lattice");

  TubeScene scene;
  scene.mesh_ = mesh;
  if (auto grid = findCollisionGrid(levelBytes)) {
    scene.collision_ = std::move(*grid);
    scene.haveCollision_ = true;
    if (scene.collision_.cylinderExtent > 0.0)
      scene.cylinderExtent_ = scene.collision_.cylinderExtent;
  }

  const auto loadBlock = [&](const std::vector<std::string>& names, bool outer,
                             std::vector<Tile>& into, std::size_t& count) {
    for (const auto& name : names) {
      if (name.empty())
        continue;
      const gfx::TilePosition pos = gfx::parseTilePosition(name);
      if (!pos.valid || pos.outer != outer) {
        scene.missing_.push_back(std::format("{}: cannot read a position", name));
        continue;
      }
      auto image = loadTexture(lib, highDetailTiles ? levelTextureAssetNameHi(name)
                                                    : levelTextureAssetName(name));
      if (!image) {
        scene.missing_.push_back(std::format("{}: {}", name, image.error()));
        continue;
      }
      into.push_back(Tile{toTexture(*image), pos.column, pos.row});
      ++count;
    }
  };
  loadBlock(mesh.tileTextures, true, scene.outer_, scene.outerTileCount_);
  loadBlock(mesh.innerTileTextures, false, scene.inner_, scene.innerTileCount_);

  // The per-cell mesh the engine builds at level load. A failure is not a scene
  // failure: it means this level's stored key table does not answer the rule.
  if (scene.haveCollision_ && !grid0.empty()) {
    if (auto built = buildCellMeshes(grid0, {}, scene.collision_, mesh.surface.size())) {
      scene.cellMesh_ = std::move(*built);
    } else {
      scene.missing_.push_back(std::format("cell mesh: {}", built.error()));
    }
  }

  // Optional cut-in geometry from the stored lattice. A failure here is not a
  // scene failure: it means this level has no reconstructible lattice faces.
  if (!grid0.empty()) {
    if (auto built = buildSurfaceMesh(mesh, grid0, {}); built && !built->quads.empty()) {
      for (const auto& q : built->quads) {
        SurfaceQuad quad;
        for (int i = 0; i < 4; ++i)
          quad.v[i] = q.v[i];
        double radius = 0;
        for (int i = 0; i < 4; ++i)
          radius += std::hypot(q.v[i].x, q.v[i].z);
        quad.outerFacing = radius / 4.0 >= mesh.innerRadius() + mesh.thickness * 0.5;
        scene.surface_.push_back(quad);
      }
      scene.surfaceQuads_ = scene.surface_.size();
    }
  }

  // Optional cut-in geometry from the stored PolyList 09 records. These carry
  // their own index into CylinderMesh::surface, a UV and an RGBA colour. They
  // are render attributes, not the runtime per-cell mesh (see the round note).
  if (!levelBytes.empty()) {
    if (auto lists = findPolyLists(levelBytes)) {
      for (const auto& block : lists->blocks) {
        for (const auto& record : block.records) {
          if (record.cornerCount < 3)
            continue;
          Vec3 corners[4];
          bool ok = true;
          for (std::uint32_t c = 0; c < record.cornerCount; ++c) {
            const auto& corner = record.corners[c];
            if (corner.index >= mesh.surface.size()) {
              ok = false;
              break;
            }
            corners[c] = mesh.surface[corner.index];
          }
          if (!ok)
            continue;
          const auto& first = record.corners[0];
          const std::array<double, 3> modulate = {
              std::clamp(first.r / 255.0, 0.0, 1.0),
              std::clamp(first.g / 255.0, 0.0, 1.0),
              std::clamp(first.b / 255.0, 0.0, 1.0)};
          const bool outerFacing =
              std::hypot(corners[0].x, corners[0].z) >= mesh.innerRadius() + mesh.thickness * 0.5;
          const std::array<std::array<int, 3>, 2> tris = {std::array<int, 3>{0, 1, 2},
                                                          std::array<int, 3>{0, 2, 3}};
          for (const auto& tri : tris) {
            if (record.cornerCount == 3 && tri[2] == 3)
              continue;
            PolyFace face;
            face.modulate = modulate;
            face.outerFacing = outerFacing;
            for (int i = 0; i < 3; ++i)
              face.v[i] = corners[tri[i]];
            scene.poly_.push_back(face);
          }
          ++scene.polyListRecords_;
        }
      }
    }
  }

  return scene;
}

const TubeScene::Tile* TubeScene::findTile(bool outer, int column, int row) const {
  const auto& block = outer ? outer_ : inner_;
  for (const auto& t : block)
    if (t.column == column && t.row == row)
      return &t;
  return nullptr;
}

void TubeScene::render(Raster& raster, const CameraBasis& camera,
                       const SceneOptions& options) const {
  raster.setDepthEnabled(options.depthTest);
  const int steps = static_cast<int>(mesh_.angularSteps);
  const int bands = static_cast<int>(mesh_.bandCount());
  const int cols = static_cast<int>(CylinderMesh::kTileColumns);
  const int rows = static_cast<int>(CylinderMesh::kTileRows);
  const int stepsPerCol = std::max(1, steps / cols);
  const int bandsPerRow = std::max(1, bands / rows);

  if (options.flat) {
    // Debug unwrap: one screen-space cell per angular step and ring band, with
    // the outer tile's own UV quadrant. No camera, no depth.
    const double cellW = static_cast<double>(camera.width) / steps;
    const double cellH = static_cast<double>(camera.height) / bands;
    for (int band = 0; band < bands; ++band) {
      const int row = (bands - 1 - band) / bandsPerRow;
      const int withinRow = (bands - 1 - band) % bandsPerRow;
      for (int step = 0; step < steps; ++step) {
        const Tile* tile = findTile(true, step / stepsPerCol, row);
        if (tile == nullptr)
          continue;
        const int withinCol = step % stepsPerCol;
        const double x0 = step * cellW, y0 = (bands - 1 - band) * cellH;
        const double u0 = static_cast<double>(withinCol) / stepsPerCol;
        const double u1 = static_cast<double>(withinCol + 1) / stepsPerCol;
        const double v0 = static_cast<double>(withinRow) / bandsPerRow;
        const double v1 = static_cast<double>(withinRow + 1) / bandsPerRow;
        std::array<std::array<double, 4>, 3> p = {
            {{x0, y0, 0, 1}, {x0, y0 + cellH, 0, 1}, {x0 + cellW, y0 + cellH, 0, 1}}};
        std::array<std::array<double, 2>, 3> uv = {{{u0, v0}, {u0, v1}, {u1, v1}}};
        raster.triangle(p, uv, tile->texture, {1.0, 1.0, 1.0});
        std::array<std::array<double, 4>, 3> p2 = {
            {{x0, y0, 0, 1}, {x0 + cellW, y0 + cellH, 0, 1}, {x0 + cellW, y0, 0, 1}}};
        std::array<std::array<double, 2>, 3> uv2 = {{{u0, v0}, {u1, v1}, {u1, v0}}};
        raster.triangle(p2, uv2, tile->texture, {1.0, 1.0, 1.0});
      }
    }
    return;
  }

  const auto drawQuad = [&](const Vec3 corners[4], const Tile* tile, double context,
                            const std::array<std::array<double, 2>, 4>& uv) {
    if (tile == nullptr)
      return;
    std::array<std::array<double, 4>, 4> projected{};
    for (int i = 0; i < 4; ++i) {
      if (!project(camera, corners[i], projected[i]))
        return;
      projected[i][2] -= context;
    }
    const std::array<double, 3> modulate = {1.0, 1.0, 1.0};
    for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
      std::array<std::array<double, 4>, 3> p{};
      std::array<std::array<double, 2>, 3> t{};
      for (int i = 0; i < 3; ++i) {
        p[i] = projected[ids[i]];
        t[i] = uv[ids[i]];
      }
      raster.triangle(p, t, tile->texture, modulate);
    }
  };

  const auto drawSurface = [&]() {
    if (surface_.empty())
      return;
    for (const auto& quad : surface_) {
      std::array<std::array<double, 4>, 4> projected{};
      bool visible = true;
      for (int i = 0; i < 4 && visible; ++i)
        visible = project(camera, quad.v[i], projected[i]);
      if (!visible)
        continue;
      // Tile chosen from the first corner's angular step and height.
      const double angle = std::atan2(-quad.v[0].x, quad.v[0].z) / (2.0 * std::numbers::pi);
      int step = static_cast<int>(std::floor(angle * steps));
      if (step < 0) step += steps;
      step %= steps;
      int band = static_cast<int>(std::floor(quad.v[0].y / std::max(1e-9, mesh_.ringSpacing())));
      band = std::clamp(band, 0, bands - 1);
      const Tile* tile = findTile(quad.outerFacing, step / stepsPerCol, band / bandsPerRow);
      std::array<std::array<double, 2>, 4> uv = {{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
      const std::array<double, 3> mod = {1.0, 1.0, 1.0};
      for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
        std::array<std::array<double, 4>, 3> p{};
        std::array<std::array<double, 2>, 3> t{};
        for (int i = 0; i < 3; ++i) {
          p[i] = projected[ids[i]];
          t[i] = uv[ids[i]];
        }
        raster.triangle(p, t, tile ? tile->texture : Texture{}, mod);
      }
    }
  };

  const auto drawWall = [&](bool outer) {
    for (int band = 0; band < bands; ++band) {
      // Vertical tile row, with the A2 flip: the source-derived row is
      // (bands-1-band)/bandsPerRow, which reverses the tile rows against the
      // band order. Within a row the two bands split the tile's V in half,
      // reversed the same way so the art stays continuous up the tube.
      const int row = options.tileRowsFlipped ? (bands - 1 - band) / bandsPerRow
                                              : band / bandsPerRow;
      const int withinRow = options.tileRowsFlipped ? (bands - 1 - band) % bandsPerRow
                                                    : band % bandsPerRow;
      for (int step = 0; step < steps; ++step) {
        const int column = step / stepsPerCol;
        if (!options.fullWall && haveCollision_) {
          // File-side presence rule measured in the round note: a runtime
          // outer-wall cell is drawn exactly when at least one collision cell
          // of its (step,band) box is solid. The collision grid is 256 x 96,
          // so one angular step is 8 columns and one ring band is 16 rows.
          const int columnsPerStep = CollisionGrid::kWidth / steps;
          const int rowsPerBand = collision_.height > 0 ? collision_.height / bands : 0;
          bool solid = false;
          for (int dx = 0; dx < columnsPerStep && !solid; ++dx)
            for (int dy = 0; dy < rowsPerBand && !solid; ++dy)
              if (collision_.solid(step * columnsPerStep + dx, band * rowsPerBand + dy))
                solid = true;
          if (!solid)
            continue;
        }
        const Tile* tile = findTile(outer, column, row);
        if (tile == nullptr)
          continue;
        const int withinCol = step % stepsPerCol;
        const double u0 = static_cast<double>(withinCol) / stepsPerCol;
        const double u1 = static_cast<double>(withinCol + 1) / stepsPerCol;
        const double v0 = static_cast<double>(withinRow) / bandsPerRow;
        const double v1 = static_cast<double>(withinRow + 1) / bandsPerRow;
        // Corners: (band,step) (band+1,step) (band+1,step+1) (band,step+1).
        const std::array<std::array<double, 2>, 4> uv = {
            {{u0, v0}, {u0, v1}, {u1, v1}, {u1, v0}}};
        const int next = (step + 1) % steps;
        Vec3 corners[4] = {
            mesh_.vertex(band, step, outer),
            mesh_.vertex(band + 1, step, outer),
            mesh_.vertex(band + 1, next, outer),
            mesh_.vertex(band, next, outer),
        };
        drawQuad(corners, tile, outer ? options.outerContext : options.innerContext, uv);
      }
    }
  };

  if (options.drawWall) {
    if (options.innerFirst && options.drawInner)
      drawWall(false);
    if (options.drawOuter)
      drawWall(true);
    if (!options.innerFirst && options.drawInner)
      drawWall(false);
  }

  if (options.drawSurface)
    drawSurface();

  if (options.drawCellMesh) {
    // The runtime tube topology: for every 8x16 cell, the solid rectangle's
    // outer patch plus the wall-thickness ribbons on its open sides. UVs are
    // tile-local (u over 32 columns, v over 32 rows), so the texture is the
    // outer tile at (material % 8, material / 8).
    for (const auto& cell : cellMesh_.cells) {
      const Tile* tile = findTile(true, static_cast<int>(cell.material % 8),
                                  static_cast<int>(cell.material / 8));
      if (tile == nullptr)
        continue;
      const auto drawBank = [&](const std::vector<CellMeshQuad>& bank) {
        for (const auto& quad : bank) {
          std::array<std::array<double, 4>, 4> projected{};
          bool ok = true;
          for (int i = 0; i < 4 && ok; ++i) {
            const std::uint32_t index = quad.index[i];
            if (index >= mesh_.surface.size())
              ok = false;
            else
              ok = project(camera, mesh_.surface[index], projected[i]);
          }
          if (!ok)
            continue;
          const double c = std::clamp(quad.colour / 255.0, 0.0, 1.0);
          const std::array<double, 3> modulate = {c, c, c};
          for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
            std::array<std::array<double, 4>, 3> p{};
            std::array<std::array<double, 2>, 3> t{};
            for (int i = 0; i < 3; ++i) {
              p[i] = projected[ids[i]];
              t[i] = quad.uv[ids[i]];
            }
            raster.triangle(p, t, tile->texture, modulate);
          }
        }
      };
      drawBank(cell.mesh0);
      drawBank(cell.mesh4);
    }
  }

  if (options.drawCellRoute && cellMesh_.buckets > 0 && cellMesh_.bands > 0) {
    // The original's tube submission, `0x43A4EC`. Everything here is a port of
    // that walker and its four callees; see the round note for the addresses.
    //
    // The pass bounds. `0x43A4EC` composes the camera's own turn fraction into
    // a bucket centre (`0x43c990 -> fsub 0.5, fmul 32.0`), then brackets it by
    // the stored extent and 0.7 of it, chopping each bound toward zero with the
    // engine's float-to-int helper `0x49C5AC`, then shifting the whole set into
    // [0, 32] in 32s.
    const double extent = cylinderExtent_;
    double centre = (camera.selectorTurns - 0.5) * 32.0;
    while (centre < 0.0)
      centre += 32.0;
    const auto chop = [](double v) { return static_cast<int>(v); };
    int a = chop(centre - extent * 32.0);
    int b = chop(centre - extent * 0.7 * 32.0);
    int mid = chop(centre);
    int c = chop(centre + extent * 0.7 * 32.0);
    int d = chop(centre + extent * 32.0);
    while (a < 0) {
      a += 32; b += 32; mid += 32; c += 32; d += 32;
    }
    while (a > 32) {
      a -= 32; b -= 32; mid -= 32; c -= 32; d -= 32;
    }
    // `0x462465` compares each quad's child coordinate against the centre
    // bucket in eighths, i.e. the camera's own collision column.
    const double centreColumn = centre * 8.0;

    // The bank texture: A2's addressing (`0x460A2D`, `0x43AB3A`). The material
    // index is the cell's stored descriptor; +24 selects the inner bank, and
    // the tile position is (material % 8, material / 8) within that bank.
    const auto tileFor = [&](std::uint32_t materialIndex) -> const Tile* {
      const std::uint32_t tile = materialIndex % 24;
      const bool outer = (materialIndex < 24) != options.routeTexBankSwap;
      return findTile(outer, static_cast<int>(tile % 8), static_cast<int>(tile / 8));
    };

    const auto drawRouteQuad = [&](const CellMeshQuad& quad, std::uint32_t materialIndex,
                                   double context) {
      const Tile* tile = tileFor(materialIndex);
      if (tile == nullptr)
        return;
      std::array<std::array<double, 4>, 4> projected{};
      for (int i = 0; i < 4; ++i) {
        const std::uint32_t index = quad.index[i];
        if (index >= mesh_.surface.size())
          return;
        if (!project(camera, mesh_.surface[index], projected[i]))
          return;
        projected[i][2] -= context;
      }
      const double cc = std::clamp(quad.colour / 255.0, 0.0, 1.0);
      const std::array<double, 3> modulate = {cc, cc, cc};
      for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
        std::array<std::array<double, 4>, 3> p{};
        std::array<std::array<double, 2>, 3> t{};
        for (int i = 0; i < 3; ++i) {
          p[i] = projected[ids[i]];
          t[i] = quad.uv[ids[i]];
        }
        raster.triangle(p, t, tile->texture, modulate);
      }
    };

    const int bands = static_cast<int>(cellMesh_.bands);
    const bool swap = options.routeSwapBanks;
    const auto cellAt = [&](int bucket, int band) {
      return cellMesh_.at(bucket & 31, band);
    };
    // The side routes draw their bank from the last element back to the first
    // (`0x4623A5` / `0x462404`); the centre route walks both banks forward.
    const auto drawBankReversed = [&](const CellMeshCell& cell, bool useMesh4) {
      const auto& bank = useMesh4 ? cell.mesh4 : cell.mesh0;
      for (auto it = bank.rbegin(); it != bank.rend(); ++it)
        drawRouteQuad(*it, cell.material, options.outerContext);
    };

    // 1. INN (`0x4622FD`): buckets c..b descending, all bands, the entry+8
    //    whole-block quad, material + 24.
    if (!options.routeNoInner) {
      for (int i = c; i >= b; --i)
        for (int j = 0; j < bands; ++j) {
          const CellMeshCell* cell = cellAt(i, j);
          if (cell != nullptr && cell->hasInner)
            drawRouteQuad(cell->inner, cell->material + 24, options.innerContext);
        }
    }

    // 2. OUT right (`0x462404`): buckets d-1..mid+1 descending, mesh4.
    for (int i = d - 1; i > mid; --i)
      for (int j = 0; j < bands; ++j) {
        const CellMeshCell* cell = cellAt(i, j);
        if (cell != nullptr)
          drawBankReversed(*cell, !swap);
      }

    // 3. OUT left (`0x4623A5`): buckets a..mid-1, mesh0.
    for (int i = a; i < mid; ++i)
      for (int j = 0; j < bands; ++j) {
        const CellMeshCell* cell = cellAt(i, j);
        if (cell != nullptr)
          drawBankReversed(*cell, swap);
      }

    // 4. OUT centre (`0x462465`): the centre bucket, mesh0 forward where the
    //    child coordinate is below the camera column, then mesh4 forward where
    //    it is at or above it. Each record is drawn by exactly one of the two.
    for (int j = 0; j < bands; ++j) {
      const CellMeshCell* cell = cellAt(mid, j);
      if (cell == nullptr)
        continue;
      const auto& first = swap ? cell->mesh4 : cell->mesh0;
      const auto& second = swap ? cell->mesh0 : cell->mesh4;
      for (const auto& quad : first)
        if (options.routeFlipCentre ? (quad.coord >= centreColumn)
                                    : (quad.coord < centreColumn))
          drawRouteQuad(quad, cell->material, options.outerContext);
      for (const auto& quad : second)
        if (options.routeFlipCentre ? (quad.coord < centreColumn)
                                    : (quad.coord >= centreColumn))
          drawRouteQuad(quad, cell->material, options.outerContext);
    }
  }

  if (options.drawPolyList) {
    for (const auto& face : poly_) {
      std::array<std::array<double, 4>, 3> p{};
      bool ok = true;
      for (int i = 0; i < 3 && ok; ++i)
        ok = project(camera, face.v[i], p[i]);
      if (!ok)
        continue;
      const double angle = std::atan2(-face.v[0].x, face.v[0].z) / (2.0 * std::numbers::pi);
      int step = static_cast<int>(std::floor(angle * steps));
      if (step < 0) step += steps;
      step %= steps;
      int band = static_cast<int>(std::floor(face.v[0].y / std::max(1e-9, mesh_.ringSpacing())));
      band = std::clamp(band, 0, bands - 1);
      const Tile* tile = findTile(face.outerFacing, step / stepsPerCol, band / bandsPerRow);
      if (tile == nullptr)
        continue;
      std::array<std::array<double, 2>, 3> t = {{{0, 0}, {0.5, 1}, {1, 0}}};
      raster.triangle(p, t, tile->texture, face.modulate);
    }
  }
}

}  // namespace webo::app
