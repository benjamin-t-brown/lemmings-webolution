if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  set(LANE_COMPARE "-")
  add_executable(test_walker_lane src/__test__/test_walker_lane.cpp
    src/app/main.cpp src/app/RuntimeHud.cpp src/app/RuntimeSkillPanel.cpp)
  target_link_libraries(test_walker_lane PRIVATE fastforward_support webolution_objects)
  add_test(NAME walker_lane_production COMMAND test_walker_lane production
    "${CMAKE_CURRENT_BINARY_DIR}/walker-lane-production" "${LANE_COMPARE}")
  add_test(NAME walker_lane_units COMMAND test_walker_lane units "${CMAKE_CURRENT_BINARY_DIR}/walker-lane-units" "-")
  set_tests_properties(walker_lane_production PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  set_tests_properties(walker_lane_units PROPERTIES TIMEOUT 60)
endif()
