cmake_minimum_required(VERSION 3.28)

project(webolution
  VERSION 0.1.0
  DESCRIPTION "Open reimplementation of Lemmings Revolution"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

include(cmake/webolution_sources.cmake)

# ---- shared object library ------------------------------------------------
# One compile of the production sources, reused by every executable.

add_library(webolution_objects OBJECT ${WEBOLUTION_SOURCES})
target_include_directories(webolution_objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(webolution_objects PUBLIC
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>)

# GCC's std::print/println live in libstdc++exp until it migrates to the main
# library. Harmless to link when unused; required the moment anyone uses them.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(webolution_objects PUBLIC stdc++exp)
endif()

# ---- SDL2 -----------------------------------------------------------------
# Emscripten supplies its own SDL2 via -sUSE_SDL=2, so it is not searched for.

if(NOT EMSCRIPTEN)
  find_package(SDL2 QUIET)
  if(NOT SDL2_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(SDL2 IMPORTED_TARGET sdl2)
    endif()
  endif()
endif()

set(WEBOLUTION_HAVE_SDL2 FALSE)
if(EMSCRIPTEN OR SDL2_FOUND)
  set(WEBOLUTION_HAVE_SDL2 TRUE)
endif()

# ---- the game -------------------------------------------------------------
# HARD RESET, 2026-10-18: src/main.cpp and src/gfx moved to old/. A4 restores a
# runnable, 3D-first entry point (src/app). The renderer rasterises on the CPU
# and presents through SDL2 when a window is requested; without a display the
# binary still renders and writes a PNG from file data (--shot).

add_executable(webolution ${WEBOLUTION_GFX_SOURCES})
target_link_libraries(webolution PRIVATE webolution_objects)

if(WEBOLUTION_HAVE_SDL2)
  target_compile_definitions(webolution PRIVATE WEBOLUTION_HAVE_SDL2=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(webolution PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(webolution PRIVATE PkgConfig::SDL2)
  endif()
else()
  target_compile_definitions(webolution PRIVATE WEBOLUTION_HAVE_SDL2=0)
endif()

# ---- tools ----------------------------------------------------------------

add_executable(lrtool src/tools/lrtool.cpp)
target_link_libraries(lrtool PRIVATE webolution_objects)

add_executable(lrcheck src/tools/lrcheck.cpp)
target_link_libraries(lrcheck PRIVATE webolution_objects)

# ---- tests ----------------------------------------------------------------

include(CTest)

# Moved to old/src/__test__ by the 2026-10-18 hard reset, because each one
# depended on src/gfx: test_cylinderview (camera math), test_cylinder's
# gfx::parseTilePosition checks (INN/OUT tile naming) and test_sprite's
# gfx::spriteNameForState checks (state -> sprite artwork). The asset-side
# coverage they also carried is a debt: re-establish it once the new renderer
# decides where tile parsing and state->sprite mapping live. See PLAN.md.

add_executable(test_box src/__test__/test_box.cpp)
target_link_libraries(test_box PRIVATE webolution_objects)
add_test(NAME box COMMAND test_box)

add_executable(test_spriteview src/__test__/test_spriteview.cpp)
target_link_libraries(test_spriteview PRIVATE webolution_objects)
add_test(NAME spriteview COMMAND test_spriteview)

# Restored asset-side coverage (debt 2): tile names carry their own position,
# and simulation states map to .SPR names. Both are renderer-independent.
add_executable(test_tileposition src/__test__/test_tileposition.cpp)
target_link_libraries(test_tileposition PRIVATE webolution_objects)
add_test(NAME tileposition COMMAND test_tileposition)

add_executable(test_spritestate src/__test__/test_spritestate.cpp)
target_link_libraries(test_spritestate PRIVATE webolution_objects)
add_test(NAME spritestate COMMAND test_spritestate)

add_executable(test_lrz src/__test__/test_lrz.cpp)
target_link_libraries(test_lrz PRIVATE webolution_objects)
add_test(NAME lrz COMMAND test_lrz)

add_executable(test_bitmap src/__test__/test_bitmap.cpp)
target_link_libraries(test_bitmap PRIVATE webolution_objects)
add_test(NAME bitmap COMMAND test_bitmap)

add_executable(test_gamedata src/__test__/test_gamedata.cpp)
target_link_libraries(test_gamedata PRIVATE webolution_objects)
add_test(NAME gamedata COMMAND test_gamedata)

add_executable(test_progress src/__test__/test_progress.cpp)
target_link_libraries(test_progress PRIVATE webolution_objects)
add_test(NAME progress COMMAND test_progress)


add_executable(test_levelindex src/__test__/test_levelindex.cpp)
target_link_libraries(test_levelindex PRIVATE webolution_objects)
add_test(NAME levelindex COMMAND test_levelindex)


add_executable(test_surface src/__test__/test_surface.cpp)
target_link_libraries(test_surface PRIVATE webolution_objects)
add_test(NAME surface COMMAND test_surface)

add_executable(test_levelobjects src/__test__/test_levelobjects.cpp)
target_link_libraries(test_levelobjects PRIVATE webolution_objects)
add_test(NAME levelobjects COMMAND test_levelobjects)

add_executable(test_polylist src/__test__/test_polylist.cpp)
target_link_libraries(test_polylist PRIVATE webolution_objects)
add_test(NAME polylist COMMAND test_polylist)

add_executable(test_wav src/__test__/test_wav.cpp)
target_link_libraries(test_wav PRIVATE webolution_objects)
add_test(NAME wav COMMAND test_wav)


add_executable(test_terrain src/__test__/test_terrain.cpp)
target_link_libraries(test_terrain PRIVATE webolution_objects)
add_test(NAME terrain COMMAND test_terrain)

add_executable(test_world src/__test__/test_world.cpp)
target_link_libraries(test_world PRIVATE webolution_objects)
add_test(NAME world COMMAND test_world)

add_executable(test_corpus src/__test__/test_corpus.cpp)
target_link_libraries(test_corpus PRIVATE webolution_objects)
add_test(NAME corpus COMMAND test_corpus)

add_executable(test_cellmesh src/__test__/test_cellmesh.cpp)
target_link_libraries(test_cellmesh PRIVATE webolution_objects)
add_test(NAME cellmesh COMMAND test_cellmesh)

add_executable(test_camerainput src/__test__/test_camerainput.cpp src/app/CameraInput.cpp)
target_include_directories(test_camerainput PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME camerainput COMMAND test_camerainput)

add_executable(test_collisiongrid src/__test__/test_collisiongrid.cpp)
target_link_libraries(test_collisiongrid PRIVATE webolution_objects)
# The live grid dumps sit in build/live_dump beside the tree, not in the game
# directory, so the test needs to know where the tree is.
target_compile_definitions(test_collisiongrid
  PRIVATE WEBOLUTION_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME collisiongrid COMMAND test_collisiongrid)

# Tests that touch real game data need an installation to point at. Supply it
# with -DWEBOLUTION_GAME_DIR=... ; without it those tests report a skip.
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(box lrz bitmap gamedata progress levelindex surface wav levelobjects polylist world corpus collisiongrid cellmesh tileposition spritestate PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()
