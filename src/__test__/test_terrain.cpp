// Tests for the terrain mask.
//
// The behaviour that matters and is easy to get wrong is the wraparound: the
// level is a cylinder, so x = -1 and x = width - 1 are the same cell, and a
// box or circle that straddles the seam must act on both sides of it.

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

  const std::size_t circle = t.clearCircle(10, 10, 3);
  check(circle > 0, "clearCircle removes something");
  check(t.at(10, 10) == Material::Empty, "circle centre cleared");
  check(t.at(10, 14) == Material::Earth, "outside the radius is untouched");

  check(t.solidCells() == before - box - circle, "removed counts add up");

  // Steel resists both.
  Terrain s(10, 10, Material::Steel);
  check(s.clearBox(0, 0, 10, 10) == 0, "steel survives digging");
  check(s.clearCircle(5, 5, 4) == 0, "steel survives explosions");

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

  Terrain c(16, 16, Material::Earth);
  c.clearCircle(0, 8, 2);
  check(c.at(15, 8) == Material::Empty, "circle at x=0 reaches x=width-1");
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

}  // namespace

int main() {
  testWrap();
  testVerticalBounds();
  testClearAndFill();
  testSeamStraddling();
  testFromLevelGrid();
  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
