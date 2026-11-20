#pragma once

#include <span>
#include <array>

namespace webo::app {

// Cursor-to-lemming overlap and ranking. 0x00454D33 builds the candidate
// box and calls 0x00454CB9 from the scan at 0x00454D6C-0x00455236.
// Both boxes use render pixels with left<=right and top<=bottom. At
// 0x0049C5AC, the accessors truncate doubles: left +0x20 (0x00414770),
// top +0x28 (0x00414790), right +0xF0 (0x0044D860), bottom +0x90 (0x0044DFF0).
struct PickBox {
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;
};

// 0x00454CB9 compares the actor box (lemming+0x128) with cursorBox
// ([screen+0x3c]+0x10). The cursor sprite is 32x32 (0x0045578C).
// Shrink the box by 4 px horizontally and grow it by 4 px vertically.
// The adjusted edges are inclusive (0x00454CD6/0x00454CEC/0x00454D02/0x00454D18).
bool pickBoxOverlaps(const PickBox& box, const PickBox& cursorBox);

// 0x4551AB..0x4551EB computes squared distance from raw terrain vectors
// at lemming+8 and screen+0x58.
double pickDistanceSquared(const std::array<double, 3>& position,
                           const std::array<double, 3>& reference);

// Candidate for screen+0x16D8. At 0x004550D9, lemming+0x128 is the embedded box.
// Eligibility requires nonzero lemming+0x29 (0x00455104) and lemming+0x370
// (0x00455159 via 0x00455F20). 43A9A4/43AA30 write +29 from the tube bucket
// gate and actor +28. The full meaning of +0x370 is unresolved.
// priority comes from [state+0x14] (0x00430A70) via 0x004C05D0.
// distanceSquared uses lemming+8 and the controller's terrain target +58 (screen+0x58). At
// 0x004551AB-0x004551EB, 0x00401830 subtracts and 0x00418350 computes the dot
// product. The caller supplies both keys (initial -1 and 1e8 at 0x00454D7D/0x00455069).
struct PickCandidate {
  void* id = nullptr;  // identifies the lemming to the caller; opaque here
  PickBox box;
  bool eligible = true;
  int priority = 0;
  double distanceSquared = 0.0;
};

// Scan in actor-list order as at 0x004550A3-0x00455236. The original
// clears screen+0x16D8 at 0x00454DC2 in 0x00454D6C before choosing a hit.
// Among eligible overlaps, prefer highest priority (0x004551F4 jg) then
// smallest squared distance (FPU C0 via fcomp + test ah,1 at 0x004551F6-0x004551FE).
// Exact ties keep the first actor. Return nullptr when no candidate qualifies.
// Callers handle facing (screen+0x16b1/+0x16b2 versus lemming+0x40,
// 0x0045511B-0x00455153) and stock (0x0045A0E0(0x004C91D8, lemming),
// 0x00455194-0x004551A5) outside this geometry helper.
const PickCandidate* pickHoveredLemming(std::span<const PickCandidate> candidates,
                                         const PickBox& cursorBox);

}  // namespace webo::app
