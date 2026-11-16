# Short control/data invariants only. Pointer playability is confirmed by the
# human playtest; retained earlier clone observations are linked in the report.
if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_release_rate src/__test__/test_release_rate.cpp)
  target_link_libraries(test_release_rate PRIVATE webolution_app webolution_objects SDL2::SDL2)
  add_test(NAME release_rate_units COMMAND test_release_rate)
  set_tests_properties(release_rate_units PROPERTIES TIMEOUT 10
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  # test_tiers.cmake leaves this test in legacy.
endif()
