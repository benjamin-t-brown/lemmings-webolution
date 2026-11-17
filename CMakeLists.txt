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

add_library(webolution_texture_decoder STATIC ${WEBOLUTION_TEXTURE_DECODER_SOURCES})
target_include_directories(webolution_texture_decoder PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/include)

add_library(webolution_sprite_resource STATIC ${WEBOLUTION_SPRITE_RESOURCE_SOURCES})
target_include_directories(webolution_sprite_resource PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/include)

# Narrow serialized compiled-level reader, linked transitively through
# webolution_objects so the application and the source tests share it. It
# links webolution_sprite_resource for the already-compiled level_stream.cpp
# instead of compiling that translation unit a second time into one binary.
add_library(webolution_compiled_level STATIC ${WEBOLUTION_COMPILED_LEVEL_SOURCES})
target_include_directories(webolution_compiled_level PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/include)
target_link_libraries(webolution_compiled_level PUBLIC webolution_sprite_resource)

# ---- shared object library ------------------------------------------------
# One compile of the production sources, reused by every executable.

add_library(webolution_objects OBJECT ${WEBOLUTION_SOURCES})
target_include_directories(webolution_objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(webolution_objects PUBLIC
  webolution_texture_decoder webolution_sprite_resource webolution_compiled_level)

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

# Production audio: the mixer and the cue adapter are compiled into the shared
# object library, so SDL2 reaches every executable that links it. Emscripten
# supplies SDL2 through -sUSE_SDL=2 and is a later phase.
if(WEBOLUTION_HAVE_SDL2)
  target_sources(webolution_objects PRIVATE ${WEBOLUTION_AUDIO_SOURCES})
  if(NOT EMSCRIPTEN)
    if(TARGET SDL2::SDL2)
      target_link_libraries(webolution_objects PUBLIC SDL2::SDL2)
    elseif(TARGET PkgConfig::SDL2)
      target_link_libraries(webolution_objects PUBLIC PkgConfig::SDL2)
    endif()
  endif()
endif()

include(cmake/webolution_app.cmake)

# ---- the game -------------------------------------------------------------
# HARD RESET, 2026-10-18: src/main.cpp and src/gfx moved to old/. A4 restores a
# runnable, 3D-first entry point (src/app). The renderer rasterises on the CPU
# and presents through SDL2 when a window is requested; without a display the
# binary still renders and writes a PNG from file data (--shot).

add_executable(webolution src/app/main.cpp)
target_link_libraries(webolution PRIVATE webolution_app)
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

# Offline snapshot of the production FrontendSession renderer (no SDL/window).
add_executable(FrontendRenderDump src/tools/FrontendRenderDump.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendSession.cpp src/app/MapScene.cpp src/app/RuntimeCursor.cpp
  src/app/CursorPolicy.cpp src/app/SkillPanel.cpp src/shell/Frontend.cpp
  src/shell/MenuRender.cpp src/shell/HudText.cpp)
target_link_libraries(FrontendRenderDump PRIVATE webolution_objects)

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

# shared-timing-impl-001: the connected explicit elapsed-time path (RuntimeSimState::advanceElapsed
# -> World::advanceElapsedMs) on installed MM_W_012. Observations go to build/shared-timing-impl-001.
add_executable(test_sharedtiming src/__test__/test_sharedtiming.cpp
  src/app/RuntimeHud.cpp src/app/SimClock.cpp src/app/LevelEndTransition.cpp
  src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp
  src/app/RuntimeSkillPanel.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_sharedtiming PRIVATE webolution_objects)
target_compile_definitions(test_sharedtiming PRIVATE STI_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME sharedtiming
  COMMAND test_sharedtiming "${CMAKE_SOURCE_DIR}/build/shared-timing-impl-001/connected-path-observations.txt")
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(sharedtiming PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_levergates src/__test__/test_levergates.cpp
  src/app/RuntimeHud.cpp src/app/SimClock.cpp src/app/LevelEndTransition.cpp
  src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp
  src/app/RuntimeSkillPanel.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_levergates PRIVATE webolution_objects)
add_test(NAME levergates
  COMMAND test_levergates "${CMAKE_SOURCE_DIR}/build/lever-gates-impl-001/after-observations.txt")
set_tests_properties(levergates PROPERTIES TIMEOUT 300)
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(levergates PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_leverinitface src/__test__/test_leverinitface.cpp
  src/app/RuntimeHud.cpp src/app/SimClock.cpp src/app/LevelEndTransition.cpp
  src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp
  src/app/RuntimeSkillPanel.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_leverinitface PRIVATE webolution_objects)
add_test(NAME leverinitface
  COMMAND test_leverinitface "${CMAKE_SOURCE_DIR}/build/lever-initial-face-impl-001/after-observations.txt")
set_tests_properties(leverinitface PROPERTIES TIMEOUT 300)
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(leverinitface PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

# pad-pipe-presentation-001: removed legacy pressurepad (required immediate beam disappearance).

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
  src/app/ActorPose.cpp
  src/app/ActorAnim.cpp
  src/app/LemmingAnim.cpp)
target_link_libraries(test_actorcompose PRIVATE webolution_objects z)

# Playtest 7 W5: independent noninteractive exit tails; legacy, no gameplay route.
add_executable(test_exit_jump src/__test__/test_exit_jump.cpp)
target_link_libraries(test_exit_jump PRIVATE webolution_app webolution_objects)
add_test(NAME exit_jump COMMAND test_exit_jump "${WEBOLUTION_GAME_DIR}")
set_tests_properties(exit_jump PROPERTIES TIMEOUT 10)
target_compile_definitions(test_actorcompose PRIVATE WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME actorcompose COMMAND test_actorcompose WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Restored asset-side coverage (debt 2): tile names carry their own position,
# and simulation states map to .SPR names. Both are renderer-independent.
add_executable(test_tileposition src/__test__/test_tileposition.cpp)
target_link_libraries(test_tileposition PRIVATE webolution_objects)
add_test(NAME tileposition COMMAND test_tileposition)

add_executable(test_spritestate src/__test__/test_spritestate.cpp)
target_link_libraries(test_spritestate PRIVATE webolution_objects)
target_compile_definitions(test_spritestate PRIVATE
  WEBOLUTION_SPRITE_SCRATCH_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/build/spritestate-test")
add_test(NAME spritestate COMMAND test_spritestate)

add_executable(test_lrz src/__test__/test_lrz.cpp)
target_link_libraries(test_lrz PRIVATE webolution_objects)
add_test(NAME lrz COMMAND test_lrz)

add_executable(test_bitmap src/__test__/test_bitmap.cpp)
target_link_libraries(test_bitmap PRIVATE webolution_objects)
target_compile_definitions(test_bitmap PRIVATE
  WEBOLUTION_BITMAP_TEST_DIR="${CMAKE_CURRENT_BINARY_DIR}/bitmap-test")
add_test(NAME bitmap COMMAND test_bitmap)

# B3j: production conditional map compositor against accepted SOURCE images.
add_executable(test_mapscene src/__test__/test_mapscene.cpp src/app/MapScene.cpp
  src/shell/MenuRender.cpp src/shell/HudText.cpp src/app/SkillPanel.cpp)
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

add_executable(test_runtimehud src/__test__/test_runtimehud.cpp src/app/RuntimeHud.cpp src/app/SimClock.cpp src/app/LevelEndTransition.cpp src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp)
target_link_libraries(test_runtimehud PRIVATE webolution_objects)
add_test(NAME runtimehud COMMAND test_runtimehud)

add_executable(test_levelend src/__test__/test_levelend.cpp src/app/LevelEndTransition.cpp)
target_include_directories(test_levelend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME levelend COMMAND test_levelend)

add_executable(test_entrystartcue src/__test__/test_entrystartcue.cpp src/app/EntryStartCue.cpp src/shell/MenuRender.cpp)
target_include_directories(test_entrystartcue PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME entrystartcue COMMAND test_entrystartcue)

add_executable(test_levelmenu src/__test__/test_levelmenu.cpp src/app/LevelMenu.cpp src/shell/MenuRender.cpp)
target_link_libraries(test_levelmenu PRIVATE webolution_objects)
add_test(NAME levelmenu COMMAND test_levelmenu)
set_tests_properties(levelmenu PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

add_executable(test_runtimeskillpanel src/__test__/test_runtimeskillpanel.cpp
  src/app/RuntimeSkillPanel.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_runtimeskillpanel PRIVATE webolution_objects)
add_test(NAME runtimeskillpanel COMMAND test_runtimeskillpanel)

# Exercise the actual SDL loop without creating a desktop window. Compare its
# deterministic presented CPU frame to the separate headless fixture route.
# They share drawing primitives; this is a wiring test, not a native oracle.
if(WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  add_test(NAME runtimewindow COMMAND ${CMAKE_COMMAND}
    "-DAPP=$<TARGET_FILE:webolution>"
    "-DFIXTURE=$<TARGET_FILE:test_hatch_entry_fixture>"
    "-DGAME_DIR=${WEBOLUTION_GAME_DIR}"
    "-DOUTPUT_DIR=${CMAKE_BINARY_DIR}/runtime-window-test"
    -P ${CMAKE_SOURCE_DIR}/src/__test__/runtime_window.cmake)
  set_tests_properties(runtimewindow PROPERTIES TIMEOUT 120)

  # targeted-play-001: separate driver. prepare copies the hash-verified node-0 save retained by playable-expansion-001
  # (run 20261103-003501, step 1) into this task's own run directory; play 1 = MM_BLOCK_001_A (node 3, <=3 Blocker attempts),
  # play 2 = MM_W_012 (node 8, <=2 attempts, only if node 8 became selectable). Outcomes are reported to
  # build/targeted-play-001/runs/<stamp>, never asserted; the exit code reflects driver integrity only.
  # Registered early so its long real-time runs execute inside a bounded verifier window; order of other tests is unchanged.
  # fluid-lever-assessment-001: scripted clone observation of the visible lever-driven fluid on installed MM_W_012 through the
  # public frontend route (Play Current Game -> map click node 8 -> natural play -> results -> map -> reload). Reads a private
  # copy of a retained targeted-play-001 clone save; images/logs go to build/fluid-lever-assessment-001/<variant>. Outcomes are
  # reported, never asserted; exit code = driver integrity (route ran, artifacts written, normal save untouched).
  set(FLA_DIR "${CMAKE_SOURCE_DIR}/build/fluid-lever-assessment-001")
  add_executable(test_fluidlever_assessment src/__test__/test_fluidlever_assessment.cpp)
  target_link_libraries(test_fluidlever_assessment PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_fluidlever_assessment PRIVATE webolution_objects)
  target_compile_definitions(test_fluidlever_assessment PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 FLA_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_fluidlever_assessment PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_fluidlever_assessment PRIVATE PkgConfig::SDL2)
  endif()
  set(FLA_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set(FLA_PREV "")
  foreach(variant IN ITEMS stable-camera ordinary-camera)
    add_test(NAME fluidlever_play_${variant} COMMAND test_fluidlever_assessment play ${variant} "${FLA_DIR}")
    set_tests_properties(fluidlever_play_${variant} PROPERTIES TIMEOUT 900 ENVIRONMENT "${FLA_ENV}")
    add_test(NAME fluidlever_restart_${variant} COMMAND test_fluidlever_assessment restart ${variant} "${FLA_DIR}")
    set_tests_properties(fluidlever_restart_${variant} PROPERTIES TIMEOUT 300 ENVIRONMENT "${FLA_ENV}" DEPENDS fluidlever_play_${variant})
  endforeach()

  set(TP_DIR "${CMAKE_SOURCE_DIR}/build/targeted-play-001")
  add_executable(test_targetedplay src/__test__/test_targetedplay.cpp)
  target_link_libraries(test_targetedplay PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_targetedplay PRIVATE webolution_objects)
  target_compile_definitions(test_targetedplay PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 TP_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_targetedplay PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_targetedplay PRIVATE PkgConfig::SDL2)
  endif()
  set(TP_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  # test-parallel-isolation-001: fluidlever_play_* scans ${TP_DIR}/runs/*/save at start (findSourceSave) for
  # a retained save with a known FNV, newest first. The serial registration order ran both fluid plays
  # before this chain, so they only ever saw completed earlier runs. DEPENDS keeps that order under
  # parallel scheduling, so no in-flight run (created by prepare, rewritten by play 1/2) can be selected or
  # copied mid-update. The fluid variants stay parallel with each other and with every other group.
  add_test(NAME targetedplay_prepare COMMAND test_targetedplay prepare 0 "${TP_DIR}")
  set_tests_properties(targetedplay_prepare PROPERTIES TIMEOUT 120 ENVIRONMENT "${TP_ENV}"
    DEPENDS "fluidlever_play_stable-camera;fluidlever_play_ordinary-camera")
  set(TP_PREV targetedplay_prepare)
  foreach(step IN ITEMS 1 2)
    add_test(NAME targetedplay_play_${step} COMMAND test_targetedplay play ${step} "${TP_DIR}")
    set_tests_properties(targetedplay_play_${step} PROPERTIES TIMEOUT 1800 ENVIRONMENT "${TP_ENV}" DEPENDS ${TP_PREV})
    add_test(NAME targetedplay_restart_${step} COMMAND test_targetedplay restart ${step} "${TP_DIR}")
    set_tests_properties(targetedplay_restart_${step} PROPERTIES TIMEOUT 300 ENVIRONMENT "${TP_ENV}" DEPENDS targetedplay_play_${step})
    set(TP_PREV targetedplay_restart_${step})
  endforeach()
endif()

# C2o: offline byte-supported cursor selection policy; no SDL or live target.
add_executable(test_cursorpolicy src/__test__/test_cursorpolicy.cpp src/app/CursorPolicy.cpp)
target_include_directories(test_cursorpolicy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME cursorpolicy COMMAND test_cursorpolicy)

add_executable(test_runtimecursor src/__test__/test_runtimecursor.cpp
  src/app/RuntimeCursor.cpp src/app/CursorPolicy.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_runtimecursor PRIVATE webolution_objects)
add_test(NAME runtimecursor COMMAND test_runtimecursor)
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(runtimecursor PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

# B2f: frontend screen sequence replayed from the b2e capture. Frontend.cpp is built
# into the test directly (as test_skillpanel does) so the shared source list stays put.
add_executable(test_frontend_sequence src/__test__/test_frontend_sequence.cpp src/shell/Frontend.cpp)
target_link_libraries(test_frontend_sequence PRIVATE webolution_objects)
add_test(NAME frontend_sequence COMMAND test_frontend_sequence)

add_executable(test_frontendsession src/__test__/test_frontendsession.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendSession.cpp src/app/MapScene.cpp src/app/RuntimeCursor.cpp
  src/app/CursorPolicy.cpp src/app/SkillPanel.cpp src/shell/Frontend.cpp
  src/shell/MenuRender.cpp src/shell/HudText.cpp)
target_link_libraries(test_frontendsession PRIVATE webolution_objects)
add_test(NAME frontendsession COMMAND test_frontendsession)

# save-codec-001: pure SavedGame codec, storage adapter, and the read-only check of
# the node mapping against the reference executable bytes (hash first).
add_executable(test_savedgame src/__test__/test_savedgame.cpp src/app/MapScene.cpp
  src/shell/MenuRender.cpp src/shell/HudText.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_savedgame PRIVATE webolution_objects)
target_compile_definitions(test_savedgame PRIVATE WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME savedgame COMMAND test_savedgame WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# FrontendSession load/save/New Game against retail assets, temp stores only.
add_executable(test_savepersist src/__test__/test_savepersist.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendSession.cpp src/app/MapScene.cpp src/app/RuntimeCursor.cpp
  src/app/CursorPolicy.cpp src/app/SkillPanel.cpp src/shell/Frontend.cpp
  src/shell/MenuRender.cpp src/shell/HudText.cpp)
target_link_libraries(test_savepersist PRIVATE webolution_objects)
add_test(NAME savepersist COMMAND test_savepersist)
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(savepersist PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_savedemo_after src/__test__/test_savedemo.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendSession.cpp src/app/MapScene.cpp src/app/RuntimeCursor.cpp
  src/app/CursorPolicy.cpp src/app/SkillPanel.cpp src/shell/Frontend.cpp
  src/shell/MenuRender.cpp src/shell/HudText.cpp)
target_link_libraries(test_savedemo_after PRIVATE webolution_objects)
target_compile_definitions(test_savedemo_after PRIVATE
  SAVEDEMO_REPORT_DIR="${CMAKE_SOURCE_DIR}/build/save-codec-001")
add_test(NAME savedemo_after COMMAND test_savedemo_after)
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(savedemo_after PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_results src/__test__/test_results.cpp
  src/app/ResultsScene.cpp src/app/FrontendSession.cpp src/app/MapScene.cpp
  src/app/RuntimeCursor.cpp src/app/CursorPolicy.cpp src/app/SkillPanel.cpp
  src/shell/Frontend.cpp src/shell/MenuRender.cpp src/shell/HudText.cpp)
target_link_libraries(test_results PRIVATE webolution_objects)
add_test(NAME results COMMAND test_results)
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(results PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_cameraintro src/__test__/test_cameraintro.cpp
  src/app/Camera.cpp src/app/CameraIntro.cpp)
target_include_directories(test_cameraintro PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME cameraintro COMMAND test_cameraintro)

if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN)
  add_executable(test_frontendwindow src/__test__/test_frontendwindow.cpp
    src/app/ResultsScene.cpp
    src/app/FrontendWindow.cpp src/app/FrontendSession.cpp src/app/MapScene.cpp
    src/app/RuntimeCursor.cpp src/app/CursorPolicy.cpp src/app/SkillPanel.cpp
    src/shell/Frontend.cpp src/shell/MenuRender.cpp src/shell/HudText.cpp)
  target_link_libraries(test_frontendwindow PRIVATE webolution_objects)
  target_compile_definitions(test_frontendwindow PRIVATE WEBOLUTION_HAVE_SDL2=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_frontendwindow PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_frontendwindow PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME frontendwindow COMMAND test_frontendwindow)
  set_tests_properties(frontendwindow PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR};WEBOLUTION_TEST_OUTPUT_DIR=${CMAKE_BINARY_DIR}/frontend-window-test")

  # Exact application path, including the frontend-to-level handoff, driven
  # through the existing frontend observer with owned-process SDL events.
  add_executable(test_frontendentry src/__test__/test_frontendentry.cpp)
  target_link_libraries(test_frontendentry PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_frontendentry PRIVATE webolution_objects)
  target_compile_definitions(test_frontendentry PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_frontendentry PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_frontendentry PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME frontendentry COMMAND test_frontendentry "${CMAKE_BINARY_DIR}/frontend-entry-test")
  set_tests_properties(frontendentry PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # P14: installed map/title/font assets, public SDL motion in runApplication.
  add_executable(test_maphover src/__test__/test_maphover.cpp)
  target_link_libraries(test_maphover PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_maphover PRIVATE webolution_objects)
  target_compile_definitions(test_maphover PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  add_test(NAME maphover COMMAND test_maphover after "${CMAKE_BINARY_DIR}/map-hover-test")
  set_tests_properties(maphover PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # Observe real SDL window lifetimes in the source application, including teardown.
  if(MINGW OR (UNIX AND NOT APPLE))
    add_executable(test_windowscale src/__test__/test_windowscale.cpp)
    target_link_libraries(test_windowscale PRIVATE webolution_app webolution_app_nomain webolution_objects)
    target_compile_definitions(test_windowscale PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
    target_link_options(test_windowscale PRIVATE "-Wl,--wrap=SDL_CreateWindow" "-Wl,--wrap=SDL_RenderPresent")
    add_test(NAME windowscale COMMAND test_windowscale after "${CMAKE_BINARY_DIR}/window-scale-test")
    set_tests_properties(windowscale PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    foreach(factor IN ITEMS 1 2)
      add_test(NAME windowscale_frames_${factor}x COMMAND test_windowscale "bench${factor}"
        "${CMAKE_BINARY_DIR}/window-scale-test")
      set_tests_properties(windowscale_frames_${factor}x PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endforeach()

    add_executable(test_frontendwindowquit src/__test__/test_frontendwindowquit.cpp)
    target_link_libraries(test_frontendwindowquit PRIVATE webolution_app webolution_app_nomain)
    target_link_libraries(test_frontendwindowquit PRIVATE webolution_objects)
    target_compile_definitions(test_frontendwindowquit PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
    target_link_options(test_frontendwindowquit PRIVATE
      "-Wl,--wrap=SDL_CreateWindow" "-Wl,--wrap=SDL_DestroyWindow")
    add_test(NAME frontendwindowquit COMMAND test_frontendwindowquit after "${CMAKE_BINARY_DIR}/frontend-window-quit-test")
    set_tests_properties(frontendwindowquit PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  endif()

  add_executable(test_levelinteraction src/__test__/test_levelinteraction.cpp)
  target_link_libraries(test_levelinteraction PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_levelinteraction PRIVATE webolution_objects)
  target_compile_definitions(test_levelinteraction PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelinteraction PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelinteraction PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME levelinteraction COMMAND test_levelinteraction)
  set_tests_properties(levelinteraction PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # basher-fix-001: the Basher at the wall on the installed MM_BASH_002 through runApplication (standalone
  # --level, owned clock, no save store). Reports go under the ignored build/ tree. The probes observe at
  # several distances from the wall and only fail on driver integrity; basherfix asserts the fixed behavior.
  add_executable(test_basherfix src/__test__/test_basherfix.cpp)
  target_link_libraries(test_basherfix PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_basherfix PRIVATE webolution_objects)
  target_compile_definitions(test_basherfix PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_basherfix PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_basherfix PRIVATE PkgConfig::SDL2)
  endif()
  set(BF_DIR "${CMAKE_SOURCE_DIR}/build/basher-fix-001")
  set(BF_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  foreach(gap 1 3)
    add_test(NAME basherfix_probe_gap${gap} COMMAND test_basherfix probe "${BF_DIR}" ${gap})
    set_tests_properties(basherfix_probe_gap${gap} PROPERTIES TIMEOUT 300 ENVIRONMENT "${BF_ENV}")
  endforeach()
  add_test(NAME basherfix COMMAND test_basherfix assert "${BF_DIR}")
  set_tests_properties(basherfix PROPERTIES TIMEOUT 300 ENVIRONMENT "${BF_ENV}")
  add_test(NAME basherfix_away COMMAND test_basherfix away "${BF_DIR}")
  set_tests_properties(basherfix_away PROPERTIES TIMEOUT 300 ENVIRONMENT "${BF_ENV}")

  # teleport-softlock-001: real installed pad visits, natural trace and the sweep's idle horizon.
  add_executable(test_teleportsoftlock src/__test__/test_teleportsoftlock.cpp)
  target_link_libraries(test_teleportsoftlock PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_teleportsoftlock PRIVATE webolution_objects)
  target_compile_definitions(test_teleportsoftlock PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_teleportsoftlock PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_teleportsoftlock PRIVATE PkgConfig::SDL2)
  endif()
  set(TS_DIR "${CMAKE_SOURCE_DIR}/build/teleport-softlock-001/after")
  set(TS_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  foreach(mode trace sweep)
    add_test(NAME teleportsoftlock_${mode} COMMAND test_teleportsoftlock ${mode} "${TS_DIR}/${mode}")
    set_tests_properties(teleportsoftlock_${mode} PROPERTIES TIMEOUT 120 ENVIRONMENT "${TS_ENV}")
  endforeach()
  foreach(level MM_TELE_003 M16_MG_1 M17_MT_1)
    add_test(NAME teleportsoftlock_${level} COMMAND test_teleportsoftlock app "${TS_DIR}/${level}" ${level})
    set_tests_properties(teleportsoftlock_${level} PROPERTIES TIMEOUT 300 ENVIRONMENT "${TS_ENV}")
  endforeach()

  # terrain-render-001: terrain a skill removes or adds reaches the level view. The scene case runs TubeScene on real
  # level data; the app cases drive runApplication with public input twice (normal and --no-terrain-sync) and compare
  # frames. Images go under the ignored build/ tree. Standalone --level runs install no save store.
  add_executable(test_terrainrender src/__test__/test_terrainrender.cpp)
  target_link_libraries(test_terrainrender PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_terrainrender PRIVATE webolution_objects)
  target_compile_definitions(test_terrainrender PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_terrainrender PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_terrainrender PRIVATE PkgConfig::SDL2)
  endif()
  set(TR_DIR "${CMAKE_SOURCE_DIR}/build/terrain-render-001")
  set(TR_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME terrainrender_scene COMMAND test_terrainrender scene "${TR_DIR}")
  set_tests_properties(terrainrender_scene PROPERTIES TIMEOUT 300 ENVIRONMENT "${TR_ENV}")
  add_test(NAME terrainrender_bomb COMMAND test_terrainrender app "${TR_DIR}" MM_BOMB 2 bomb)
  set_tests_properties(terrainrender_bomb PROPERTIES TIMEOUT 300 ENVIRONMENT "${TR_ENV}")
  add_test(NAME terrainrender_build COMMAND test_terrainrender app "${TR_DIR}" MM_BUILD 4 build)
  set_tests_properties(terrainrender_build PROPERTIES TIMEOUT 300 ENVIRONMENT "${TR_ENV}")
  add_test(NAME terrainrender_dig COMMAND test_terrainrender app "${TR_DIR}" MM_DIG_000 7 dig)
  set_tests_properties(terrainrender_dig PROPERTIES TIMEOUT 300 ENVIRONMENT "${TR_ENV}")
  # actor-anim-001: lemmings draw the strip of their own state, advance it with level time and freeze it while
  # paused; a lit bomber fuse shows a countdown. `units` reads the installed lemming sheet; `app` cases drive
  # runApplication with public input twice (normal and --no-actor-anim); `pause` holds the entries under the
  # pause key. Images go under the ignored build/ tree.
  add_executable(test_actoranimstate src/__test__/test_actoranimstate.cpp)
  target_link_libraries(test_actoranimstate PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_actoranimstate PRIVATE webolution_objects)
  target_compile_definitions(test_actoranimstate PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_actoranimstate PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_actoranimstate PRIVATE PkgConfig::SDL2)
  endif()
  set(AA_DIR "${CMAKE_SOURCE_DIR}/build/actor-anim-001")
  set(AA_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME actoranimstate_units COMMAND test_actoranimstate units "${AA_DIR}")
  set_tests_properties(actoranimstate_units PROPERTIES TIMEOUT 300 ENVIRONMENT "${AA_ENV}")
  add_test(NAME actoranimstate_pause COMMAND test_actoranimstate pause "${AA_DIR}")
  set_tests_properties(actoranimstate_pause PROPERTIES TIMEOUT 300 ENVIRONMENT "${AA_ENV}")
  add_test(NAME actoranimstate_climb COMMAND test_actoranimstate app "${AA_DIR}" MM_CLIMB_002 0 climb walker climber climb)
  add_test(NAME actoranimstate_float COMMAND test_actoranimstate app "${AA_DIR}" MM_FLOAT 1 float walker floater float)
  add_test(NAME actoranimstate_bomb COMMAND test_actoranimstate app "${AA_DIR}" MM_BOMB 2 bomb walker exploder explode)
  add_test(NAME actoranimstate_block COMMAND test_actoranimstate app "${AA_DIR}" MM_BLOCK_001_A 3 block walker blocker block)
  add_test(NAME actoranimstate_build COMMAND test_actoranimstate app "${AA_DIR}" MM_BUILD 4 build walker builder build)
  add_test(NAME actoranimstate_bash COMMAND test_actoranimstate app "${AA_DIR}" MM_BASH_002 5 bash walker basher bash)
  set_tests_properties(actoranimstate_climb actoranimstate_float actoranimstate_bomb actoranimstate_block
    actoranimstate_build actoranimstate_bash PROPERTIES TIMEOUT 300 ENVIRONMENT "${AA_ENV}")
  add_executable(test_leveloverlap src/__test__/test_leveloverlap.cpp)
  target_link_libraries(test_leveloverlap PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_leveloverlap PRIVATE webolution_objects)
  target_compile_definitions(test_leveloverlap PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_leveloverlap PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_leveloverlap PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME leveloverlap COMMAND test_leveloverlap)
  set_tests_properties(leveloverlap PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_executable(test_levelvisibility src/__test__/test_levelvisibility.cpp)
  target_link_libraries(test_levelvisibility PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_levelvisibility PRIVATE webolution_objects)
  target_compile_definitions(test_levelvisibility PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 WEBOLUTION_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelvisibility PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelvisibility PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME levelvisibility COMMAND test_levelvisibility "${CMAKE_BINARY_DIR}/level-visibility-test")
  set_tests_properties(levelvisibility PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_executable(test_levelcursor src/__test__/test_levelcursor.cpp)
  target_link_libraries(test_levelcursor PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_levelcursor PRIVATE webolution_objects)
  target_compile_definitions(test_levelcursor PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelcursor PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelcursor PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME levelcursor COMMAND test_levelcursor)
  set_tests_properties(levelcursor PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_executable(test_levelreturn src/__test__/test_levelreturn.cpp)
  target_link_libraries(test_levelreturn PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_levelreturn PRIVATE webolution_objects)
  target_compile_definitions(test_levelreturn PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelreturn PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelreturn PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME levelreturn COMMAND test_levelreturn)
  # Failure guard only. The three natural runs take 190-239 s on this machine and
  # two earlier runs on pre-save sources already hit 240 s; 360 s matches the
  # verifier's own ctest ceiling. Success is still decided by the observers.
  set_tests_properties(levelreturn PROPERTIES TIMEOUT 360
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR};WEBOLUTION_TEST_OUTPUT_DIR=${CMAKE_BINARY_DIR}/levelreturn-test")
  # native-022: sim cue -> retail cue routing, degrade paths and world cues on
  # the real mixer through a dummy SDL audio device (no physical output).
  add_executable(test_cueplayback src/__test__/test_cueplayback.cpp)
  target_link_libraries(test_cueplayback PRIVATE webolution_objects)
  add_test(NAME cueplayback COMMAND test_cueplayback)
  set_tests_properties(cueplayback PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  # Playtest 6 V4: save event multiplicity and exit overlap headroom, no device.
  add_executable(test_exit_sound src/__test__/test_exit_sound.cpp)
  target_link_libraries(test_exit_sound PRIVATE webolution_objects)
  add_test(NAME exit_sound COMMAND test_exit_sound)
  set_tests_properties(exit_sound PROPERTIES TIMEOUT 10)
  # Playtest 9 Y3/Y5: event multiplicity and paused/fast-forward skill audio.
  add_executable(test_gameplay_sounds src/__test__/test_gameplay_sounds.cpp)
  target_link_libraries(test_gameplay_sounds PRIVATE webolution_app webolution_objects)
  add_test(NAME gameplay_sounds COMMAND test_gameplay_sounds)
  set_tests_properties(gameplay_sounds PROPERTIES TIMEOUT 10)
  set(AUDIO_DEMO_SOURCES ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM AUDIO_DEMO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/app/main.cpp")
  add_library(audio_demo_app STATIC ${AUDIO_DEMO_SOURCES}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(audio_demo_app PUBLIC webolution_objects)
  target_compile_definitions(audio_demo_app PUBLIC
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  set(demo_main "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
  if(EXISTS "${demo_main}")
    add_executable(test_audiodemo_after src/__test__/test_audiodemo.cpp)
    target_link_libraries(test_audiodemo_after PRIVATE audio_demo_app webolution_objects)
    target_compile_definitions(test_audiodemo_after PRIVATE
      WEBOLUTION_AUDIO_DEMO_MAIN="${demo_main}"
      WEBOLUTION_AUDIO_DEMO_DIR="${CMAKE_SOURCE_DIR}/build/native-022")
    add_test(NAME audiodemo_after COMMAND test_audiodemo_after)
    set_tests_properties(audiodemo_after PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
  add_executable(test_entrycuewindow src/__test__/test_entrycuewindow.cpp)
  target_link_libraries(test_entrycuewindow PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_entrycuewindow PRIVATE webolution_objects)
  target_compile_definitions(test_entrycuewindow PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_entrycuewindow PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_entrycuewindow PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME entrycuewindow COMMAND test_entrycuewindow)
  set_tests_properties(entrycuewindow PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_executable(test_levelmenuwindow src/__test__/test_levelmenuwindow.cpp)
  target_link_libraries(test_levelmenuwindow PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_levelmenuwindow PRIVATE webolution_objects)
  target_compile_definitions(test_levelmenuwindow PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelmenuwindow PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelmenuwindow PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME levelmenuwindow COMMAND test_levelmenuwindow)
  set_tests_properties(levelmenuwindow PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR};WEBOLUTION_TEST_OUTPUT_DIR=${CMAKE_BINARY_DIR}/levelmenuwindow-test")
  if(DEFINED WEBOLUTION_GAME_DIR)
    add_test(NAME frontend_menu COMMAND ${CMAKE_COMMAND}
      "-DAPP=$<TARGET_FILE:webolution>" "-DDUMP=$<TARGET_FILE:FrontendRenderDump>"
      "-DMENU_DUMP=$<TARGET_FILE:MenuRenderDump>"
      "-DGAME_DIR=${WEBOLUTION_GAME_DIR}" "-DOUTPUT_DIR=${CMAKE_BINARY_DIR}/frontend-menu-test"
      -P ${CMAKE_SOURCE_DIR}/src/__test__/frontend_menu.cmake)
    set_tests_properties(frontend_menu PROPERTIES TIMEOUT 120)
  endif()
endif()

add_executable(test_levelindex src/__test__/test_levelindex.cpp)
target_link_libraries(test_levelindex PRIVATE webolution_objects)
add_test(NAME levelindex COMMAND test_levelindex)

add_executable(test_surface src/__test__/test_surface.cpp)
target_link_libraries(test_surface PRIVATE webolution_objects)
add_test(NAME surface COMMAND test_surface)

add_executable(test_levelobjects src/__test__/test_levelobjects.cpp)
target_link_libraries(test_levelobjects PRIVATE webolution_objects)
add_test(NAME levelobjects COMMAND test_levelobjects)

add_executable(test_teleportlinks_measure src/__test__/test_teleportlinks_measure.cpp)
target_link_libraries(test_teleportlinks_measure PRIVATE webolution_objects)
add_test(NAME teleportlinks_measure COMMAND test_teleportlinks_measure)
set_tests_properties(teleportlinks_measure PROPERTIES TIMEOUT 120)

# lever-links-research-001: read-only measurement of serialized lever links.
add_executable(test_leverlinks_measure src/__test__/test_leverlinks_measure.cpp)
target_link_libraries(test_leverlinks_measure PRIVATE webolution_objects)
add_test(NAME leverlinks_measure COMMAND test_leverlinks_measure)
set_tests_properties(leverlinks_measure PROPERTIES TIMEOUT 300)

# fluid-surface-research-001: read-only dump of the installed MM_W_012 fluid gate records.
add_executable(test_fluidsurface_measure src/__test__/test_fluidsurface_measure.cpp)
target_link_libraries(test_fluidsurface_measure PRIVATE webolution_objects)
add_test(NAME fluidsurface_measure COMMAND test_fluidsurface_measure)
set_tests_properties(fluidsurface_measure PROPERTIES TIMEOUT 120)

# lever-init-research-001: read-only measurement of the original rect painters' cell traversal over the
# installed MM_W_012 gate rectangles (traced bounds, not the earlier geometric enumeration).
add_executable(test_leverinit_measure src/__test__/test_leverinit_measure.cpp)
target_link_libraries(test_leverinit_measure PRIVATE webolution_objects)
add_test(NAME leverinit_measure COMMAND test_leverinit_measure)
set_tests_properties(leverinit_measure PROPERTIES TIMEOUT 300)

add_executable(test_teleportlinks src/__test__/test_teleportlinks.cpp)
target_link_libraries(test_teleportlinks PRIVATE webolution_objects)
add_test(NAME teleportlinks COMMAND test_teleportlinks)
set_tests_properties(teleportlinks PROPERTIES TIMEOUT 120)

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

set(SCENE_ADMISSION_DIR "${CMAKE_SOURCE_DIR}/build/scene-admission-001")
if(DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_sceneadmission_census
    src/__test__/test_sceneadmission_census.cpp
    src/app/SceneBillboardSource.cpp
    src/app/Camera.cpp)
  target_link_libraries(test_sceneadmission_census PRIVATE webolution_objects)
  target_include_directories(test_sceneadmission_census PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_compile_definitions(test_sceneadmission_census PRIVATE SCENE_ADMISSION_DIR="${SCENE_ADMISSION_DIR}")
  add_test(NAME sceneadmission_census COMMAND test_sceneadmission_census)
  set_tests_properties(sceneadmission_census PROPERTIES TIMEOUT 120
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # scene-branch-research-002: read-only measurement of the installed MM_DIG_000
  # Secondary tree (type-2 branch-1 records) and their SPRITES resolution. Writes only
  # under build/scene-branch-research-002; no production reader/renderer is changed.
  add_executable(test_scenebranch_measure src/__test__/test_scenebranch_measure.cpp)
  target_link_libraries(test_scenebranch_measure PRIVATE webolution_objects)
  target_include_directories(test_scenebranch_measure PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_compile_definitions(test_scenebranch_measure PRIVATE
    SCENE_BRANCH_DIR="${CMAKE_SOURCE_DIR}/build/scene-branch-research-002")
  add_test(NAME scenebranch_measure COMMAND test_scenebranch_measure)
  set_tests_properties(scenebranch_measure PROPERTIES TIMEOUT 120
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # scene-nested-research-001: read-only extraction of the nested/non-leaf Secondary trees of
  # the 18 affected installed levels (production readers reused, no production change). Writes
  # only under build/scene-nested-research-001.
  add_executable(test_scenenested_measure
    src/__test__/test_scenenested_measure.cpp
    src/app/SceneBillboardSource.cpp
    src/app/Camera.cpp)
  target_link_libraries(test_scenenested_measure PRIVATE webolution_objects)
  target_include_directories(test_scenenested_measure PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_compile_definitions(test_scenenested_measure PRIVATE
    SCENE_NESTED_DIR="${CMAKE_SOURCE_DIR}/build/scene-nested-research-001")
  add_test(NAME scenenested_measure COMMAND test_scenenested_measure)
  set_tests_properties(scenenested_measure PROPERTIES TIMEOUT 120
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # gameplay-commands-research-001: read-only census of the serialized stock words
  # values_27d0[0..10] of every effective installed level (Armageddon stock = word 9).
  # Writes only under build/gameplay-commands-research-001; no production change.
  add_executable(test_commandstock_census src/__test__/test_commandstock_census.cpp)
  target_link_libraries(test_commandstock_census PRIVATE webolution_objects)
  target_include_directories(test_commandstock_census PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_compile_definitions(test_commandstock_census PRIVATE
    COMMAND_CENSUS_DIR="${CMAKE_SOURCE_DIR}/build/gameplay-commands-research-001")
  add_test(NAME commandstock_census COMMAND test_commandstock_census)
  set_tests_properties(commandstock_census PROPERTIES TIMEOUT 120
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_scenebranch_units src/__test__/test_scenebranch_units.cpp
  src/app/CrtRand.cpp src/app/SceneBillboardSource.cpp)
target_link_libraries(test_scenebranch_units PRIVATE webolution_objects)
target_include_directories(test_scenebranch_units PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenebranch_units COMMAND test_scenebranch_units)
set_tests_properties(scenebranch_units PROPERTIES TIMEOUT 60)

add_executable(test_scenenested_units src/__test__/test_scenenested_units.cpp src/app/SceneDispatch.cpp)
target_include_directories(test_scenenested_units PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenenested_units COMMAND test_scenenested_units)
set_tests_properties(scenenested_units PROPERTIES TIMEOUT 60)

if(WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  set(SCENE_DEMO_DIR "${CMAKE_SOURCE_DIR}/build/scene-integration-001")
  set(SCENE_DEMO_SUPPORT ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM SCENE_DEMO_SUPPORT
    "${CMAKE_SOURCE_DIR}/src/app/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
  add_library(scene_demo_support STATIC ${SCENE_DEMO_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(scene_demo_support PUBLIC webolution_objects)
  target_compile_definitions(scene_demo_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  set(scene_loader "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
  if(EXISTS "${scene_loader}")
    add_executable(test_sceneintegration_after
      src/__test__/test_sceneintegration.cpp "${scene_loader}")
    target_link_libraries(test_sceneintegration_after PRIVATE scene_demo_support webolution_objects)
    target_compile_definitions(test_sceneintegration_after PRIVATE SCENE_DEMO_DIR="${SCENE_DEMO_DIR}")
    target_sources(test_sceneintegration_after PRIVATE src/app/main.cpp)
    add_test(NAME sceneintegration_after COMMAND test_sceneintegration_after)
    set_tests_properties(sceneintegration_after PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()

  set(sa_loader "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
  if(EXISTS "${sa_loader}")
    add_executable(test_sceneadmission_after
      src/__test__/test_sceneadmission.cpp src/__test__/test_sceneadmission_census.cpp "${sa_loader}")
    target_link_libraries(test_sceneadmission_after PRIVATE scene_demo_support webolution_objects)
    target_compile_definitions(test_sceneadmission_after PRIVATE
      SCENE_ADMISSION_DIR="${SCENE_ADMISSION_DIR}" SCENE_ADMISSION_NO_MAIN=1)
    target_sources(test_sceneadmission_after PRIVATE src/app/main.cpp)
    add_test(NAME sceneadmission_after COMMAND test_sceneadmission_after)
    set_tests_properties(sceneadmission_after PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()

  set(SBI_DIR "${CMAKE_SOURCE_DIR}/build/scene-branch-impl-001")
  set(sbi_sources src/app/BackdropScene.cpp src/app/main.cpp
      src/__test__/test_sceneadmission_census.cpp decomp-lemmings/src/texture_creation.cpp)
  add_executable(test_scenebranchimpl_after src/__test__/test_scenebranch_impl.cpp ${sbi_sources})
  target_link_libraries(test_scenebranchimpl_after PRIVATE scene_demo_support webolution_objects)
  target_compile_definitions(test_scenebranchimpl_after PRIVATE SCENE_BRANCH_IMPL_DIR="${SBI_DIR}")
  target_compile_definitions(test_scenebranchimpl_after PRIVATE SCENE_ADMISSION_NO_MAIN=1 SCENE_ADMISSION_DIR="${SCENE_ADMISSION_DIR}")
  target_include_directories(test_scenebranchimpl_after PRIVATE ${CMAKE_SOURCE_DIR}/decomp-lemmings/include)
  add_test(NAME scenebranchimpl_after COMMAND test_scenebranchimpl_after)
  set_tests_properties(scenebranchimpl_after PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(SNI_DIR "${CMAKE_SOURCE_DIR}/build/scene-nested-impl-001")
  set(sni_sources src/app/BackdropScene.cpp src/app/main.cpp
      src/__test__/test_sceneadmission_census.cpp decomp-lemmings/src/texture_creation.cpp)
  add_executable(test_scenenestedimpl_after src/__test__/test_scenenested_impl.cpp ${sni_sources})
  target_link_libraries(test_scenenestedimpl_after PRIVATE scene_demo_support webolution_objects)
  target_compile_definitions(test_scenenestedimpl_after PRIVATE SCENE_NESTED_IMPL_DIR="${SNI_DIR}")
  target_compile_definitions(test_scenenestedimpl_after PRIVATE SCENE_ADMISSION_NO_MAIN=1 SCENE_ADMISSION_DIR="${SCENE_ADMISSION_DIR}")
  target_include_directories(test_scenenestedimpl_after PRIVATE ${CMAKE_SOURCE_DIR}/decomp-lemmings/include)
  add_test(NAME scenenestedimpl_after COMMAND test_scenenestedimpl_after)
  set_tests_properties(scenenestedimpl_after PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(CAMERA_ENTRY_DIR "${CMAKE_SOURCE_DIR}/build/camera-entry-001")
  set(camera_main "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
  if(EXISTS "${camera_main}")
    add_executable(test_cameraentry_after
      src/__test__/test_cameraentry.cpp "${camera_main}" src/app/BackdropScene.cpp)
    target_link_libraries(test_cameraentry_after PRIVATE scene_demo_support webolution_objects)
    target_compile_definitions(test_cameraentry_after PRIVATE
      CAMERA_ENTRY_DIR="${CAMERA_ENTRY_DIR}")
    add_test(NAME cameraentry_after COMMAND test_cameraentry_after)
    set_tests_properties(cameraentry_after PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()

  # native-023: public runApplication --level MM_TELE_003 --sim reaches a World
  # built from the id-paired teleports; ends through observer-queued SDL_QUIT.
  add_executable(test_teleportapp
    src/__test__/test_teleportapp.cpp src/app/main.cpp src/app/BackdropScene.cpp)
  target_link_libraries(test_teleportapp PRIVATE scene_demo_support webolution_objects)
  add_test(NAME teleportapp COMMAND test_teleportapp)
  set_tests_properties(teleportapp PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # fluid-surface-impl-001: installed MM_W_012 base-Fluid surface from live gate state. Source-built
  # offscreen images (production BackdropScene fluid stage + shared-dt World + tube) and the public
  # runApplication route; PNGs and observations are written under build/fluid-surface-impl-001.
  # Depth is the named PROVISIONAL clone policy, never an original-fidelity claim.
  add_executable(test_fluidsurface_impl
    src/__test__/test_fluidsurface_impl.cpp src/app/main.cpp src/app/BackdropScene.cpp)
  target_link_libraries(test_fluidsurface_impl PRIVATE scene_demo_support webolution_objects)
  add_test(NAME fluidsurface_impl COMMAND test_fluidsurface_impl "${CMAKE_SOURCE_DIR}/build/fluid-surface-impl-001")
  set_tests_properties(fluidsurface_impl PROPERTIES TIMEOUT 600
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  add_executable(test_fastforward_units src/__test__/test_fastforward_units.cpp
    src/app/RuntimeHud.cpp src/app/SimClock.cpp src/app/LevelEndTransition.cpp
    src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp
    src/app/RuntimeSkillPanel.cpp src/app/SkillPanel.cpp)
  target_link_libraries(test_fastforward_units PRIVATE webolution_objects)
  add_test(NAME fastforward_units COMMAND test_fastforward_units)
  set_tests_properties(fastforward_units PROPERTIES TIMEOUT 60
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(FFI_DIR "${CMAKE_SOURCE_DIR}/build/fastforward-impl-001")
  set(FFI_SUPPORT ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM FFI_SUPPORT
    "${CMAKE_SOURCE_DIR}/src/app/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/RuntimeHud.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/RuntimeSkillPanel.cpp")
  add_library(fastforward_support STATIC ${FFI_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(fastforward_support PUBLIC webolution_objects)
  target_compile_definitions(fastforward_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  set(HATCH_DIRECTION_DIR "${CMAKE_SOURCE_DIR}/build/hatch-release-direction-001")
  add_executable(test_hatch_direction_after src/__test__/test_hatch_direction.cpp
    src/app/main.cpp src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_hatch_direction_after PRIVATE fastforward_support webolution_objects)
  foreach(level IN ITEMS MM_W_012 MM_CLIMB_002 MM_BASH_002)
    add_test(NAME hatch_direction_${level}_after COMMAND test_hatch_direction_after
      run "${HATCH_DIRECTION_DIR}/after/${level}" "${level}")
    set_tests_properties(hatch_direction_${level}_after PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  # release-rate-control-001: retire legacy frame hashes that omit RATE.
  add_test(NAME hatch_direction_assets COMMAND test_hatch_direction_after assets "${HATCH_DIRECTION_DIR}/assets")
  set_tests_properties(hatch_direction_assets PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set(ffi_files src/app/main.cpp src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  add_executable(test_fastforwardapp_after src/__test__/test_fastforward_app.cpp ${ffi_files})
  target_link_libraries(test_fastforwardapp_after PRIVATE fastforward_support webolution_objects)
  target_compile_definitions(test_fastforwardapp_after PRIVATE FASTFORWARD_IMPL_DIR="${FFI_DIR}")
  add_test(NAME fastforwardapp_after COMMAND test_fastforwardapp_after)
  set_tests_properties(fastforwardapp_after PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(fft_sources src/app/main.cpp src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  add_executable(test_fastforward_toggle_production src/__test__/test_fastforward_toggle.cpp ${fft_sources})
  target_link_libraries(test_fastforward_toggle_production PRIVATE fastforward_support webolution_objects)
  add_test(NAME fastforward_toggle_production COMMAND test_fastforward_toggle_production production
    "${CMAKE_CURRENT_BINARY_DIR}/fastforward-toggle-production")
  set_tests_properties(fastforward_toggle_production PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(MOTION_COMPARE "-")
  set(motion_main src/app/main.cpp)
  add_executable(test_actor_motion_production src/__test__/test_actor_motion.cpp
    ${motion_main} src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_actor_motion_production PRIVATE fastforward_support webolution_objects)
  add_test(NAME actor_motion_production COMMAND test_actor_motion_production production
    "${CMAKE_CURRENT_BINARY_DIR}/actor-motion-production"
    "${MOTION_COMPARE}")
  set_tests_properties(actor_motion_production PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME actor_motion_wall_production COMMAND test_actor_motion_production wall-production
    "${CMAKE_CURRENT_BINARY_DIR}/actor-motion-wall-production")
  set_tests_properties(actor_motion_wall_production PROPERTIES TIMEOUT 60
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME actor_motion_units COMMAND test_actor_motion_production units
    "${CMAKE_CURRENT_BINARY_DIR}/actor-motion-units")
  set_tests_properties(actor_motion_units PROPERTIES TIMEOUT 60)

  set(HOVER_EVIDENCE "${CMAKE_SOURCE_DIR}/build/hover-feedback-001")
  set(hover_main src/app/main.cpp)
  add_executable(test_hover_feedback_after src/__test__/test_hover_feedback.cpp ${hover_main}
    src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_hover_feedback_after PRIVATE fastforward_support webolution_objects)
  foreach(route IN ITEMS actor hatch)
    add_test(NAME hover_feedback_${route}_after COMMAND test_hover_feedback_after ${route}
      "${HOVER_EVIDENCE}/after/${route}")
    set_tests_properties(hover_feedback_${route}_after PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  add_test(NAME hover_feedback_units COMMAND test_hover_feedback_after units)
  set_tests_properties(hover_feedback_units PROPERTIES TIMEOUT 30)
  add_test(NAME hover_feedback_reference COMMAND ${CMAKE_COMMAND}
    "-DGAME_DIR=${WEBOLUTION_GAME_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/hover_feedback_reference.cmake")
  set_tests_properties(hover_feedback_reference PROPERTIES TIMEOUT 30)
  set(timing_main src/app/main.cpp)
  add_executable(test_hatch_timing_after src/__test__/test_hatch_timing.cpp ${timing_main}
    src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_hatch_timing_after PRIVATE fastforward_support webolution_objects)
  # letsgo-text-nuke-ohno-001: restore the retired load-time text checks.
  add_test(NAME hatch_timing_after COMMAND test_hatch_timing_after entry
    "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/after")
  add_test(NAME hatch_timing_win_after COMMAND test_hatch_timing_after win
    "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/win-after")
  add_test(NAME hatch_timing_routes_after COMMAND test_hatch_timing_after routes
    "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/routes-after")
  # The routes take about 145 s alone against the unchanged 180 s limit; sharing the
  # machine with three other play tests has pushed them past it, so they run alone.
  set_tests_properties(hatch_timing_routes_after PROPERTIES TIMEOUT 180 RUN_SERIAL TRUE
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(hatch_timing_after hatch_timing_win_after PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME hatch_timing_units COMMAND test_hatch_timing_after units "${CMAKE_CURRENT_BINARY_DIR}/hatch-timing-units")
  add_test(NAME hatch_timing_fast COMMAND test_hatch_timing_after fast "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/fast")
  add_test(NAME hatch_timing_assets COMMAND test_hatch_timing_after assets "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/assets")
  add_test(NAME hatch_timing_retry COMMAND test_hatch_timing_after retry "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/retry")
  set_tests_properties(hatch_timing_units hatch_timing_fast hatch_timing_assets hatch_timing_retry PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set(HATCH_EXIT_SUPPORT ${FFI_SUPPORT})
  list(REMOVE_ITEM HATCH_EXIT_SUPPORT "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
  add_library(hatch_exit_support STATIC ${HATCH_EXIT_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(hatch_exit_support PUBLIC webolution_objects)
  target_compile_definitions(hatch_exit_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  set(balloon_main src/app/main.cpp)
  set(balloon_scene src/app/BackdropScene.cpp)
  add_executable(test_exit_balloon_after src/__test__/test_exit_balloon.cpp
    ${balloon_main} ${balloon_scene} src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_exit_balloon_after PRIVATE hatch_exit_support webolution_objects)
  add_test(NAME exit_balloon_after COMMAND test_exit_balloon_after
    "${CMAKE_SOURCE_DIR}/build/exit-balloon-path-001/after")
  set_tests_properties(exit_balloon_after PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME exit_balloon_assets COMMAND test_exit_balloon_after
    "${CMAKE_SOURCE_DIR}/build/exit-balloon-path-001/assets" assets)
  set_tests_properties(exit_balloon_assets PROPERTIES TIMEOUT 60
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  # release-rate-control-001: legacy exit_balloon_compare omits RATE.
  add_executable(test_hatch_entry_fixture src/__test__/test_hatch_entry_fixture.cpp
    src/app/BackdropScene.cpp src/app/RuntimeHud.cpp)
  target_link_libraries(test_hatch_entry_fixture PRIVATE hatch_exit_support webolution_objects)
  set(hatch_main src/app/main.cpp)
  set(hatch_scene src/app/BackdropScene.cpp)
  add_executable(test_hatch_exit_after src/__test__/test_hatch_exit.cpp ${hatch_main} ${hatch_scene}
    src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_hatch_exit_after PRIVATE hatch_exit_support webolution_objects)
  add_test(NAME hatch_exit_app_after COMMAND test_hatch_exit_after app
    "${CMAKE_SOURCE_DIR}/build/hatch-exit-presentation-001/after")
  add_test(NAME hatch_exit_end_after COMMAND test_hatch_exit_after end
    "${CMAKE_SOURCE_DIR}/build/hatch-exit-presentation-001/end-after")
  set_tests_properties(hatch_exit_app_after hatch_exit_end_after PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME hatch_exit_assets COMMAND test_hatch_exit_after assets "${CMAKE_CURRENT_BINARY_DIR}/hatch-exit-assets")
  set_tests_properties(hatch_exit_assets PROPERTIES TIMEOUT 60
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME hatch_exit_models COMMAND test_hatch_exit_after models "${CMAKE_SOURCE_DIR}/build/hatch-exit-presentation-001/models")
  set_tests_properties(hatch_exit_models PROPERTIES TIMEOUT 60
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(OCCLUSION_SUPPORT ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM OCCLUSION_SUPPORT "${CMAKE_SOURCE_DIR}/src/app/TubeScene.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
  add_library(actor_occlusion_support STATIC ${OCCLUSION_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(actor_occlusion_support PUBLIC webolution_objects)
  target_compile_definitions(actor_occlusion_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  # basher-occlusion-001: retire legacy actor_occlusion_after's flat biased depth oracle.

  add_test(NAME actor_depth_reference COMMAND ${CMAKE_COMMAND}
    "-DGAME_DIR=${WEBOLUTION_GAME_DIR}"
    -P "${CMAKE_SOURCE_DIR}/cmake/actor_depth_reference.cmake")
  set_tests_properties(actor_depth_reference PROPERTIES TIMEOUT 30)
  # basher-occlusion-001: retire legacy actor_depth_after's four-unit contact allowance.

  # playable-baseline-001: scripted public-input assessment of three installed levels. Outcomes
  # (win/loss/watchdog) are reported to build/playable-baseline-001, never asserted; the exit code
  # reflects driver integrity only. Isolated FileSaveSink under that directory; no installed save.
  set(PB_DIR "${CMAKE_SOURCE_DIR}/build/playable-baseline-001")
  add_executable(test_playablebaseline src/__test__/test_playablebaseline.cpp)
  target_link_libraries(test_playablebaseline PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_playablebaseline PRIVATE webolution_objects)
  target_compile_definitions(test_playablebaseline PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 PB_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_playablebaseline PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_playablebaseline PRIVATE PkgConfig::SDL2)
  endif()
  set(PB_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME playablebaseline_survey COMMAND test_playablebaseline survey 0 "${PB_DIR}")
  set_tests_properties(playablebaseline_survey PROPERTIES TIMEOUT 300 ENVIRONMENT "${PB_ENV}")
  foreach(route IN ITEMS 1 2 3)
    add_test(NAME playablebaseline_play_${route} COMMAND test_playablebaseline play ${route} "${PB_DIR}")
    set_tests_properties(playablebaseline_play_${route} PROPERTIES TIMEOUT 1800 ENVIRONMENT "${PB_ENV}"
      DEPENDS playablebaseline_survey)
    add_test(NAME playablebaseline_restart_${route} COMMAND test_playablebaseline restart ${route} "${PB_DIR}")
    set_tests_properties(playablebaseline_restart_${route} PROPERTIES TIMEOUT 300 ENVIRONMENT "${PB_ENV}"
      DEPENDS playablebaseline_play_${route})
  endforeach()

  # level-sweep-001: direct-entry, no-input smoke sweep of all 102 map nodes, one CHILD PROCESS per level so a crash
  # or hang is isolated and named. 17 chunks of 6 nodes; each chunk owns build/level-sweep-001/chunk-NNN (child
  # records, logs, frames); the report test aggregates them. The tests assert driver integrity only (every node
  # visited, a result record written, sheets written); a level's outcome is reported, never asserted. No save store is
  # supplied to runApplication, so no save is read or written by the sweep.
  set(LS_DIR "${CMAKE_SOURCE_DIR}/build/level-sweep-001")
  add_executable(test_levelsweep src/__test__/test_levelsweep.cpp)
  target_link_libraries(test_levelsweep PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_levelsweep PRIVATE webolution_objects)
  target_compile_definitions(test_levelsweep PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 LS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelsweep PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelsweep PRIVATE PkgConfig::SDL2)
  endif()
  set(LS_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set(LS_CHUNK_TESTS "")
  foreach(chunk RANGE 0 16)
    string(LENGTH "${chunk}" LS_LEN)
    if(LS_LEN EQUAL 1)
      set(LS_NAME "0${chunk}")
  else()
      set(LS_NAME "${chunk}")
    endif()
    add_test(NAME levelsweep_chunk_${LS_NAME} COMMAND test_levelsweep chunk ${chunk} "${LS_DIR}")
    set_tests_properties(levelsweep_chunk_${LS_NAME} PROPERTIES TIMEOUT 1500 ENVIRONMENT "${LS_ENV}")
    list(APPEND LS_CHUNK_TESTS levelsweep_chunk_${LS_NAME})
  endforeach()
  add_test(NAME levelsweep_report COMMAND test_levelsweep report 0 "${LS_DIR}")
  set_tests_properties(levelsweep_report PROPERTIES TIMEOUT 300 ENVIRONMENT "${LS_ENV}" DEPENDS "${LS_CHUNK_TESTS}")

  # playable-expansion-001: scripted public-input assessment of Blocker (node 3), Builder (node 4) and the nearest
  # selectable gadget level, over ONE isolated save earned from fresh by step 1 (node 0 win). Outcomes (win/loss/
  # blocked/watchdog) are reported to build/playable-expansion-001/runs/<stamp>, never asserted; the exit code reflects
  # driver integrity only. Order is play N -> restart N -> play N+1 so each restart sees the save as play N left it.
  set(PE_DIR "${CMAKE_SOURCE_DIR}/build/playable-expansion-001")
  add_executable(test_playableexpansion src/__test__/test_playableexpansion.cpp)
  target_link_libraries(test_playableexpansion PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_playableexpansion PRIVATE webolution_objects)
  target_compile_definitions(test_playableexpansion PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 PE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_playableexpansion PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_playableexpansion PRIVATE PkgConfig::SDL2)
  endif()
  set(PE_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME playableexpansion_survey COMMAND test_playableexpansion survey 0 "${PE_DIR}")
  set_tests_properties(playableexpansion_survey PROPERTIES TIMEOUT 300 ENVIRONMENT "${PE_ENV}")
  set(PE_PREV playableexpansion_survey)
  foreach(step IN ITEMS 1 2 3 4)
    add_test(NAME playableexpansion_play_${step} COMMAND test_playableexpansion play ${step} "${PE_DIR}")
    set_tests_properties(playableexpansion_play_${step} PROPERTIES TIMEOUT 1800 ENVIRONMENT "${PE_ENV}"
      DEPENDS ${PE_PREV})
    add_test(NAME playableexpansion_restart_${step} COMMAND test_playableexpansion restart ${step} "${PE_DIR}")
    set_tests_properties(playableexpansion_restart_${step} PROPERTIES TIMEOUT 300 ENVIRONMENT "${PE_ENV}"
      DEPENDS playableexpansion_play_${step})
    set(PE_PREV playableexpansion_restart_${step})
  endforeach()

  set(LSC_DIR "${CMAKE_SOURCE_DIR}/build/level-selection-case-001")
  set(LSC_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  add_executable(test_levelselection_after src/__test__/test_levelselection.cpp)
  target_link_libraries(test_levelselection_after PRIVATE webolution_app webolution_app_nomain)
  target_link_libraries(test_levelselection_after PRIVATE webolution_objects)
  target_compile_definitions(test_levelselection_after PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelselection_after PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelselection_after PRIVATE PkgConfig::SDL2)
  endif()

  foreach(node IN ITEMS 0 9 11 21)
    add_test(NAME levelselection_after_node${node}
      COMMAND test_levelselection_after frontend ${node} "${LSC_DIR}/evidence")
    set_tests_properties(levelselection_after_node${node} PROPERTIES TIMEOUT 300 ENVIRONMENT "${LSC_ENV}")
  endforeach()
  add_test(NAME levelselection_census COMMAND test_levelselection_after census "${LSC_DIR}/evidence")
  set_tests_properties(levelselection_census PROPERTIES TIMEOUT 300 ENVIRONMENT "${LSC_ENV}")
endif()

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

add_executable(test_runtimepick src/__test__/test_runtimepick.cpp
  src/app/RuntimePick.cpp src/app/Pick.cpp)
target_include_directories(test_runtimepick PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_runtimepick PRIVATE webolution_objects)
add_test(NAME runtimepick COMMAND test_runtimepick)

add_executable(test_actorvisibility src/__test__/test_actorvisibility.cpp src/app/ActorVisibility.cpp)
target_include_directories(test_actorvisibility PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME actorvisibility COMMAND test_actorvisibility)

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
  src/app/ActorAnim.cpp
  src/app/LemmingAnim.cpp
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
  set_tests_properties(teleportlinks PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(teleportlinks_measure PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(leverlinks_measure PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(fluidsurface_measure PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(leverinit_measure PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(runtimeskillpanel frontendsession PROPERTIES
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(N023_DIR "${CMAKE_SOURCE_DIR}/build/native-023")
  set(n023_lib webolution_objects)
  add_executable(test_teleportdemo_after src/__test__/test_teleportdemo.cpp)
  target_link_libraries(test_teleportdemo_after PRIVATE ${n023_lib})
  target_compile_definitions(test_teleportdemo_after PRIVATE
    TELEPORTDEMO_OUT_DIR="${N023_DIR}")
  add_test(NAME teleportdemo_after COMMAND test_teleportdemo_after)
  set_tests_properties(teleportdemo_after PROPERTIES TIMEOUT 120
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

include(cmake/walker_lane.cmake)
include(cmake/fluid_kinds.cmake)
include(cmake/fluid_hazard.cmake)
include(cmake/lever_render.cmake)
include(cmake/panel_icon_anim.cmake)
include(cmake/antigrav_presentation.cmake)
include(cmake/empty_receiver.cmake)
include(cmake/climber_occlusion.cmake)
include(cmake/lemming_type_sprites.cmake)
include(cmake/frame_rate_profile.cmake)
include(cmake/gravity_gadgets.cmake)

webolution_share_registered_app_sources()

include(cmake/frame_consumption.cmake)
include(cmake/antigrav_skill_assign.cmake)
include(cmake/pick_nearest.cmake)
include(cmake/nuke.cmake)
include(cmake/release_rate.cmake)

if(BUILD_TESTING AND NOT EMSCRIPTEN)
  # Player recordings are regression evidence, not original fidelity evidence.
  # test_tiers.cmake leaves this corpus check in legacy; an empty corpus passes.
  add_executable(test_playtest_replays src/__test__/test_playtest_replays.cpp)
  target_link_libraries(test_playtest_replays PRIVATE webolution_app webolution_objects)
  add_test(NAME playtest_replays COMMAND test_playtest_replays
    "${CMAKE_SOURCE_DIR}/docs/playtest/replays" "${WEBOLUTION_GAME_DIR}")
endif()

# Keep last: assigns the smoke/legacy tiers to every test registered above.
if(BUILD_TESTING)
  # Small depth/footprint invariants; stays legacy, no gameplay route.
  add_executable(test_actor_depth src/__test__/test_actor_depth.cpp)
  target_link_libraries(test_actor_depth PRIVATE webolution_app webolution_objects)
  add_test(NAME actor_depth COMMAND test_actor_depth "${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_death_particles src/__test__/test_death_particles.cpp)
target_link_libraries(test_death_particles PRIVATE webolution_objects)
add_test(NAME death_particles COMMAND test_death_particles)

add_executable(test_terrain_debris src/__test__/test_terrain_debris.cpp)
target_link_libraries(test_terrain_debris PRIVATE webolution_app webolution_objects)
add_test(NAME terrain_debris COMMAND test_terrain_debris)

add_executable(test_crab src/__test__/test_crab.cpp)
target_link_libraries(test_crab PRIVATE webolution_app webolution_objects)
add_test(NAME crab COMMAND test_crab)
if(WEBOLUTION_GAME_DIR)
  set_tests_properties(crab PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

add_executable(test_namedhazards src/__test__/test_namedhazards.cpp)
target_link_libraries(test_namedhazards PRIVATE webolution_app webolution_objects)
add_test(NAME namedhazards COMMAND test_namedhazards)
if(WEBOLUTION_GAME_DIR)
  set_tests_properties(namedhazards PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

if(BUILD_TESTING)
  add_executable(test_shrug_assign src/__test__/test_shrug_assign.cpp)
  target_link_libraries(test_shrug_assign PRIVATE webolution_app webolution_objects)
  add_test(NAME shrug_assign COMMAND test_shrug_assign)
endif()

if(BUILD_TESTING)
  add_executable(test_builder_brick src/__test__/test_builder_brick.cpp)
  target_link_libraries(test_builder_brick PRIVATE webolution_app webolution_objects)
  add_test(NAME builder_brick COMMAND test_builder_brick "${WEBOLUTION_GAME_DIR}")
endif()

if(BUILD_TESTING)
  # Small boundary unit; left in the legacy tier, not a new route/replay.
  add_executable(test_fall_out src/__test__/test_fall_out.cpp)
  target_link_libraries(test_fall_out PRIVATE webolution_objects)
  add_test(NAME fall_out COMMAND test_fall_out)
endif()

if(BUILD_TESTING)
  # Short scheduler/quota invariants and installed-data census; stays legacy.
  add_executable(test_multi_hatch_simultaneous src/__test__/test_multi_hatch_simultaneous.cpp)
  target_link_libraries(test_multi_hatch_simultaneous PRIVATE webolution_app webolution_objects)
  add_test(NAME multi_hatch_simultaneous COMMAND test_multi_hatch_simultaneous "${WEBOLUTION_GAME_DIR}")
endif()

include(cmake/test_tiers.cmake)
