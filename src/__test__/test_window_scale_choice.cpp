#include "app/WindowScale.h"

using namespace webo::app;

static_assert(windowScaleChoice(3, windowScaleOverride(0, false)) == 3);
static_assert(windowScaleChoice(0, windowScaleOverride(0, false)) == 0);
static_assert(windowScaleChoice(3, windowScaleOverride(0, true)) == 1);
static_assert(windowScaleChoice(3, windowScaleOverride(2, false)) == 2);
static_assert(windowScaleChoice(3, windowScaleOverride(2, true)) == 2);

// The usable area is reduced by the title bar and borders before fitting.
static_assert(desktopWindowScale(0, largestFittingWindowScale(1910, 1002)) == 2);
static_assert(desktopWindowScale(0, largestFittingWindowScale(1920, 1439)) == 2);
static_assert(desktopWindowScale(0, largestFittingWindowScale(1920, 1440)) == 3);
static_assert(desktopWindowScale(0, largestFittingWindowScale(3000, 2000)) == 4);
static_assert(desktopWindowScale(0, largestFittingWindowScale(3840, 2879)) == 5);
static_assert(desktopWindowScale(3, largestFittingWindowScale(3000, 2000)) == 3);
static_assert(desktopWindowScale(3, largestFittingWindowScale(1910, 1002)) == 2);
static_assert(desktopWindowScale(1, largestFittingWindowScale(1910, 1002)) == 1);
static_assert(desktopWindowScale(3, largestFittingWindowScale(639, 479)) == 1);

int main() {}
