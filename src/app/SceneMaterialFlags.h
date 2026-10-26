#pragma once

#include "app/SceneRaster.h"

namespace webo::app {

// Conditional software row for the accepted successful-normal caret constructor
// route (leaf prefix '^' => encoded flags 8). This does not model cache hits or
// claim that the native device accepted these requests.
inline SceneRasterOptions conditionalCaretFlags8Policy(SceneRasterOptions options) {
  options.alphaFunction = SceneAlphaFunction::Disabled;
  options.alphaOperation = SceneAlphaOperation::SelectTexture;
  return options;
}

}  // namespace webo::app
