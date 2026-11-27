#pragma once

namespace webo::web {
void setCanvasScale(int scale);
void canvasFillSize(int& width, int& height);
bool prefersPortraitLevel();
void setCanvasPortrait(bool portrait);
void nextFrame();
// True when it is time to advance the frame, false to service a queued click.
bool waitForLevelFrameOrClick();
bool replaceSave(const char* temporary, const char* target);
}
