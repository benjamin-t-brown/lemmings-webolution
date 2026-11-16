# release_rate_units removed: it encoded the superseded RATE panel geometry.
# Short hatch-limit and replay compatibility invariants; remains legacy.
if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_hatch_rate src/__test__/test_hatch_rate.cpp)
  target_link_libraries(test_hatch_rate PRIVATE webolution_app webolution_objects SDL2::SDL2)
  add_test(NAME hatch_rate_units COMMAND test_hatch_rate "${CMAKE_CURRENT_BINARY_DIR}/hatch-rate-unit")
  set_tests_properties(hatch_rate_units PROPERTIES TIMEOUT 10
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()
