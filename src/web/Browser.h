#pragma once

namespace webo::web {
void setCanvasScale(int scale);
void canvasFillSize(int& width, int& height);
// Apply the CSS layout and return the matching render height, or zero for landscape.
int layoutPortraitCanvas(bool allowed, int renderWidth);
void nextFrame();
// True when it is time to advance the frame, false to service a queued click.
bool waitForLevelFrameOrClick();
bool replaceSave(const char* temporary, const char* target);
}
