#pragma once

#include "assets/Bitmap.h"

namespace webo::app {
// Shared presentation for the live control and independent frame composition.
// The caller supplies the actual stored/table-selected rate, not a time or count.
void drawReleaseRateDisplay(Image& image, const Image& font, int rate);
}
