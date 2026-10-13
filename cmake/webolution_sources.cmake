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
)

list(TRANSFORM WEBOLUTION_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
