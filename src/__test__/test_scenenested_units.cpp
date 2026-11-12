// scene-nested-impl-001 (d): asset-free checks of the evidenced Secondary ordering and traversal
// (docs/scratch/scene-nested-research-001.md E6-E11). Expected values are hand-computed literals.
#include "app/SceneDispatch.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace webo;
using namespace webo::app;
namespace {
int failures = 0;
void check(bool ok, const char* what) { if (!ok) { ++failures; std::cerr << "FAIL: " << what << '\n'; } }
using Order = std::vector<std::size_t>;
Order order(std::vector<double> keys) { return binaryInsertionOrder(keys); }

// Identity-rotation node at translation (0,0,z); camera/source matrices are identity so key = z + bias.
SceneNode node(std::size_t offset, std::uint32_t type, double z, double radius, double bias) {
  SceneNode n; n.sourceOffset = offset; n.type = type;
  n.matrix[0] = n.matrix[4] = n.matrix[8] = 1; n.matrix[11] = z;
  n.headDoubles = {radius, bias};
  return n;
}
const std::array<double, 12> kIdentity{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
std::vector<SecondaryDispatchItem> run(const SceneNode& root, const SecondaryDispatchOptions& options = {}) {
  std::vector<SecondaryDispatchItem> out;
  dispatchSecondaryChildren(root, kIdentity, kIdentity, [&](const SecondaryDispatchItem& i) { out.push_back(i); }, options);
  return out;
}
Order offsets(const std::vector<SecondaryDispatchItem>& items) { Order o; for (const auto& i : items) o.push_back(i.nodeSourceOffset); return o; }
}  // namespace

int main() {
  // ---- E10 hand-computed tie cases (descending keys; idx starts {0}; lo/hi search then insert) ----
  check(order({}) .empty(), "empty input");
  check(order({7}) == Order{0}, "single key");
  check(order({5, 5}) == (Order{0, 1}), "A: two equal keys keep file order (size-1 path inserts at lo+1)");
  check(order({1, 1, 1}) == (Order{0, 2, 1}), "B: three equal keys -> 0,2,1");
  check(order({3, 3, 3, 3}) == (Order{0, 3, 2, 1}), "C: four equal keys -> 0,3,2,1");
  check(order({10, 5, 5}) == (Order{0, 2, 1}), "D: tie below a larger key is later-first");
  check(order({5, 5, 1}) == (Order{0, 1, 2}), "E: tie above a smaller key keeps file order");
  check(order({1, 5, 5}) == (Order{1, 2, 0}), "F: tie above a smaller key placed earlier-first");
  check(order({1, 2, 3}) == (Order{2, 1, 0}) && order({3, 2, 1}) == (Order{0, 1, 2}), "strict keys are descending");
  // The retained E11_KF_1 key list (scene-nested-research-001 section 2.3, by file index) and its measured order,
  // including the triple (23,22,17) and the earlier-first pair (0,1).
  const std::vector<double> keys = {691.500, 691.500, 385.435, 385.435, 705.052, 536.543, 424.629, 453.666, 404.091, 406.535, 305.593, 312.298, 381.384, 374.718,
                                    499.777, 334.607, 356.987, 560.183, 687.373, 687.373, 686.638, 686.638, 560.183, 560.183, 693.165, 693.165, 694.427, 694.427};
  check(order(keys) == (Order{4, 27, 26, 25, 24, 0, 1, 19, 18, 21, 20, 23, 22, 17, 5, 14, 7, 6, 9, 8, 3, 2, 12, 13, 16, 15, 11, 10}), "E11_KF_1 measured direct-child order");
  std::vector<std::size_t> viaSort(keys.size()); for (std::size_t i = 0; i < viaSort.size(); ++i) viaSort[i] = i;
  std::sort(viaSort.begin(), viaSort.end(), [&](std::size_t a, std::size_t b) { return keys[a] > keys[b]; });
  check(viaSort != order(keys), "std::sort differs from the evidenced order where keys tie");
  // Tie-free input: identical to std::sort (descending).
  {
    std::vector<double> k; std::uint32_t s = 12345;
    for (int i = 0; i < 80; ++i) { s = s * 1103515245u + 12345u; k.push_back(static_cast<double>(s >> 8) + i * 1e-3); }
    std::vector<std::size_t> idx(k.size()); for (std::size_t i = 0; i < idx.size(); ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](std::size_t a, std::size_t b) { return k[a] > k[b]; });
    check(order(k) == idx, "tie-free keys equal std::sort(key >)");
  }

  // ---- nested emission: depth-first, file order, immediately after the owner, never reordered ----
  {
    SceneNode root = node(1, 0, 0, 0, 0);
    SceneNode owner = node(10, 1, 10, 100, 0);                 // key 10
    owner.children.push_back(node(11, 1, 0, 100, 0));          // nested key 10 (file order first)
    owner.children.push_back(node(12, 1, 50, 100, 0));         // nested key 60: would sort first if nested children were ordered
    SceneNode grand = node(13, 1, 0, 100, 0); grand.children.push_back(node(14, 1, 0, 100, 0));
    owner.children.back().children.push_back(grand);
    root.children = {node(20, 1, 5, 100, 0), owner, node(21, 2, 7, 0, 0)};  // keys 5, 10, 7
    root.children[2].type2.visibility = 1;
    const auto items = run(root);
    check(offsets(items) == (Order{10, 11, 12, 13, 14, 21, 20}), "owner, its descendants depth-first in file order, then the next ordered child");
    check(items[0].depth == 1 && items[0].parentOffset == 1 && items[1].depth == 2 && items[1].parentOffset == 10 && items[3].depth == 3 && items[3].parentOffset == 12 &&
          items[4].depth == 4 && items[4].parentOffset == 13 && items[5].depth == 1 && items[5].parentOffset == 1, "depth and parentOffset");
  }
  // ---- visibility: the byte gates the node and its whole subtree ----
  {
    SceneNode root = node(1, 0, 0, 0, 0);
    SceneNode owner = node(10, 1, 10, 100, 0); owner.children.push_back(node(11, 1, 0, 100, 0));
    root.children = {owner, node(20, 1, 5, 100, 0)};
    SecondaryDispatchOptions hide;
    hide.hidden = [](const SceneNode& n) { return n.sourceOffset == 10; };
    check(offsets(run(root, hide)) == (Order{20}), "hidden owner skips itself and its child");
    hide.hidden = [](const SceneNode& n) { return n.sourceOffset == 11; };
    check(offsets(run(root, hide)) == (Order{10, 20}), "hidden child skips only the child");
    // Type-2 serialized byte: honoured only with the original gates.
    SceneNode t2 = node(30, 2, 3, 0, 0);  // visibility 0 by default
    SceneNode r2 = node(2, 0, 0, 0, 0); r2.children = {t2};
    check(offsets(run(r2)) == (Order{30}), "legacy default options emit a type-2 leaf unconditionally");
    SecondaryDispatchOptions gates; gates.originalGates = true;
    check(run(r2, gates).empty(), "type-2 visibility byte 0 draws nothing with the original gates");
    r2.children[0].type2.visibility = 1;
    check(offsets(run(r2, gates)) == (Order{30}), "type-2 visibility byte 1 is drawn");
  }
  // ---- type-1 radius cull: skip node and children when z + radius <= 0 ----
  {
    SecondaryDispatchOptions gates; gates.originalGates = true;
    SceneNode root = node(1, 0, 0, 0, 0);
    SceneNode behind = node(10, 1, -5, 5, 0);  behind.children.push_back(node(11, 1, 100, 1000, 0));  // z + r == 0 -> culled with its child
    SceneNode edge = node(12, 1, -5, 5.5, 0);                                                           // z + r = 0.5 -> kept
    root.children = {behind, edge, node(13, 1, 1, 0, 0)};
    check(offsets(run(root, gates)) == (Order{13, 12}), "z + radius <= 0 culls the node and its children; z + radius > 0 is drawn");
    check(offsets(run(root)).size() == 4, "without the gates nothing is culled");
    // A nested type-1 child is culled on its own accumulated depth.
    SceneNode owner = node(20, 1, 10, 100, 0); owner.children.push_back(node(21, 1, -20, 5, 0));
    root.children = {owner};
    check(offsets(run(root, gates)) == (Order{20}), "nested child behind the camera is culled by its composed depth");
  }
  std::cout << (failures ? "FAILED" : "PASSED") << " scenenested_units\n";
  return failures ? 1 : 0;
}
