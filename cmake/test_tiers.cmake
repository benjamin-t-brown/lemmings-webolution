# Test tiers (2026-11-15, project lead). Included last, after every test is registered.
#
# smoke   the per-task gate: asset/format/save/sim units, one frontend -> level -> results ->
#         save round trip, and the 102-level sweep. `verify.py --profile smoke` builds only
#         the smoke_tests target and runs `ctest -L smoke`.
# legacy  everything else: task-specific routes and their before/baseline/identity replays.
#         Out of the gate; run with `verify.py --profile all` at a milestone sign-off. A
#         legacy test broken by an intended behavior change is removed, not repaired.
#
# A new test joins smoke only if it guards something every later task can break and it runs
# in a few seconds. Playability is confirmed by human playtest, not by adding tests here.
set(WEBOLUTION_SMOKE_TESTS
  # assets, formats, save
  box lrz bitmap wav surface polylist gamedata corpus levelindex levelobjects retaillevel
  savedgame savepersist progress results
  # simulation
  world terrain collisiongrid simclock sharedtiming levergates leverinitface teleportlinks
  fastforward_units clicktoassign pick runtimepick
  # rendering units
  spriteview actorsprite actorpose actorterrain actoranim actorcompose actorvisibility
  spritestate tileposition cellmesh scenenode sceneraster sceneclip scenedepth scenedispatch
  scenediffuse scenematerialflags scenemeshsource scenevertexpack scene_pack_adoption
  scenebillboardsource backdropscene mapscene
  # shell, HUD, input
  frontend frontend_sequence frontendsession menurender hudtext pausecaption levelmenu
  skillpanel runtimeskillpanel runtimehud runtimecursor cursorpolicy camerainput cameraintro
  levelend entrystartcue cueplayback
  # application paths
  frontendentry frontendwindow frontendwindowquit levelinteraction levelcursor leveloverlap
  levelvisibility teleportapp levelreturn
  # every installed level: load, render, idle simulation
  levelsweep_report teleportsoftlock_sweep)
foreach(chunk RANGE 0 16)
  string(LENGTH "${chunk}" chunk_digits)
  if(chunk_digits EQUAL 1)
    list(APPEND WEBOLUTION_SMOKE_TESTS "levelsweep_chunk_0${chunk}")
  else()
    list(APPEND WEBOLUTION_SMOKE_TESTS "levelsweep_chunk_${chunk}")
  endif()
endforeach()

if(BUILD_TESTING)
  get_property(all_tests DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY TESTS)
  add_custom_target(smoke_tests)
  add_dependencies(smoke_tests webolution)
  set(smoke_count 0)
  foreach(name IN LISTS all_tests)
    if(name IN_LIST WEBOLUTION_SMOKE_TESTS)
      set_property(TEST ${name} APPEND PROPERTY LABELS smoke)
      math(EXPR smoke_count "${smoke_count} + 1")
      set(target "test_${name}")
      if(name MATCHES "^levelsweep_")
        set(target test_levelsweep)
      elseif(name STREQUAL "teleportsoftlock_sweep")
        set(target test_teleportsoftlock)
      endif()
      if(NOT TARGET ${target})
        message(FATAL_ERROR "Smoke test ${name} has no executable target ${target}")
      endif()
      add_dependencies(smoke_tests ${target})
    else()
      set_property(TEST ${name} APPEND PROPERTY LABELS legacy)
    endif()
  endforeach()
  # The gate must not shrink silently when the game data and SDL are present.
  if(WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR AND NOT EMSCRIPTEN)
    foreach(name IN LISTS WEBOLUTION_SMOKE_TESTS)
      if(NOT name IN_LIST all_tests)
        message(FATAL_ERROR "Smoke test ${name} is not registered")
      endif()
    endforeach()
  endif()
  list(LENGTH all_tests all_count)
  message(STATUS "Test tiers: ${smoke_count} smoke of ${all_count} registered")
endif()
