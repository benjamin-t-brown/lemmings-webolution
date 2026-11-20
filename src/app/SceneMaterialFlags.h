#pragma once

#include "app/SceneRaster.h"

namespace webo::app {

// Software material row for a successfully constructed texture with leaf
// prefix '^' and encoded flags 8.
inline SceneRasterOptions conditionalCaretFlags8Policy(SceneRasterOptions options) {
  options.alphaFunction = SceneAlphaFunction::Disabled;
  options.alphaOperation = SceneAlphaOperation::SelectTexture;
  return options;
}

}  // namespace webo::app
