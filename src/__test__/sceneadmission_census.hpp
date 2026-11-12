// scene-admission-001: independent structural classifier shared by the step-1
// census and the before/after demonstration (implemented in
// test_sceneadmission_census.cpp). Test-only; no production seam.
#pragma once
#include <cstddef>
#include <span>
#include <string>

namespace webo { class AssetLibrary; }
namespace sceneadmission {
struct Outcome {
  bool supported = false;   // every root passes the structural pipeline
  bool malformed = false;   // failure is malformed data, not an unsupported form
  std::string role, reason; // first failing role/reason (empty when supported)
};
// Independent re-implementation of the production structural pipeline with no
// level-ID dependence; used to cross-check BackdropScene::build admission.
// scene-branch-impl-001: `library` supplies the SPRITES\*.SPR / texture existence checks of
// the type-2 branch-1 admission mirror; without it any branch-1 node is unsupported (the
// pre-task rule).
Outcome structuralOutcome(std::span<const std::byte> levelBytes,
                          const webo::AssetLibrary* library = nullptr);
}
