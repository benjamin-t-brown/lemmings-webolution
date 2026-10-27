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

add_executable(webolution ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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

# B1e repair 1: offline decoded retail template export through production readers.
add_executable(FrontendAssetDump src/tools/FrontendAssetDump.cpp)
target_link_libraries(FrontendAssetDump PRIVATE webolution_objects)

# B1w: highlight-0 main menu render from the read/observed inputs (offline).
add_executable(MenuRenderDump src/tools/MenuRenderDump.cpp src/shell/MenuRender.cpp)
target_link_libraries(MenuRenderDump PRIVATE webolution_objects)

add_executable(actorfixture src/tools/actorfixture.cpp src/app/ActorSprite.cpp src/app/Camera.cpp)
target_link_libraries(actorfixture PRIVATE webolution_objects)

add_executable(SimReleaseTrace src/tools/SimReleaseTrace.cpp)
target_link_libraries(SimReleaseTrace PRIVATE webolution_objects)

# D1f (round 6): re-added. old/src/tools/compare_tick.cpp already exists and
# already builds against the current World/LevelSetup/Terrain API unchanged;
# tools/harness/measure_steppers.py --compare shells out to this exact path
# (build/cmake/<preset>/compare_tick.exe).
add_executable(compare_tick old/src/tools/compare_tick.cpp)
target_link_libraries(compare_tick PRIVATE webolution_objects)

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

add_executable(test_actorsprite src/__test__/test_actorsprite.cpp src/app/ActorSprite.cpp src/app/Camera.cpp)
target_link_libraries(test_actorsprite PRIVATE webolution_objects)
add_test(NAME actorsprite COMMAND test_actorsprite)

add_executable(test_sceneraster src/__test__/test_sceneraster.cpp src/app/SceneRaster.cpp)
target_include_directories(test_sceneraster PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME sceneraster COMMAND test_sceneraster)

add_executable(test_sceneclip src/__test__/test_sceneclip.cpp src/app/SceneClip.cpp)
target_include_directories(test_sceneclip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME sceneclip COMMAND test_sceneclip)

add_executable(test_scenedepth src/__test__/test_scenedepth.cpp src/app/SceneDepth.cpp src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneRaster.cpp src/app/SceneBillboardSource.cpp)
target_include_directories(test_scenedepth PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenedepth COMMAND test_scenedepth)

add_executable(test_actorpose src/__test__/test_actorpose.cpp src/app/ActorPose.cpp)
target_link_libraries(test_actorpose PRIVATE webolution_objects)
add_test(NAME actorpose COMMAND test_actorpose)

add_executable(test_actorterrain src/__test__/test_actorterrain.cpp src/app/ActorTerrain.cpp)
target_link_libraries(test_actorterrain PRIVATE webolution_objects)
add_test(NAME actorterrain COMMAND test_actorterrain)

add_executable(test_actoranim src/__test__/test_actoranim.cpp src/app/ActorAnim.cpp)
target_link_libraries(test_actoranim PRIVATE webolution_objects)
add_test(NAME actoranim COMMAND test_actoranim WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_scenebillboardsource
  src/__test__/test_scenebillboardsource.cpp
  src/app/SceneBillboardSource.cpp)
target_link_libraries(test_scenebillboardsource PRIVATE webolution_objects)
add_test(NAME scenebillboardsource COMMAND test_scenebillboardsource)

add_executable(test_actorcompose
  src/__test__/test_actorcompose.cpp
  src/app/TubeScene.cpp
  src/app/Camera.cpp
  src/app/Raster.cpp
  src/app/ActorSprite.cpp
  src/app/ActorPose.cpp)
target_link_libraries(test_actorcompose PRIVATE webolution_objects z)
target_compile_definitions(test_actorcompose PRIVATE WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME actorcompose COMMAND test_actorcompose WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

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

# B3j: production conditional map compositor against accepted SOURCE images.
add_executable(test_mapscene src/__test__/test_mapscene.cpp src/app/MapScene.cpp)
target_link_libraries(test_mapscene PRIVATE webolution_objects)
target_compile_definitions(test_mapscene PRIVATE WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME mapscene COMMAND test_mapscene WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_gamedata src/__test__/test_gamedata.cpp)
target_link_libraries(test_gamedata PRIVATE webolution_objects)
add_test(NAME gamedata COMMAND test_gamedata)

# B1a: app-side wiring proof -- a representative background texture, the
# font texture and GERMAN.LTX's strings, through the same production readers
# (loadTexture/loadStrings) src/app/TubeScene.cpp already calls. No new
# reverse engineering; see src/__test__/test_frontend.cpp's own header.
add_executable(test_frontend src/__test__/test_frontend.cpp)
target_link_libraries(test_frontend PRIVATE webolution_objects)
add_test(NAME frontend COMMAND test_frontend)

# C1b: the panel-icon compositing primitive (offline) plus the real-archive
# negative finding that GreyIcons/BlackIcons/AdditiveIcons are not the ten
# skill-panel icons (gated the same way `frontend`'s archive half is).
add_executable(test_skillpanel src/__test__/test_skillpanel.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_skillpanel PRIVATE webolution_objects)
add_test(NAME skillpanel COMMAND test_skillpanel)

add_executable(test_progress src/__test__/test_progress.cpp)
target_link_libraries(test_progress PRIVATE webolution_objects)
add_test(NAME progress COMMAND test_progress)

# B1w: the menu renderer's rules plus the real-archive check of the glyph table.
add_executable(test_menurender src/__test__/test_menurender.cpp src/shell/MenuRender.cpp)
target_link_libraries(test_menurender PRIVATE webolution_objects)
add_test(NAME menurender COMMAND test_menurender)

# C2g: HUD counters and timer layout (font table and widths from MenuRender, read-only).
add_executable(test_hudtext src/__test__/test_hudtext.cpp src/shell/HudText.cpp src/shell/MenuRender.cpp)
target_link_libraries(test_hudtext PRIVATE webolution_objects)
add_test(NAME hudtext COMMAND test_hudtext)

# C2i: pause caption panel rectangle and fill.
add_executable(test_pausecaption src/__test__/test_pausecaption.cpp src/shell/PauseCaption.cpp)
target_link_libraries(test_pausecaption PRIVATE webolution_objects)
add_test(NAME pausecaption COMMAND test_pausecaption)

add_executable(test_runtimehud src/__test__/test_runtimehud.cpp src/app/RuntimeHud.cpp src/app/SimClock.cpp src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp)
target_link_libraries(test_runtimehud PRIVATE webolution_objects)
add_test(NAME runtimehud COMMAND test_runtimehud)

# C2o: offline byte-supported cursor selection policy; no SDL or live target.
add_executable(test_cursorpolicy src/__test__/test_cursorpolicy.cpp src/app/CursorPolicy.cpp)
target_include_directories(test_cursorpolicy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME cursorpolicy COMMAND test_cursorpolicy)

# B2f: frontend screen sequence replayed from the b2e capture. Frontend.cpp is built
# into the test directly (as test_skillpanel does) so the shared source list stays put.
add_executable(test_frontend_sequence src/__test__/test_frontend_sequence.cpp src/shell/Frontend.cpp)
target_link_libraries(test_frontend_sequence PRIVATE webolution_objects)
add_test(NAME frontend_sequence COMMAND test_frontend_sequence)


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

add_executable(test_scenenode src/__test__/test_scenenode.cpp)
target_link_libraries(test_scenenode PRIVATE webolution_objects)
add_test(NAME scenenode COMMAND test_scenenode WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# C3ae production scene integration, with optional retail-backed root/source/pixel checks.
add_executable(test_backdropscene
  src/__test__/test_backdropscene.cpp
  src/app/BackdropScene.cpp
  src/app/SceneBillboardSource.cpp
  src/app/SceneDispatch.cpp
  src/app/SceneDepth.cpp
  src/app/SceneVertexPack.cpp
  src/app/SceneDiffuse.cpp
  src/app/Camera.cpp
  src/app/Raster.cpp
  src/app/SceneRaster.cpp)
target_link_libraries(test_backdropscene PRIVATE webolution_objects)
target_include_directories(test_backdropscene PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME backdropscene COMMAND test_backdropscene WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_scenedispatch src/__test__/test_scenedispatch.cpp src/app/SceneDispatch.cpp)
target_include_directories(test_scenedispatch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenedispatch COMMAND test_scenedispatch)

add_executable(test_scenematerialflags
  src/__test__/test_scenematerialflags.cpp
  src/app/SceneRaster.cpp)
target_include_directories(test_scenematerialflags PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenematerialflags COMMAND test_scenematerialflags)

add_executable(test_scenevertexpack
  src/__test__/test_scenevertexpack.cpp
  src/app/SceneDiffuse.cpp
  src/app/SceneVertexPack.cpp)
target_include_directories(test_scenevertexpack PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenevertexpack COMMAND test_scenevertexpack)

add_executable(test_scene_pack_adoption src/__test__/test_scene_pack_adoption.cpp src/app/SceneDepth.cpp src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneRaster.cpp src/app/SceneBillboardSource.cpp)
target_include_directories(test_scene_pack_adoption PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scene_pack_adoption COMMAND test_scene_pack_adoption)

add_executable(test_scenediffuse
  src/__test__/test_scenediffuse.cpp
  src/app/SceneDiffuse.cpp
  src/app/SceneRaster.cpp)
target_include_directories(test_scenediffuse PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenediffuse COMMAND test_scenediffuse)

add_executable(test_scenemeshsource src/__test__/test_scenemeshsource.cpp)
target_link_libraries(test_scenemeshsource PRIVATE webolution_objects)
add_test(NAME scenemeshsource COMMAND test_scenemeshsource)

add_executable(test_wav src/__test__/test_wav.cpp)
target_link_libraries(test_wav PRIVATE webolution_objects)
add_test(NAME wav COMMAND test_wav)


add_executable(test_terrain src/__test__/test_terrain.cpp)
target_link_libraries(test_terrain PRIVATE webolution_objects)
add_test(NAME terrain COMMAND test_terrain)

add_executable(test_world src/__test__/test_world.cpp)
target_link_libraries(test_world PRIVATE webolution_objects)
add_test(NAME world COMMAND test_world)

# D3a: the production retail-level loader (sim/RetailLevel.*). Writes
# build/tmp/d3a-retail-level-setup/sweep.txt as a deliverable when
# WEBOLUTION_GAME_DIR is set, so it needs the source tree as its working
# directory, the same reason actoranim/actorcompose already set it.
add_executable(test_retaillevel src/__test__/test_retaillevel.cpp)
target_link_libraries(test_retaillevel PRIVATE webolution_objects)
add_test(NAME retaillevel COMMAND test_retaillevel WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_corpus src/__test__/test_corpus.cpp)
target_link_libraries(test_corpus PRIVATE webolution_objects)
add_test(NAME corpus COMMAND test_corpus)

add_executable(test_cellmesh src/__test__/test_cellmesh.cpp)
target_link_libraries(test_cellmesh PRIVATE webolution_objects)
add_test(NAME cellmesh COMMAND test_cellmesh)

add_executable(test_camerainput src/__test__/test_camerainput.cpp src/app/CameraInput.cpp)
target_include_directories(test_camerainput PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME camerainput COMMAND test_camerainput)

add_executable(test_simclock src/__test__/test_simclock.cpp src/app/SimClock.cpp)
target_include_directories(test_simclock PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME simclock COMMAND test_simclock)

add_executable(test_pick src/__test__/test_pick.cpp src/app/Pick.cpp)
target_include_directories(test_pick PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME pick COMMAND test_pick)

# D3c: offline click-to-assign wiring. Same shape as test_retaillevel (links
# the object library, needs the install) plus the app sources the accessor
# compiles with, following test_actorcompose.
add_executable(test_clicktoassign
  src/__test__/test_clicktoassign.cpp
  src/app/TubeScene.cpp
  src/app/Camera.cpp
  src/app/Raster.cpp
  src/app/ActorSprite.cpp
  src/app/ActorPose.cpp
  src/app/Pick.cpp)
target_link_libraries(test_clicktoassign PRIVATE webolution_objects)
target_compile_definitions(test_clicktoassign PRIVATE WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME clicktoassign COMMAND test_clicktoassign WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

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
  set_tests_properties(box lrz bitmap gamedata frontend skillpanel progress levelindex surface wav levelobjects polylist scenenode backdropscene world retaillevel corpus collisiongrid cellmesh tileposition spritestate actorpose actorcompose clicktoassign scenemeshsource scenebillboardsource mapscene PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()
