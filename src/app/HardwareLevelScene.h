#pragma once

#include "app/PreparedLevelFrame.h"
#include "app/PresentationBackend.h"

namespace webo::app {

class HardwareLevelScene {
 public:
  virtual ~HardwareLevelScene() = default;
  virtual std::expected<void, std::string> draw(const PreparedLevelScene&) = 0;
  virtual GlSceneImage image() const = 0;
};

std::expected<std::unique_ptr<HardwareLevelScene>, std::string> createHardwareLevelScene(
    GlProgramCache&);

}  // namespace webo::app
