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

  # sim -- the game itself, independent of rendering
  src/sim/Terrain.cpp
  src/sim/World.cpp

)

list(TRANSFORM WEBOLUTION_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Presentation sources. Separate, because they need SDL2 and the asset tools
# and tests do not.
set(WEBOLUTION_GFX_SOURCES
  src/gfx/Gl.cpp
  src/gfx/Renderer.cpp
)
list(TRANSFORM WEBOLUTION_GFX_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
