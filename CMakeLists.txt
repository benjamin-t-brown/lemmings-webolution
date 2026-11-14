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

# shared-timing-impl-001: legacy-path baseline/wrapper driver for the MM_W_012 lever route.
# Registered first so a client time limit on a long suite cannot cut it off. It uses only the
# pre-existing RuntimeSimState::advance/World::tick API, so one source records the pre-edit
# baseline and then guards the documented compatibility wrapper.
add_executable(test_sharedtiming_baseline src/__test__/test_sharedtiming_baseline.cpp
  src/app/RuntimeHud.cpp src/app/SimClock.cpp src/app/LevelEndTransition.cpp
  src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp
  src/app/RuntimeSkillPanel.cpp src/app/SkillPanel.cpp)
target_link_libraries(test_sharedtiming_baseline PRIVATE webolution_objects)
add_test(NAME sharedtiming_baseline
  COMMAND test_sharedtiming_baseline "${CMAKE_SOURCE_DIR}/build/shared-timing-impl-001/legacy-wrapper-observations.txt")
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(sharedtiming_baseline PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()

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

# lever-gates-impl-001: the genuine pre-edit baseline driver src/__test__/test_levergates_baseline.cpp
# (legacy-visible API only) was registered as `levergates_baseline` and PASSED against the UNEDITED
# production sources (verify.py --profile main, before any production edit); its output is retained as
# build/lever-gates-impl-001/baseline-before-edit.txt. Its assertions describe the pre-edit behaviour
# (3600 stored cells, pulls change nothing), which the integration deliberately changes, so it is no
# longer registered; the source and the retained output are the record.
# After the integration: public loader + shared elapsed path, independent reference recurrence.
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

# lever-initial-face-impl-001: the genuine pre-edit driver src/__test__/test_leverinitface_baseline.cpp
# (public loader + shared elapsed path only, observation only) was registered as `leverinitface_baseline`
# and run by verify.py --profile main against the UNEDITED production sources; its output is retained as
# build/lever-initial-face-impl-001/baseline-before-edit.txt. It records the pre-edit wrong default, so it
# is no longer registered (registering it would overwrite that retained evidence); the source and the
# output are the record.
# After the change: independent serialized records, all four levers, first pull/repeat/reload, the
# ids 5/6 ramp, once-only load fan-out and unsupported-face fixtures through the unchanged loader.
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
  add_executable(test_fluidlever_assessment src/__test__/test_fluidlever_assessment.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_targetedplay src/__test__/test_targetedplay.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
add_executable(test_savedgame src/__test__/test_savedgame.cpp src/app/MapScene.cpp)
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

# save-codec-001 before/after restart demonstration. One driver, two builds: "after" uses
# the current FrontendSession.cpp with a temp-directory store; "before" is built only when
# the frozen pre-change FrontendSession.{h,cpp} exist under the ignored
# build/save-codec-001/frozen/ (they are never tracked), so a clean checkout skips it.
add_executable(test_savedemo_after src/__test__/test_savedemo.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendSession.cpp src/app/MapScene.cpp src/app/RuntimeCursor.cpp
  src/app/CursorPolicy.cpp src/app/SkillPanel.cpp src/shell/Frontend.cpp
  src/shell/MenuRender.cpp src/shell/HudText.cpp)
target_link_libraries(test_savedemo_after PRIVATE webolution_objects)
target_compile_definitions(test_savedemo_after PRIVATE
  SAVEDEMO_REPORT_DIR="${CMAKE_SOURCE_DIR}/build/save-codec-001")
add_test(NAME savedemo_after COMMAND test_savedemo_after)
set(SAVEDEMO_FROZEN_DIR ${CMAKE_SOURCE_DIR}/build/save-codec-001/frozen/app)
if(EXISTS ${SAVEDEMO_FROZEN_DIR}/FrontendSession.cpp AND EXISTS ${SAVEDEMO_FROZEN_DIR}/FrontendSession.h)
  add_executable(test_savedemo_before src/__test__/test_savedemo.cpp
    ${SAVEDEMO_FROZEN_DIR}/FrontendSession.cpp
    src/app/ResultsScene.cpp src/app/MapScene.cpp src/app/RuntimeCursor.cpp
    src/app/CursorPolicy.cpp src/app/SkillPanel.cpp src/shell/Frontend.cpp
    src/shell/MenuRender.cpp src/shell/HudText.cpp)
  target_include_directories(test_savedemo_before BEFORE PRIVATE ${CMAKE_SOURCE_DIR}/build/save-codec-001/frozen)
  target_link_libraries(test_savedemo_before PRIVATE webolution_objects)
  target_compile_definitions(test_savedemo_before PRIVATE SAVEDEMO_BEFORE
    SAVEDEMO_REPORT_DIR="${CMAKE_SOURCE_DIR}/build/save-codec-001")
  add_test(NAME savedemo_before COMMAND test_savedemo_before)
endif()
if(DEFINED WEBOLUTION_GAME_DIR)
  set_tests_properties(savedemo_after PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  if(TARGET test_savedemo_before)
    set_tests_properties(savedemo_before PROPERTIES ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
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
  add_executable(test_frontendentry src/__test__/test_frontendentry.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_maphover src/__test__/test_maphover.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(test_maphover PRIVATE webolution_objects)
  target_compile_definitions(test_maphover PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  add_test(NAME maphover COMMAND test_maphover after "${CMAKE_BINARY_DIR}/map-hover-test")
  set_tests_properties(maphover PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # Authentic task-entry map/session sources, optional and never tracked. The
  # same public-input driver measures the old missing-title behavior.
  set(MAP_HOVER_FROZEN "${CMAKE_SOURCE_DIR}/build/map-level-names-001/before-source")
  if(EXISTS "${MAP_HOVER_FROZEN}/src/app/FrontendSession.cpp" AND
     EXISTS "${MAP_HOVER_FROZEN}/src/app/MapScene.cpp")
    set(MAP_HOVER_BEFORE_GFX ${WEBOLUTION_GFX_SOURCES})
    list(REMOVE_ITEM MAP_HOVER_BEFORE_GFX
      "${CMAKE_SOURCE_DIR}/src/app/FrontendSession.cpp" "${CMAKE_SOURCE_DIR}/src/app/MapScene.cpp")
    add_executable(test_maphover_before src/__test__/test_maphover.cpp
      ${MAP_HOVER_BEFORE_GFX} "${MAP_HOVER_FROZEN}/src/app/FrontendSession.cpp"
      "${MAP_HOVER_FROZEN}/src/app/MapScene.cpp" src/app/SceneVertexPack.cpp
      src/app/SceneDiffuse.cpp src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
    target_include_directories(test_maphover_before BEFORE PRIVATE "${MAP_HOVER_FROZEN}/src")
    target_link_libraries(test_maphover_before PRIVATE webolution_objects)
    target_compile_definitions(test_maphover_before PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
    add_test(NAME maphover_before COMMAND test_maphover_before baseline "${CMAKE_BINARY_DIR}/map-hover-before-test")
    set_tests_properties(maphover_before PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()

  # Observe real SDL window lifetimes in the source application, including teardown.
  if(MINGW OR (UNIX AND NOT APPLE))
    add_executable(test_frontendwindowquit src/__test__/test_frontendwindowquit.cpp
      ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
      src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
    target_link_libraries(test_frontendwindowquit PRIVATE webolution_objects)
    target_compile_definitions(test_frontendwindowquit PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
    target_link_options(test_frontendwindowquit PRIVATE
      "-Wl,--wrap=SDL_CreateWindow" "-Wl,--wrap=SDL_DestroyWindow")
    add_test(NAME frontendwindowquit COMMAND test_frontendwindowquit after "${CMAKE_BINARY_DIR}/frontend-window-quit-test")
    set_tests_properties(frontendwindowquit PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

    # Optional authentic task-entry sources, retained under ignored build/. The
    # same input driver asserts the measured three-window / ineffective-Quit
    # baseline; a clean checkout builds only the production regression above.
    set(FWQ_FROZEN "${CMAKE_SOURCE_DIR}/build/frontend-window-quit-001/before-source")
    if(EXISTS "${FWQ_FROZEN}/manifest.json")
      set(FWQ_BEFORE_GFX)
      foreach(source IN LISTS WEBOLUTION_GFX_SOURCES)
        file(RELATIVE_PATH relative_source "${CMAKE_SOURCE_DIR}" "${source}")
        list(APPEND FWQ_BEFORE_GFX "${FWQ_FROZEN}/${relative_source}")
      endforeach()
      add_executable(test_frontendwindowquit_before src/__test__/test_frontendwindowquit.cpp
        ${FWQ_BEFORE_GFX} "${FWQ_FROZEN}/src/app/SceneVertexPack.cpp"
        "${FWQ_FROZEN}/src/app/SceneDiffuse.cpp" "${FWQ_FROZEN}/src/app/SceneDepth.cpp"
        "${FWQ_FROZEN}/src/app/SceneDispatch.cpp")
      target_include_directories(test_frontendwindowquit_before BEFORE PRIVATE "${FWQ_FROZEN}/src")
      target_link_libraries(test_frontendwindowquit_before PRIVATE webolution_objects)
      target_compile_definitions(test_frontendwindowquit_before PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
      target_link_options(test_frontendwindowquit_before PRIVATE
        "-Wl,--wrap=SDL_CreateWindow" "-Wl,--wrap=SDL_DestroyWindow")
      add_test(NAME frontendwindowquit_before COMMAND test_frontendwindowquit_before baseline
        "${CMAKE_BINARY_DIR}/frontend-window-quit-before-test")
      set_tests_properties(frontendwindowquit_before PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endif()

  add_executable(test_levelinteraction src/__test__/test_levelinteraction.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_basherfix src/__test__/test_basherfix.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_teleportsoftlock src/__test__/test_teleportsoftlock.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_terrainrender src/__test__/test_terrainrender.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_actoranimstate src/__test__/test_actoranimstate.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_leveloverlap src/__test__/test_leveloverlap.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_levelvisibility src/__test__/test_levelvisibility.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_levelcursor src/__test__/test_levelcursor.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_levelreturn src/__test__/test_levelreturn.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  # native-022: ordinary frontend entry -> level with one owned audio device.
  # Failure guard only; success is decided by the observers and SDL_QUIT.
  add_executable(test_audioapp src/__test__/test_audioapp.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(test_audioapp PRIVATE webolution_objects)
  target_compile_definitions(test_audioapp PRIVATE
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_audioapp PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_audioapp PRIVATE PkgConfig::SDL2)
  endif()
  add_test(NAME audioapp COMMAND test_audioapp)
  set_tests_properties(audioapp PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  # native-022: same synthetic installed-terrain scenarios in authentic frozen
  # HEAD main.cpp and current main.cpp. Only the test driver's loader is replaced.
  # The frozen source stays ignored; clean checkouts still run the current side.
  set(AUDIO_DEMO_SOURCES ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM AUDIO_DEMO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/app/main.cpp")
  add_library(audio_demo_app OBJECT ${AUDIO_DEMO_SOURCES}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(audio_demo_app PUBLIC webolution_objects)
  target_compile_definitions(audio_demo_app PUBLIC
    WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  foreach(side IN ITEMS after before)
    if(side STREQUAL "before")
      set(demo_main "${CMAKE_SOURCE_DIR}/build/native-022/frozen-main.cpp")
    else()
      set(demo_main "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
    endif()
    if(EXISTS "${demo_main}")
      add_executable(test_audiodemo_${side} src/__test__/test_audiodemo.cpp)
      target_link_libraries(test_audiodemo_${side} PRIVATE audio_demo_app webolution_objects)
      target_compile_definitions(test_audiodemo_${side} PRIVATE
        WEBOLUTION_AUDIO_DEMO_MAIN="${demo_main}"
        WEBOLUTION_AUDIO_DEMO_DIR="${CMAKE_SOURCE_DIR}/build/native-022")
      if(side STREQUAL "before")
        target_compile_definitions(test_audiodemo_${side} PRIVATE WEBOLUTION_AUDIO_DEMO_BEFORE=1)
      endif()
      add_test(NAME audiodemo_${side} COMMAND test_audiodemo_${side})
      set_tests_properties(audiodemo_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  add_executable(test_entrycuewindow src/__test__/test_entrycuewindow.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_levelmenuwindow src/__test__/test_levelmenuwindow.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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

# scene-admission-001: authentic task-entry BackdropScene.cpp/.h are frozen once into
# the ignored build/scene-admission-001/frozen/ (never swapped into src/) and
# identified by the supervisor entry manifest's SHA-256; the step-1 census is a
# source-built read-only measurement of every effective installed level.
set(SCENE_ADMISSION_DIR "${CMAKE_SOURCE_DIR}/build/scene-admission-001")
include(cmake/sceneadmission_freeze.cmake)
sceneadmission_freeze("${CMAKE_SOURCE_DIR}/src" "${SCENE_ADMISSION_DIR}/frozen")
add_test(NAME sceneadmission_identity COMMAND ${CMAKE_COMMAND}
  "-DFROZEN_DIR=${SCENE_ADMISSION_DIR}/frozen"
  "-DMANIFEST=${CMAKE_SOURCE_DIR}/build/agent-runs/${SA_ENTRY_RUN}/before-source-hashes.json"
  -P ${CMAKE_SOURCE_DIR}/cmake/sceneadmission_identity.cmake)
set_tests_properties(sceneadmission_identity PROPERTIES TIMEOUT 60)
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

# scene-branch-impl-001: asset-free branch-1 formula tests (CRT generator, first-draw latch,
# temporal advance) and the frozen-source identity check against the supervisor entry manifest.
add_executable(test_scenebranch_units src/__test__/test_scenebranch_units.cpp
  src/app/CrtRand.cpp src/app/SceneBillboardSource.cpp)
target_link_libraries(test_scenebranch_units PRIVATE webolution_objects)
target_include_directories(test_scenebranch_units PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenebranch_units COMMAND test_scenebranch_units)
set_tests_properties(scenebranch_units PROPERTIES TIMEOUT 60)
add_test(NAME scenebranchimpl_identity COMMAND ${CMAKE_COMMAND}
  "-DFROZEN_DIR=${CMAKE_SOURCE_DIR}/build/scene-branch-impl-001/frozen"
  "-DMANIFEST=${CMAKE_SOURCE_DIR}/build/agent-runs/20261031T193658-47832-2e53d4d4/before-source-hashes.json"
  -P ${CMAKE_SOURCE_DIR}/cmake/scenebranchimpl_identity.cmake)
set_tests_properties(scenebranchimpl_identity PROPERTIES TIMEOUT 60)

# scene-nested-impl-001: asset-free ordering/traversal checks and the frozen-source identity check
# against the supervisor entry manifest (the frozen copies live in the ignored
# build/scene-nested-impl-001/frozen/ and are never swapped into src/).
add_executable(test_scenenested_units src/__test__/test_scenenested_units.cpp src/app/SceneDispatch.cpp)
target_include_directories(test_scenenested_units PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME scenenested_units COMMAND test_scenenested_units)
set_tests_properties(scenenested_units PROPERTIES TIMEOUT 60)
add_test(NAME scenenestedimpl_identity COMMAND ${CMAKE_COMMAND}
  "-DFROZEN_DIR=${CMAKE_SOURCE_DIR}/build/scene-nested-impl-001/frozen"
  "-DMANIFEST=${CMAKE_SOURCE_DIR}/build/scene-nested-impl-001/entry-manifest.json"
  -P ${CMAKE_SOURCE_DIR}/cmake/scenenestedimpl_identity.cmake)
set_tests_properties(scenenestedimpl_identity PROPERTIES TIMEOUT 60)

# scene-integration-001: compile authentic entry-snapshot and current backdrop
# loaders separately. Never swap production files. Shared support excludes both
# the loader and application entry; only the current side calls runApplication.
if(WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  set(SCENE_DEMO_DIR "${CMAKE_SOURCE_DIR}/build/scene-integration-001")
  set(SCENE_DEMO_SUPPORT ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM SCENE_DEMO_SUPPORT
    "${CMAKE_SOURCE_DIR}/src/app/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
  add_library(scene_demo_support OBJECT ${SCENE_DEMO_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(scene_demo_support PUBLIC webolution_objects)
  target_compile_definitions(scene_demo_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(scene_loader "${SCENE_DEMO_DIR}/BackdropScene.cpp")
    else()
      set(scene_loader "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
    endif()
    if(EXISTS "${scene_loader}")
      add_executable(test_sceneintegration_${side}
        src/__test__/test_sceneintegration.cpp "${scene_loader}")
      target_link_libraries(test_sceneintegration_${side} PRIVATE scene_demo_support webolution_objects)
      target_compile_definitions(test_sceneintegration_${side} PRIVATE SCENE_DEMO_DIR="${SCENE_DEMO_DIR}")
      if(side STREQUAL "before")
        target_include_directories(test_sceneintegration_${side} BEFORE PRIVATE "${SCENE_DEMO_DIR}")
        target_compile_definitions(test_sceneintegration_${side} PRIVATE SCENE_FROZEN=1)
      else()
        target_sources(test_sceneintegration_${side} PRIVATE src/app/main.cpp)
      endif()
      add_test(NAME sceneintegration_${side} COMMAND test_sceneintegration_${side})
      set_tests_properties(sceneintegration_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_sceneintegration_before)
    set_tests_properties(sceneintegration_after PROPERTIES DEPENDS sceneintegration_before)
  endif()

  # scene-admission-001: authentic task-entry BackdropScene.cpp/.h (frozen, identity-checked by
  # sceneadmission_identity) versus the current structural-admission loader, one driver over
  # every effective installed level, separate executables, shared support, no file swapping.
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(sa_loader "${SCENE_ADMISSION_DIR}/frozen/BackdropScene.cpp")
    else()
      set(sa_loader "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
    endif()
    if(EXISTS "${sa_loader}" AND EXISTS "${SCENE_ADMISSION_DIR}/frozen/app/BackdropScene.h")
      add_executable(test_sceneadmission_${side}
        src/__test__/test_sceneadmission.cpp src/__test__/test_sceneadmission_census.cpp "${sa_loader}")
      target_link_libraries(test_sceneadmission_${side} PRIVATE scene_demo_support webolution_objects)
      target_compile_definitions(test_sceneadmission_${side} PRIVATE
        SCENE_ADMISSION_DIR="${SCENE_ADMISSION_DIR}" SCENE_ADMISSION_NO_MAIN=1)
      if(side STREQUAL "before")
        target_include_directories(test_sceneadmission_${side} BEFORE PRIVATE "${SCENE_ADMISSION_DIR}/frozen")
        target_compile_definitions(test_sceneadmission_${side} PRIVATE SCENE_FROZEN=1)
      else()
        target_sources(test_sceneadmission_${side} PRIVATE src/app/main.cpp)
      endif()
      add_test(NAME sceneadmission_${side} COMMAND test_sceneadmission_${side})
      set_tests_properties(sceneadmission_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_sceneadmission_before)
    set_tests_properties(sceneadmission_after PROPERTIES DEPENDS sceneadmission_before)
  endif()

  # scene-branch-impl-001: authentic task-entry SceneNode.{h,cpp}, BackdropScene.{h,cpp},
  # SceneBillboardSource.{h,cpp} and main.cpp were copied (never swapped into src/) to the ignored
  # build/scene-branch-impl-001/frozen/ before any production edit; scenebranchimpl_identity checks
  # them against the supervisor entry manifest. The frozen driver compiles the frozen SceneNode.cpp
  # and BackdropScene.cpp under a renamed readSceneNode so it never links the current parser; the
  # current driver adds the real runApplication. One driver source, two executables.
  set(SBI_DIR "${CMAKE_SOURCE_DIR}/build/scene-branch-impl-001")
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(sbi_sources "${SBI_DIR}/frozen/src/app/BackdropScene.cpp" "${SBI_DIR}/frozen/src/assets/SceneNode.cpp")
    else()
      set(sbi_sources src/app/BackdropScene.cpp src/app/main.cpp
          src/__test__/test_sceneadmission_census.cpp decomp-lemmings/src/texture_creation.cpp)
    endif()
    if(side STREQUAL "after" OR (EXISTS "${SBI_DIR}/frozen/src/app/BackdropScene.cpp" AND EXISTS "${SBI_DIR}/frozen/src/assets/SceneNode.cpp"))
      add_executable(test_scenebranchimpl_${side} src/__test__/test_scenebranch_impl.cpp ${sbi_sources})
      target_link_libraries(test_scenebranchimpl_${side} PRIVATE scene_demo_support webolution_objects)
      target_compile_definitions(test_scenebranchimpl_${side} PRIVATE SCENE_BRANCH_IMPL_DIR="${SBI_DIR}")
      if(side STREQUAL "before")
        target_include_directories(test_scenebranchimpl_${side} BEFORE PRIVATE "${SBI_DIR}/frozen/src")
        target_compile_definitions(test_scenebranchimpl_${side} PRIVATE SCENE_FROZEN=1)
        set_source_files_properties("${SBI_DIR}/frozen/src/app/BackdropScene.cpp" "${SBI_DIR}/frozen/src/assets/SceneNode.cpp"
          PROPERTIES COMPILE_DEFINITIONS "readSceneNode=frozenReadSceneNode")
      else()
        target_compile_definitions(test_scenebranchimpl_${side} PRIVATE SCENE_ADMISSION_NO_MAIN=1 SCENE_ADMISSION_DIR="${SCENE_ADMISSION_DIR}")
        target_include_directories(test_scenebranchimpl_${side} PRIVATE ${CMAKE_SOURCE_DIR}/decomp-lemmings/include)
      endif()
      add_test(NAME scenebranchimpl_${side} COMMAND test_scenebranchimpl_${side})
      set_tests_properties(scenebranchimpl_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_scenebranchimpl_before)
    set_tests_properties(scenebranchimpl_after PROPERTIES DEPENDS scenebranchimpl_before)
  endif()

  # scene-nested-impl-001: authentic task-entry BackdropScene/SceneDispatch/SceneNode (identity-checked by
  # scenenestedimpl_identity) are compiled from the ignored build/scene-nested-impl-001/frozen/ into
  # the "before" driver under renamed entry points (readSceneNode, dispatchSecondaryChildren) so the
  # current parser/dispatch never link into it; the "after" driver adds the real runApplication.
  # One driver source, two executables; no production file is swapped.
  set(SNI_DIR "${CMAKE_SOURCE_DIR}/build/scene-nested-impl-001")
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(sni_sources "${SNI_DIR}/frozen/src/app/BackdropScene.cpp" "${SNI_DIR}/frozen/src/app/SceneDispatch.cpp" "${SNI_DIR}/frozen/src/assets/SceneNode.cpp")
    else()
      set(sni_sources src/app/BackdropScene.cpp src/app/main.cpp
          src/__test__/test_sceneadmission_census.cpp decomp-lemmings/src/texture_creation.cpp)
    endif()
    if(side STREQUAL "after" OR (EXISTS "${SNI_DIR}/frozen/src/app/BackdropScene.cpp" AND EXISTS "${SNI_DIR}/frozen/src/app/SceneDispatch.cpp" AND EXISTS "${SNI_DIR}/frozen/src/assets/SceneNode.cpp"))
      add_executable(test_scenenestedimpl_${side} src/__test__/test_scenenested_impl.cpp ${sni_sources})
      target_link_libraries(test_scenenestedimpl_${side} PRIVATE scene_demo_support webolution_objects)
      target_compile_definitions(test_scenenestedimpl_${side} PRIVATE SCENE_NESTED_IMPL_DIR="${SNI_DIR}")
      if(side STREQUAL "before")
        target_include_directories(test_scenenestedimpl_${side} BEFORE PRIVATE "${SNI_DIR}/frozen/src")
        target_compile_definitions(test_scenenestedimpl_${side} PRIVATE SCENE_FROZEN=1)
        set_source_files_properties("${SNI_DIR}/frozen/src/app/BackdropScene.cpp" "${SNI_DIR}/frozen/src/app/SceneDispatch.cpp" "${SNI_DIR}/frozen/src/assets/SceneNode.cpp"
          PROPERTIES COMPILE_DEFINITIONS "readSceneNode=frozenReadSceneNode;dispatchSecondaryChildren=frozenDispatchSecondaryChildren")
      else()
        target_compile_definitions(test_scenenestedimpl_${side} PRIVATE SCENE_ADMISSION_NO_MAIN=1 SCENE_ADMISSION_DIR="${SCENE_ADMISSION_DIR}")
        target_include_directories(test_scenenestedimpl_${side} PRIVATE ${CMAKE_SOURCE_DIR}/decomp-lemmings/include)
      endif()
      add_test(NAME scenenestedimpl_${side} COMMAND test_scenenestedimpl_${side})
      set_tests_properties(scenenestedimpl_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_scenenestedimpl_before)
    set_tests_properties(scenenestedimpl_after PROPERTIES DEPENDS scenenestedimpl_before)
  endif()

  # camera-entry-001: authentic entry-snapshot main.cpp/AppEntry.h versus current,
  # one driver, separate executables over the shared (scene-integration) support.
  # The frozen files live in the ignored build/camera-entry-001/frozen/ and are
  # never swapped into src/. Failure guards only; success comes from the observers.
  set(CAMERA_ENTRY_DIR "${CMAKE_SOURCE_DIR}/build/camera-entry-001")
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(camera_main "${CAMERA_ENTRY_DIR}/frozen/main.cpp")
    else()
      set(camera_main "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
    endif()
    if(EXISTS "${camera_main}" AND EXISTS "${CAMERA_ENTRY_DIR}/frozen/app/AppEntry.h")
      add_executable(test_cameraentry_${side}
        src/__test__/test_cameraentry.cpp "${camera_main}" src/app/BackdropScene.cpp)
      target_link_libraries(test_cameraentry_${side} PRIVATE scene_demo_support webolution_objects)
      target_compile_definitions(test_cameraentry_${side} PRIVATE
        CAMERA_ENTRY_DIR="${CAMERA_ENTRY_DIR}")
      if(side STREQUAL "before")
        target_include_directories(test_cameraentry_${side} BEFORE PRIVATE "${CAMERA_ENTRY_DIR}/frozen")
        target_compile_definitions(test_cameraentry_${side} PRIVATE CAMERAENTRY_FROZEN=1)
      endif()
      add_test(NAME cameraentry_${side} COMMAND test_cameraentry_${side})
      set_tests_properties(cameraentry_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_cameraentry_before)
    set_tests_properties(cameraentry_after PROPERTIES DEPENDS cameraentry_before)
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

  # fastforward-impl-001: asset-backed unit checks on the installed MM_CLIMB_002 World, plus
  # public runApplication demonstrations. The authentic task-entry main.cpp, RuntimeHud.{h,cpp}
  # and RuntimeSkillPanel.{h,cpp} were copied (never swapped into src/) to the ignored
  # build/fastforward-impl-001/frozen/ before any production edit; fastforwardimpl_identity
  # checks them against the supervisor entry manifest. One driver source, two executables: the
  # "before" side compiles the frozen three files with the frozen headers first on the include
  # path, the "after" side the current ones, over identical shared support.
  add_executable(test_fastforward_units src/__test__/test_fastforward_units.cpp
    src/app/RuntimeHud.cpp src/app/SimClock.cpp src/app/LevelEndTransition.cpp
    src/shell/HudText.cpp src/shell/PauseCaption.cpp src/shell/MenuRender.cpp
    src/app/RuntimeSkillPanel.cpp src/app/SkillPanel.cpp)
  target_link_libraries(test_fastforward_units PRIVATE webolution_objects)
  add_test(NAME fastforward_units COMMAND test_fastforward_units)
  set_tests_properties(fastforward_units PROPERTIES TIMEOUT 60
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  set(FFI_DIR "${CMAKE_SOURCE_DIR}/build/fastforward-impl-001")
  add_test(NAME fastforwardimpl_identity COMMAND ${CMAKE_COMMAND}
    "-DFROZEN_DIR=${FFI_DIR}/frozen"
    "-DMANIFEST=${FFI_DIR}/entry-manifest.json"
    -P ${CMAKE_SOURCE_DIR}/cmake/fastforwardimpl_identity.cmake)
  set_tests_properties(fastforwardimpl_identity PROPERTIES TIMEOUT 60)
  set(FFI_SUPPORT ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM FFI_SUPPORT
    "${CMAKE_SOURCE_DIR}/src/app/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/RuntimeHud.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/RuntimeSkillPanel.cpp")
  add_library(fastforward_support OBJECT ${FFI_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(fastforward_support PUBLIC webolution_objects)
  target_compile_definitions(fastforward_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(ffi_files "${FFI_DIR}/frozen/src/app/main.cpp" "${FFI_DIR}/frozen/src/app/RuntimeHud.cpp"
                    "${FFI_DIR}/frozen/src/app/RuntimeSkillPanel.cpp")
    else()
      set(ffi_files src/app/main.cpp src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
    endif()
    if(side STREQUAL "after" OR (EXISTS "${FFI_DIR}/frozen/src/app/main.cpp" AND
       EXISTS "${FFI_DIR}/frozen/src/app/RuntimeHud.cpp" AND EXISTS "${FFI_DIR}/frozen/src/app/RuntimeHud.h" AND
       EXISTS "${FFI_DIR}/frozen/src/app/RuntimeSkillPanel.cpp" AND EXISTS "${FFI_DIR}/frozen/src/app/RuntimeSkillPanel.h"))
      add_executable(test_fastforwardapp_${side} src/__test__/test_fastforward_app.cpp ${ffi_files})
      target_link_libraries(test_fastforwardapp_${side} PRIVATE fastforward_support webolution_objects)
      target_compile_definitions(test_fastforwardapp_${side} PRIVATE FASTFORWARD_IMPL_DIR="${FFI_DIR}")
      if(side STREQUAL "before")
        target_include_directories(test_fastforwardapp_${side} BEFORE PRIVATE "${FFI_DIR}/frozen/src")
        target_compile_definitions(test_fastforwardapp_${side} PRIVATE FASTFORWARD_FROZEN=1)
      endif()
      add_test(NAME fastforwardapp_${side} COMMAND test_fastforwardapp_${side})
      set_tests_properties(fastforwardapp_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_fastforwardapp_before)
    set_tests_properties(fastforwardapp_after PROPERTIES DEPENDS fastforwardapp_before)
  endif()

  # P17: public-input on/off/on, per-frame World budgets and actual plate pixels.
  # The optional baseline uses authentic task-entry files, never modified copies.
  set(FFT_BASE "${CMAKE_SOURCE_DIR}/build/fastforward-toggle-001/before-source")
  foreach(side IN ITEMS production baseline)
    if(side STREQUAL "baseline")
      if(NOT EXISTS "${FFT_BASE}/src/app/main.cpp")
        continue()
      endif()
      set(fft_sources "${FFT_BASE}/src/app/main.cpp" "${FFT_BASE}/src/app/RuntimeHud.cpp"
        "${FFT_BASE}/src/app/RuntimeSkillPanel.cpp")
    else()
      set(fft_sources src/app/main.cpp src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
    endif()
    add_executable(test_fastforward_toggle_${side} src/__test__/test_fastforward_toggle.cpp ${fft_sources})
    target_link_libraries(test_fastforward_toggle_${side} PRIVATE fastforward_support webolution_objects)
    if(side STREQUAL "baseline")
      target_include_directories(test_fastforward_toggle_${side} BEFORE PRIVATE "${FFT_BASE}/src")
    endif()
    add_test(NAME fastforward_toggle_${side} COMMAND test_fastforward_toggle_${side} ${side}
      "${CMAKE_CURRENT_BINARY_DIR}/fastforward-toggle-${side}")
    set_tests_properties(fastforward_toggle_${side} PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()

  # P7: per-frame fractional placements, public hover/click at that placement,
  # and identical simulation traces against the authentic task-entry frame loop.
  set(MOTION_BASE "${CMAKE_SOURCE_DIR}/build/actor-motion-001/before-source")
  set(MOTION_COMPARE "-")
  if(EXISTS "${MOTION_BASE}/src/app/main.cpp")
    set(MOTION_COMPARE "${CMAKE_CURRENT_BINARY_DIR}/actor-motion-baseline")
  endif()
  foreach(side IN ITEMS baseline production)
    if(side STREQUAL "baseline")
      if(NOT EXISTS "${MOTION_BASE}/src/app/main.cpp")
        continue()
      endif()
      set(motion_main "${MOTION_BASE}/src/app/main.cpp")
    else()
      set(motion_main src/app/main.cpp)
    endif()
    add_executable(test_actor_motion_${side} src/__test__/test_actor_motion.cpp
      ${motion_main} src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
    target_link_libraries(test_actor_motion_${side} PRIVATE fastforward_support webolution_objects)
    add_test(NAME actor_motion_${side} COMMAND test_actor_motion_${side} ${side}
      "${CMAKE_CURRENT_BINARY_DIR}/actor-motion-${side}"
      "${MOTION_COMPARE}")
    set_tests_properties(actor_motion_${side} PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    add_test(NAME actor_motion_wall_${side} COMMAND test_actor_motion_${side} wall-${side}
      "${CMAKE_CURRENT_BINARY_DIR}/actor-motion-wall-${side}")
    set_tests_properties(actor_motion_wall_${side} PROPERTIES TIMEOUT 60
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  add_test(NAME actor_motion_units COMMAND test_actor_motion_production units
    "${CMAKE_CURRENT_BINARY_DIR}/actor-motion-units")
  set_tests_properties(actor_motion_units PROPERTIES TIMEOUT 60)
  if(TARGET test_actor_motion_baseline)
    add_test(NAME actor_motion_identity COMMAND ${CMAKE_COMMAND}
      "-DBASELINE=${MOTION_BASE}" -P "${CMAKE_SOURCE_DIR}/cmake/actor_motion_identity.cmake")
    set_tests_properties(actor_motion_identity PROPERTIES TIMEOUT 30)
    set_tests_properties(actor_motion_baseline PROPERTIES DEPENDS actor_motion_identity)
    set_tests_properties(actor_motion_production PROPERTIES DEPENDS actor_motion_baseline)
  endif()

  set(HATCH_EXIT_BASE "${CMAKE_SOURCE_DIR}/build/hatch-exit-presentation-001/before-source")
  # Playtest R5/R6: public pointer routes, frozen task-entry main and exact
  # paired framebuffer comparison. Current-only checks also run on clean trees.
  set(HOVER_BASE "${CMAKE_SOURCE_DIR}/build/hover-feedback-001/before-source")
  set(HOVER_EVIDENCE "${CMAKE_SOURCE_DIR}/build/hover-feedback-001")
  foreach(side IN ITEMS after before)
    if(side STREQUAL "before")
      if(NOT EXISTS "${HOVER_BASE}/src/app/main.cpp")
        continue()
      endif()
      set(hover_main "${HOVER_BASE}/src/app/main.cpp")
    else()
      set(hover_main src/app/main.cpp)
    endif()
    add_executable(test_hover_feedback_${side} src/__test__/test_hover_feedback.cpp ${hover_main}
      src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
    target_link_libraries(test_hover_feedback_${side} PRIVATE fastforward_support webolution_objects)
    if(side STREQUAL "before")
      target_compile_definitions(test_hover_feedback_${side} PRIVATE HOVER_FEEDBACK_BEFORE=1)
    endif()
    foreach(route IN ITEMS actor hatch)
      add_test(NAME hover_feedback_${route}_${side} COMMAND test_hover_feedback_${side} ${route}
        "${HOVER_EVIDENCE}/${side}/${route}")
      set_tests_properties(hover_feedback_${route}_${side} PROPERTIES TIMEOUT 180
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endforeach()
  endforeach()
  add_test(NAME hover_feedback_units COMMAND test_hover_feedback_after units)
  set_tests_properties(hover_feedback_units PROPERTIES TIMEOUT 30)
  add_test(NAME hover_feedback_reference COMMAND ${CMAKE_COMMAND}
    "-DGAME_DIR=${WEBOLUTION_GAME_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/hover_feedback_reference.cmake")
  set_tests_properties(hover_feedback_reference PROPERTIES TIMEOUT 30)
  if(TARGET test_hover_feedback_before)
    add_test(NAME hover_feedback_identity COMMAND ${CMAKE_COMMAND}
      "-DBASELINE=${HOVER_BASE}" -P "${CMAKE_SOURCE_DIR}/cmake/hover_feedback_identity.cmake")
    set_tests_properties(hover_feedback_identity PROPERTIES TIMEOUT 30)
    foreach(route IN ITEMS actor hatch)
      set_tests_properties(hover_feedback_${route}_before PROPERTIES DEPENDS "hover_feedback_identity;hover_feedback_${route}_after")
    endforeach()
    add_test(NAME hover_feedback_comparison COMMAND test_hover_feedback_after compare "${HOVER_EVIDENCE}")
    set_tests_properties(hover_feedback_comparison PROPERTIES TIMEOUT 30
      DEPENDS "hover_feedback_actor_before;hover_feedback_hatch_before"
      ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
  # R1/R2: authentic task-entry application against the connected start fix.
  set(HATCH_TIMING_BASE "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/before-source")
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      if(NOT EXISTS "${HATCH_TIMING_BASE}/src/app/main.cpp")
        continue()
      endif()
      set(timing_main "${HATCH_TIMING_BASE}/src/app/main.cpp")
    else()
      set(timing_main src/app/main.cpp)
    endif()
    add_executable(test_hatch_timing_${side} src/__test__/test_hatch_timing.cpp ${timing_main}
      src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
    target_link_libraries(test_hatch_timing_${side} PRIVATE fastforward_support webolution_objects)
    if(side STREQUAL "before")
      target_compile_definitions(test_hatch_timing_${side} PRIVATE HATCH_TIMING_BEFORE=1)
    endif()
    add_test(NAME hatch_timing_${side} COMMAND test_hatch_timing_${side} entry
      "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/${side}")
    add_test(NAME hatch_timing_win_${side} COMMAND test_hatch_timing_${side} win
      "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/win-${side}")
    add_test(NAME hatch_timing_routes_${side} COMMAND test_hatch_timing_${side} routes
      "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/routes-${side}")
    # The routes take about 145 s alone against the unchanged 180 s limit; sharing the
    # machine with three other play tests has pushed them past it, so they run alone.
    set_tests_properties(hatch_timing_routes_${side} PROPERTIES TIMEOUT 180 RUN_SERIAL TRUE
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    set_tests_properties(hatch_timing_${side} hatch_timing_win_${side} PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  add_test(NAME hatch_timing_units COMMAND test_hatch_timing_after units "${CMAKE_CURRENT_BINARY_DIR}/hatch-timing-units")
  add_test(NAME hatch_timing_fast COMMAND test_hatch_timing_after fast "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/fast")
  add_test(NAME hatch_timing_assets COMMAND test_hatch_timing_after assets "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/assets")
  add_test(NAME hatch_timing_retry COMMAND test_hatch_timing_after retry "${CMAKE_SOURCE_DIR}/build/hatch-timing-001/retry")
  set_tests_properties(hatch_timing_units hatch_timing_fast hatch_timing_assets hatch_timing_retry PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  if(TARGET test_hatch_timing_before)
    add_test(NAME hatch_timing_identity COMMAND ${CMAKE_COMMAND}
      "-DBASELINE=${HATCH_TIMING_BASE}" -P "${CMAKE_SOURCE_DIR}/cmake/hatch_timing_identity.cmake")
    set_tests_properties(hatch_timing_identity PROPERTIES TIMEOUT 30)
    set_tests_properties(hatch_timing_before hatch_timing_win_before PROPERTIES DEPENDS hatch_timing_identity)
    set_tests_properties(hatch_timing_after PROPERTIES DEPENDS hatch_timing_before)
    set_tests_properties(hatch_timing_routes_before PROPERTIES DEPENDS hatch_timing_identity)
    set_tests_properties(hatch_timing_routes_after PROPERTIES DEPENDS hatch_timing_routes_before)
  endif()
  set(HATCH_EXIT_SUPPORT ${FFI_SUPPORT})
  list(REMOVE_ITEM HATCH_EXIT_SUPPORT "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp")
  add_library(hatch_exit_support OBJECT ${HATCH_EXIT_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(hatch_exit_support PUBLIC webolution_objects)
  target_compile_definitions(hatch_exit_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  add_executable(test_hatch_entry_fixture src/__test__/test_hatch_entry_fixture.cpp
    src/app/BackdropScene.cpp src/app/RuntimeHud.cpp)
  target_link_libraries(test_hatch_entry_fixture PRIVATE hatch_exit_support webolution_objects)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      if(NOT EXISTS "${HATCH_EXIT_BASE}/src/app/main.cpp")
        continue()
      endif()
      set(hatch_main "${HATCH_EXIT_BASE}/src/app/main.cpp")
      set(hatch_scene "${HATCH_EXIT_BASE}/src/app/BackdropScene.cpp")
    else()
      set(hatch_main src/app/main.cpp)
      set(hatch_scene src/app/BackdropScene.cpp)
    endif()
    add_executable(test_hatch_exit_${side} src/__test__/test_hatch_exit.cpp ${hatch_main} ${hatch_scene}
      src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
    target_link_libraries(test_hatch_exit_${side} PRIVATE hatch_exit_support webolution_objects)
    if(side STREQUAL "before")
      target_compile_definitions(test_hatch_exit_${side} PRIVATE HATCH_EXIT_BASELINE=1)
    endif()
    add_test(NAME hatch_exit_app_${side} COMMAND test_hatch_exit_${side} app
      "${CMAKE_SOURCE_DIR}/build/hatch-exit-presentation-001/${side}")
    add_test(NAME hatch_exit_end_${side} COMMAND test_hatch_exit_${side} end
      "${CMAKE_SOURCE_DIR}/build/hatch-exit-presentation-001/end-${side}")
    set_tests_properties(hatch_exit_app_${side} hatch_exit_end_${side} PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  add_test(NAME hatch_exit_assets COMMAND test_hatch_exit_after assets "${CMAKE_CURRENT_BINARY_DIR}/hatch-exit-assets")
  set_tests_properties(hatch_exit_assets PROPERTIES TIMEOUT 60
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME hatch_exit_models COMMAND test_hatch_exit_after models "${CMAKE_SOURCE_DIR}/build/hatch-exit-presentation-001/models")
  set_tests_properties(hatch_exit_models PROPERTIES TIMEOUT 60
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  if(TARGET test_hatch_exit_before)
    add_test(NAME hatch_exit_identity COMMAND ${CMAKE_COMMAND}
      "-DBASELINE=${HATCH_EXIT_BASE}" -P "${CMAKE_SOURCE_DIR}/cmake/hatch_exit_identity.cmake")
    set_tests_properties(hatch_exit_identity PROPERTIES TIMEOUT 30)
    set_tests_properties(hatch_exit_app_before hatch_exit_end_before PROPERTIES DEPENDS hatch_exit_identity)
    set_tests_properties(hatch_exit_app_after PROPERTIES DEPENDS hatch_exit_app_before)
    set_tests_properties(hatch_exit_end_after PROPERTIES DEPENDS hatch_exit_end_before)
  endif()

  # P8: installed ledge, naturally excavated Basher tunnel and live Bomber full
  # rotations. Baseline compiles the authentic task-entry TubeScene separately.
  set(OCCLUSION_BASE "${CMAKE_SOURCE_DIR}/build/actor-occlusion-001/before-source")
  set(OCCLUSION_SUPPORT ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM OCCLUSION_SUPPORT "${CMAKE_SOURCE_DIR}/src/app/TubeScene.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
  add_library(actor_occlusion_support OBJECT ${OCCLUSION_SUPPORT}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(actor_occlusion_support PUBLIC webolution_objects)
  target_compile_definitions(actor_occlusion_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  set(OCCLUSION_COMPARE "-")
  if(EXISTS "${OCCLUSION_BASE}/src/app/TubeScene.cpp")
    set(OCCLUSION_COMPARE "${CMAKE_CURRENT_BINARY_DIR}/actor-occlusion-before")
  endif()
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      if(NOT EXISTS "${OCCLUSION_BASE}/src/app/TubeScene.cpp")
        continue()
      endif()
      set(occlusion_scene "${OCCLUSION_BASE}/src/app/TubeScene.cpp")
      set(occlusion_app "${OCCLUSION_BASE}/src/app/main.cpp")
      if(EXISTS "${CMAKE_SOURCE_DIR}/build/actor-depth-001/before-source/src/app/main.cpp")
        set(occlusion_app "${CMAKE_SOURCE_DIR}/build/actor-depth-001/before-source/src/app/main.cpp")
      endif()
      set(occlusion_compare "-")
    else()
      set(occlusion_scene src/app/TubeScene.cpp)
      set(occlusion_app src/app/main.cpp)
      set(occlusion_compare "${OCCLUSION_COMPARE}")
    endif()
    add_executable(test_actor_occlusion_${side} src/__test__/test_actor_occlusion.cpp
      ${occlusion_scene} ${occlusion_app})
    target_link_libraries(test_actor_occlusion_${side} PRIVATE actor_occlusion_support webolution_objects)
    if(side STREQUAL "after")
      target_compile_definitions(test_actor_occlusion_${side} PRIVATE OCCLUSION_HOVER_EXPECTATION=1)
    endif()
    add_test(NAME actor_occlusion_${side} COMMAND test_actor_occlusion_${side} ${side}
      "${CMAKE_CURRENT_BINARY_DIR}/actor-occlusion-${side}" "${occlusion_compare}")
    set_tests_properties(actor_occlusion_${side} PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    set(occlusion_app_compare "-")
    if(side STREQUAL "after" AND TARGET test_actor_occlusion_before)
      set(occlusion_app_compare "${CMAKE_CURRENT_BINARY_DIR}/actor-occlusion-app-before")
    endif()
    add_test(NAME actor_occlusion_app_${side} COMMAND test_actor_occlusion_${side} app-${side}
      "${CMAKE_CURRENT_BINARY_DIR}/actor-occlusion-app-${side}" "${occlusion_app_compare}")
    set_tests_properties(actor_occlusion_app_${side} PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  if(TARGET test_actor_occlusion_before)
    add_test(NAME actor_occlusion_identity COMMAND ${CMAKE_COMMAND}
      "-DBASELINE=${OCCLUSION_BASE}" -P "${CMAKE_SOURCE_DIR}/cmake/actor_occlusion_identity.cmake")
    set_tests_properties(actor_occlusion_identity PROPERTIES TIMEOUT 30)
    set_tests_properties(actor_occlusion_before actor_occlusion_app_before PROPERTIES DEPENDS actor_occlusion_identity)
    set_tests_properties(actor_occlusion_after PROPERTIES DEPENDS actor_occlusion_before)
    set_tests_properties(actor_occlusion_app_after PROPERTIES DEPENDS actor_occlusion_app_before)
  endif()

  # R3: actual installed climber/ledge/pillar/dug-tunnel snapshots, 48 views
  # each. Before source was frozen at actor-depth-001 entry, never regenerated.
  set(ACTOR_DEPTH_BASE "${CMAKE_SOURCE_DIR}/build/actor-depth-001/before-source")
  add_test(NAME actor_depth_reference COMMAND ${CMAKE_COMMAND}
    "-DGAME_DIR=${WEBOLUTION_GAME_DIR}"
    -P "${CMAKE_SOURCE_DIR}/cmake/actor_depth_reference.cmake")
  set_tests_properties(actor_depth_reference PROPERTIES TIMEOUT 30)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      if(NOT EXISTS "${ACTOR_DEPTH_BASE}/src/app/TubeScene.cpp")
        continue()
      endif()
      set(depth_scene "${ACTOR_DEPTH_BASE}/src/app/TubeScene.cpp")
      set(depth_compare "-")
    else()
      set(depth_scene src/app/TubeScene.cpp)
      set(depth_compare "-")
      if(EXISTS "${ACTOR_DEPTH_BASE}/src/app/TubeScene.cpp")
        set(depth_compare "${CMAKE_CURRENT_BINARY_DIR}/actor-depth-before")
      endif()
    endif()
    add_executable(test_actor_depth_${side} src/__test__/test_actor_depth.cpp ${depth_scene})
    target_link_libraries(test_actor_depth_${side} PRIVATE actor_occlusion_support webolution_objects)
    if(side STREQUAL "before")
      target_compile_definitions(test_actor_depth_${side} PRIVATE ACTOR_DEPTH_BEFORE=1)
    endif()
    add_test(NAME actor_depth_${side} COMMAND test_actor_depth_${side} ${side}
      "${CMAKE_CURRENT_BINARY_DIR}/actor-depth-${side}" "${depth_compare}")
    set_tests_properties(actor_depth_${side} PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  if(TARGET test_actor_depth_before)
    add_test(NAME actor_depth_identity COMMAND ${CMAKE_COMMAND}
      "-DBASELINE=${ACTOR_DEPTH_BASE}" -P "${CMAKE_SOURCE_DIR}/cmake/actor_depth_identity.cmake")
    set_tests_properties(actor_depth_identity PROPERTIES TIMEOUT 30)
    set_tests_properties(actor_depth_before PROPERTIES DEPENDS "actor_depth_identity;actor_depth_reference")
    if(TARGET test_actor_occlusion_before)
      set_tests_properties(actor_occlusion_app_before PROPERTIES DEPENDS "actor_occlusion_identity;actor_depth_identity")
    endif()
    set_tests_properties(actor_depth_after PROPERTIES DEPENDS actor_depth_before)
  endif()

  # playable-baseline-001: scripted public-input assessment of three installed levels. Outcomes
  # (win/loss/watchdog) are reported to build/playable-baseline-001, never asserted; the exit code
  # reflects driver integrity only. Isolated FileSaveSink under that directory; no installed save.
  set(PB_DIR "${CMAKE_SOURCE_DIR}/build/playable-baseline-001")
  add_executable(test_playablebaseline src/__test__/test_playablebaseline.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_levelsweep src/__test__/test_levelsweep.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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
  add_executable(test_playableexpansion src/__test__/test_playableexpansion.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
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

  # level-selection-case-001: the 20 map spellings that differ in case from the archive. The frozen
  # task-entry main.cpp/RetailLevel.cpp (ignored build/level-selection-case-001/frozen, written before any
  # production edit, identity-checked below) are built into their own driver and must REPRODUCE the failure
  # through the frontend; the current build must enter the canonical level on the same route. Frontend runs use
  # an isolated save (nodes >= 3 a labelled DIAGNOSTIC prepared-save fixture); outputs under LSC_DIR/evidence.
  set(LSC_DIR "${CMAKE_SOURCE_DIR}/build/level-selection-case-001")
  set(LSC_ENV "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_test(NAME levelselection_identity COMMAND ${CMAKE_COMMAND}
    "-DFROZEN_DIR=${LSC_DIR}/frozen"
    "-DMANIFEST=${CMAKE_SOURCE_DIR}/build/agent-runs/loop-4070ec7c9130425f9242b8cbc09c0fab/0004-review-af050c/before-source-hashes.json"
    -P ${CMAKE_SOURCE_DIR}/cmake/levelselectioncase_identity.cmake)
  set_tests_properties(levelselection_identity PROPERTIES TIMEOUT 60)

  add_executable(test_levelselection_after src/__test__/test_levelselection.cpp
    ${WEBOLUTION_GFX_SOURCES} src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
    src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(test_levelselection_after PRIVATE webolution_objects)
  target_compile_definitions(test_levelselection_after PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(test_levelselection_after PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(test_levelselection_after PRIVATE PkgConfig::SDL2)
  endif()

  if(EXISTS "${LSC_DIR}/frozen/src/app/main.cpp" AND EXISTS "${LSC_DIR}/frozen/src/sim/RetailLevel.cpp")
    set(LSC_BASE ${WEBOLUTION_SOURCES})
    list(REMOVE_ITEM LSC_BASE "${CMAKE_SOURCE_DIR}/src/sim/RetailLevel.cpp")
    add_library(lsc_before_base OBJECT ${LSC_BASE} ${WEBOLUTION_AUDIO_SOURCES}
      "${LSC_DIR}/frozen/src/sim/RetailLevel.cpp")
    target_include_directories(lsc_before_base PUBLIC
      ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/assets ${CMAKE_SOURCE_DIR}/src/sim)
    target_link_libraries(lsc_before_base PUBLIC
      webolution_texture_decoder webolution_sprite_resource webolution_compiled_level)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_link_libraries(lsc_before_base PUBLIC stdc++exp)
    endif()
    target_compile_options(lsc_before_base PUBLIC
      $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>)
    if(TARGET SDL2::SDL2)
      target_link_libraries(lsc_before_base PUBLIC SDL2::SDL2)
    elseif(TARGET PkgConfig::SDL2)
      target_link_libraries(lsc_before_base PUBLIC PkgConfig::SDL2)
    endif()
    set(LSC_GFX ${WEBOLUTION_GFX_SOURCES})
    list(REMOVE_ITEM LSC_GFX "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
    add_executable(test_levelselection_before src/__test__/test_levelselection.cpp ${LSC_GFX}
      "${LSC_DIR}/frozen/src/app/main.cpp" src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp
      src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
    target_link_libraries(test_levelselection_before PRIVATE lsc_before_base)
    target_compile_definitions(test_levelselection_before PRIVATE
      WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 LSC_FROZEN=1)
    foreach(node IN ITEMS 0 9)
      add_test(NAME levelselection_before_node${node}
        COMMAND test_levelselection_before frontend ${node} "${LSC_DIR}/evidence")
      set_tests_properties(levelselection_before_node${node} PROPERTIES TIMEOUT 300 ENVIRONMENT "${LSC_ENV}")
    endforeach()
  endif()
  foreach(node IN ITEMS 0 9 11 21)
    add_test(NAME levelselection_after_node${node}
      COMMAND test_levelselection_after frontend ${node} "${LSC_DIR}/evidence")
    set_tests_properties(levelselection_after_node${node} PROPERTIES TIMEOUT 300 ENVIRONMENT "${LSC_ENV}")
    if(TARGET test_levelselection_before)
      set_tests_properties(levelselection_after_node${node} PROPERTIES DEPENDS levelselection_before_node9)
    endif()
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

  # native-023: public loadRetailLevel on every installed teleport level in the
  # frozen task-entry build and in the current build. The frozen sources live in
  # the ignored build/native-023/frozen/ (never in src/); their identity is the
  # supervisor entry manifest's, checked by native023_identity_report.
  include(cmake/native023_freeze.cmake)
  set(N023_DIR "${CMAKE_SOURCE_DIR}/build/native-023")
  native023_freeze("${N023_DIR}/frozen")
  message(STATUS "native-023 frozen sources: ${NATIVE023_FROZEN_STATE}")
  # Fails unless the frozen files hash like the supervisor entry manifest.
  add_test(NAME native023_identity_report COMMAND ${CMAKE_COMMAND}
    "-DFROZEN_DIR=${N023_DIR}/frozen"
    "-DMANIFEST=${CMAKE_SOURCE_DIR}/build/agent-runs/${N023_ENTRY_RUN}/before-source-hashes.json"
    -P ${CMAKE_SOURCE_DIR}/cmake/native023_identity.cmake)
  set_tests_properties(native023_identity_report PROPERTIES TIMEOUT 60)
  native023_frozen_matches("${N023_DIR}/frozen" N023_FROZEN_OK)
  if(N023_FROZEN_OK)
    set(N023_BASE ${WEBOLUTION_SOURCES})
    list(REMOVE_ITEM N023_BASE
      "${CMAKE_SOURCE_DIR}/src/assets/LevelObjects.cpp" "${CMAKE_SOURCE_DIR}/src/sim/RetailLevel.cpp")
    add_library(native023_before_base OBJECT ${N023_BASE}
      "${N023_DIR}/frozen/assets/LevelObjects.cpp" "${N023_DIR}/frozen/sim/RetailLevel.cpp")
    # Frozen headers first (assets/LevelObjects.h), then the tree for everything else.
    target_include_directories(native023_before_base PUBLIC
      "${N023_DIR}/frozen" ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/assets ${CMAKE_SOURCE_DIR}/src/sim)
    target_link_libraries(native023_before_base PUBLIC
      webolution_texture_decoder webolution_sprite_resource webolution_compiled_level)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_link_libraries(native023_before_base PUBLIC stdc++exp)
    endif()
    target_compile_options(native023_before_base PUBLIC
      $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>)
    if(TARGET SDL2::SDL2)
      target_link_libraries(native023_before_base PUBLIC SDL2::SDL2)
    elseif(TARGET PkgConfig::SDL2)
      target_link_libraries(native023_before_base PUBLIC PkgConfig::SDL2)
    endif()
  endif()
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      if(NOT TARGET native023_before_base)
        continue()
      endif()
      set(n023_lib native023_before_base)
    else()
      set(n023_lib webolution_objects)
    endif()
    add_executable(test_teleportdemo_${side} src/__test__/test_teleportdemo.cpp)
    target_link_libraries(test_teleportdemo_${side} PRIVATE ${n023_lib})
    target_compile_definitions(test_teleportdemo_${side} PRIVATE
      TELEPORTDEMO_OUT_DIR="${N023_DIR}")
    if(side STREQUAL "before")
      target_compile_definitions(test_teleportdemo_${side} PRIVATE TELEPORTDEMO_FROZEN=1)
    endif()
    add_test(NAME teleportdemo_${side} COMMAND test_teleportdemo_${side})
    set_tests_properties(teleportdemo_${side} PROPERTIES TIMEOUT 120
      ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
endif()

# Playtest Q1 radial walker lanes and source-built before/after evidence.
include(cmake/walker_lane.cmake)
include(cmake/fluid_kinds.cmake)
