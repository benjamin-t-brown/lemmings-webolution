# Q1: production app and optional authentic task-entry app, with one driver.
if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  set(LANE_BASE "${CMAKE_SOURCE_DIR}/build/walker-lane-001/before-source")
  set(LANE_COMPARE "-")
  if(EXISTS "${LANE_BASE}/src/app/TubeScene.cpp")
    set(LANE_COMPARE "${CMAKE_CURRENT_BINARY_DIR}/walker-lane-baseline")
    set(lane_frozen_sources ${WEBOLUTION_SOURCES} ${WEBOLUTION_AUDIO_SOURCES} ${WEBOLUTION_GFX_SOURCES})
    list(APPEND lane_frozen_sources
      "${CMAKE_SOURCE_DIR}/src/app/SceneVertexPack.cpp" "${CMAKE_SOURCE_DIR}/src/app/SceneDiffuse.cpp"
      "${CMAKE_SOURCE_DIR}/src/app/SceneDepth.cpp" "${CMAKE_SOURCE_DIR}/src/app/SceneDispatch.cpp")
    list(TRANSFORM lane_frozen_sources REPLACE "${CMAKE_SOURCE_DIR}/src/" "${LANE_BASE}/src/")
    add_executable(test_walker_lane_baseline src/__test__/test_walker_lane.cpp ${lane_frozen_sources})
    target_include_directories(test_walker_lane_baseline BEFORE PRIVATE "${LANE_BASE}/src")
    target_link_libraries(test_walker_lane_baseline PRIVATE webolution_texture_decoder
      webolution_sprite_resource webolution_compiled_level stdc++exp)
    target_compile_definitions(test_walker_lane_baseline PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1 LANE_BASELINE=1)
    if(TARGET SDL2::SDL2)
      target_link_libraries(test_walker_lane_baseline PRIVATE SDL2::SDL2)
    elseif(TARGET PkgConfig::SDL2)
      target_link_libraries(test_walker_lane_baseline PRIVATE PkgConfig::SDL2)
    endif()
    add_test(NAME walker_lane_baseline COMMAND test_walker_lane_baseline baseline
      "${CMAKE_CURRENT_BINARY_DIR}/walker-lane-baseline" "-")
    set_tests_properties(walker_lane_baseline PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
  add_executable(test_walker_lane src/__test__/test_walker_lane.cpp
    src/app/main.cpp src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_walker_lane PRIVATE fastforward_support webolution_objects)
  add_test(NAME walker_lane_production COMMAND test_walker_lane production
    "${CMAKE_CURRENT_BINARY_DIR}/walker-lane-production" "${LANE_COMPARE}")
  add_test(NAME walker_lane_units COMMAND test_walker_lane units "${CMAKE_CURRENT_BINARY_DIR}/walker-lane-units" "-")
  set_tests_properties(walker_lane_production PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(walker_lane_units PROPERTIES TIMEOUT 60)
  if(TARGET test_walker_lane_baseline)
    add_test(NAME walker_lane_identity COMMAND ${CMAKE_COMMAND}
      "-DBASELINE=${LANE_BASE}" -P "${CMAKE_SOURCE_DIR}/cmake/walker_lane_identity.cmake")
    set_tests_properties(walker_lane_baseline PROPERTIES DEPENDS walker_lane_identity)
    set_tests_properties(walker_lane_production PROPERTIES DEPENDS walker_lane_baseline)
  endif()
endif()
