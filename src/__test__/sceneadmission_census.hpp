// Shared test-only classifier for installed scenery structures.
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
// Independently check BackdropScene::build admission without using level IDs.
// Type-2 branch-1 nodes require a library for SPRITES\*.SPR and texture existence checks.
Outcome structuralOutcome(std::span<const std::byte> levelBytes,
                          const webo::AssetLibrary* library = nullptr);
}
