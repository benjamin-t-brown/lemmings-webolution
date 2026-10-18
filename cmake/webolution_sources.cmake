# Production sources, grouped by domain. Add every new .cpp here.
#
# Convention (shared with Carcer):
#   Name.h   public interface for a sibling Name.cpp
#   Name.hpp header-only, no sibling implementation
# A production .cpp requires a same-directory .h, except src/main.cpp and
# standalone programs under src/tools/ and src/__test__/.

set(WEBOLUTION_SOURCES
  # assets -- reading the original game's data files
  src/assets/Box.cpp
  src/assets/Lrz.cpp
  src/assets/Bitmap.cpp
  src/assets/Png.cpp
  src/assets/GameData.cpp
  src/assets/Sprite.cpp
  src/assets/LevelIndex.cpp
  src/assets/Cylinder.cpp
  src/assets/CollisionGrid.cpp
  src/assets/SurfaceMesh.cpp
  src/assets/LevelObjects.cpp
  src/assets/LevelStats.cpp
  src/assets/PolyList.cpp
  src/assets/Wav.cpp

  # sim -- the game itself, independent of rendering
  src/sim/Terrain.cpp
  src/sim/World.cpp

  # shell -- progress / unlock (no SDL)
  src/shell/Progress.cpp

)

list(TRANSFORM WEBOLUTION_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Presentation sources. Empty during the 2026-10-18 hard reset: the flat-first
# renderer and the placeholder shell moved to old/src/gfx and old/src/shell.
# The replacement is built 3D-first, from what the original's own code says,
# and is re-added here as it lands. src/audio/Mixer.cpp is kept and correct but
# has no target to link into until the new entry point exists.
set(WEBOLUTION_GFX_SOURCES)
list(TRANSFORM WEBOLUTION_GFX_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
