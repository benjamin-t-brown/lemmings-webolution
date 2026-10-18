// Tests for the terrain mask.
//
// The behaviour that matters and is easy to get wrong is the wraparound: the
// level is a cylinder, so x = -1 and x = width - 1 are the same cell, and a
// box or circle that straddles the seam must act on both sides of it.

#include "assets/CollisionGrid.h"
#include "sim/Terrain.h"

#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

using webo::sim::Material;
using webo::sim::Terrain;

void testWrap() {
  std::cout << "horizontal wraparound\n";
  Terrain t(10, 5);
  t.set(0, 0, Material::Earth);
  check(t.at(0, 0) == Material::Earth, "set and read");
  check(t.at(10, 0) == Material::Earth, "x = width wraps to 0");
  check(t.at(-10, 0) == Material::Earth, "x = -width wraps to 0");
  check(t.wrapX(-1) == 9, "wrapX(-1)");
  check(t.wrapX(23) == 3, "wrapX(23)");

  t.set(-1, 1, Material::Rock);
  check(t.at(9, 1) == Material::Rock, "writing past the left edge wraps");
}

void testVerticalBounds() {
  std::cout << "vertical bounds\n";
  Terrain t(8, 4, Material::Earth);
  check(t.at(0, -1) == Material::Empty, "above the world is sky");
  check(t.at(0, 4) == Material::Steel, "below the world is solid");
  check(t.at(0, 400) == Material::Steel, "far below is still solid");
  t.set(0, -1, Material::Rock);
  t.set(0, 9, Material::Rock);
  check(t.solidCells() == 8 * 4, "out-of-range writes are ignored");
}

void testClearAndFill() {
  std::cout << "digging and building\n";
  Terrain t(20, 20, Material::Earth);
  const std::size_t before = t.solidCells();

  const std::size_t box = t.clearBox(2, 2, 4, 3);
  check(box == 12, std::format("clearBox removes w*h cells (got {})", box));
  check(t.at(2, 2) == Material::Empty && t.at(5, 4) == Material::Empty, "box corners cleared");
  check(t.at(6, 2) == Material::Earth, "just outside the box is untouched");

  // The explosion stencil, written out from 0x004B1528: 37 of 49 cells, in a
  // 7-wide window at columns x-4..x+2 and rows y-4..y+2. A blast at (10, 10)
  // therefore spans columns 6..12 and rows 6..12.
  const std::size_t blast = t.clearExplosion(10, 10);
  check(blast == 37, std::format("the blast stencil is 37 cells (got {})", blast));
  check(t.at(9, 6) == Material::Empty && t.at(6, 10) == Material::Empty,
        "the stencil's top row and its left edge are cleared");
  check(t.at(6, 6) == Material::Earth && t.at(12, 12) == Material::Earth,
        "and its corners are not");
  check(t.at(13, 10) == Material::Earth && t.at(5, 10) == Material::Earth,
        "nor is the column either side of the window");
  check(t.at(10, 13) == Material::Earth && t.at(10, 5) == Material::Earth,
        "nor the row above or below it");

  check(t.solidCells() == before - box - blast, "removed counts add up");

  // Steel resists both.
  Terrain s(10, 10, Material::Steel);
  check(s.clearBox(0, 0, 10, 10) == 0, "steel survives digging");
  check(s.clearExplosion(5, 5) == 0, "steel survives explosions");

  // And so does a cell carrying a second blocker's field -- 0x0043F63E.
  Terrain f(20, 20, Material::Earth);
  f.setBlockerField(10, 10, true);
  const std::size_t guarded = f.clearExplosion(10, 10);
  check(guarded == 36, std::format("a blocker's field cell survives (got {})", guarded));
  check(f.at(10, 10) == Material::Earth, "the guarded cell is still there");

  // Building only fills empty cells.
  Terrain b(10, 10);
  b.set(3, 3, Material::Rock);
  const std::size_t filled = b.fillBox(2, 2, 3, 3, Material::Built);
  check(filled == 8, std::format("fillBox skips occupied cells (got {})", filled));
  check(b.at(3, 3) == Material::Rock, "existing cell not overwritten");
}

void testSeamStraddling() {
  std::cout << "operations across the seam\n";
  Terrain t(16, 8, Material::Earth);
  // A box starting near the right edge must continue past x = 0.
  const std::size_t removed = t.clearBox(14, 0, 4, 1);
  check(removed == 4, std::format("box across the seam clears 4 (got {})", removed));
  check(t.at(14, 0) == Material::Empty && t.at(15, 0) == Material::Empty, "right of the seam");
  check(t.at(0, 0) == Material::Empty && t.at(1, 0) == Material::Empty, "left of the seam");

  // The blast window wraps too: 0x0043F804's `and eax, 0xff` wraps the
  // column inside the inner loop.
  Terrain c(16, 16, Material::Earth);
  c.clearExplosion(2, 8);
  check(c.at(14, 8) == Material::Empty,
        "a blast at x=2 reaches x=width-2 across the seam");
  check(c.at(13, 8) == Material::Earth, "and stops there");
}

void testFromLevelGrid() {
  std::cout << "building from a level grid\n";
  const std::vector<std::uint32_t> grid = {
      0xFFFFFFFF, 1, 0xFFFFFFFF, 3,
      1,          1, 0xFFFFFFFF, 0xFFFFFFFF,
  };
  Terrain t = Terrain::fromLevelGrid(grid, 4, 2);
  check(t.width() == 4 && t.height() == 2, "dimensions");
  check(t.at(0, 0) == Material::Empty, "0xFFFFFFFF is empty");
  check(t.at(1, 0) == Material::Earth, "anything else is solid");
  check(t.solidCells() == 4, "solid count");

  // A short grid must not read past its end.
  Terrain small = Terrain::fromLevelGrid(std::span(grid).first(2), 4, 2);
  check(small.solidCells() == 1, "a short grid fills what it can");
}

void testAntiGravFromCollisionGrid() {
  // Round 11 gap: every prior anti-grav test built pads with setAntiGrav, so
  // the 0x8000 extraction in fromCollisionGrid could be deleted and the
  // suite would stay green. A hand-built grid word pins the line.
  std::cout << "anti-grav bit fromCollisionGrid\n";
  webo::CollisionGrid g;
  g.width = 8;
  g.height = 4;
  g.cells.assign(static_cast<std::size_t>(g.width) * g.height, 0);
  // File row 1, col 3: solid + 0x8000. fromCollisionGrid flips file row r
  // to terrain row height-1-r, so the pad lands at terrain (3, 2).
  g.cells[static_cast<std::size_t>(1) * g.width + 3] =
      webo::kCellSolid | webo::kCellFlag0x8000;
  const Terrain t = Terrain::fromCollisionGrid(g);
  check(t.antiGravCells() == 1, "one pad extracted");
  check(t.antiGrav(3, 2), "pad at flipped terrain cell (3, 2)");
  check(!t.antiGrav(3, 1), "neighbours are clear");
}

void testWaterHazardFromCollisionGrid() {
  // Round 14: 0x40 extraction in fromCollisionGrid.
  std::cout << "water-hazard bit fromCollisionGrid\n";
  webo::CollisionGrid g;
  g.width = 8;
  g.height = 4;
  g.cells.assign(static_cast<std::size_t>(g.width) * g.height, 0);
  g.cells[static_cast<std::size_t>(1) * g.width + 3] =
      webo::kCellSolid | webo::kCellFlag0x40;
  const Terrain t = Terrain::fromCollisionGrid(g);
  check(t.waterHazardCells() == 1, "one hazard extracted");
  check(t.waterHazard(3, 2), "hazard at flipped terrain cell (3, 2)");
  check(!t.waterHazard(3, 1), "neighbours are clear");
}

}  // namespace

int main() {
  testWrap();
  testVerticalBounds();
  testClearAndFill();
  testSeamStraddling();
  testFromLevelGrid();
  testAntiGravFromCollisionGrid();
  testWaterHazardFromCollisionGrid();
  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
