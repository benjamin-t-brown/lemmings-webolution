if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  set(pick_nearest_root "${CMAKE_SOURCE_DIR}/build/lemming-pick-nearest-001")
  add_executable(test_pick_nearest_after src/__test__/test_pick_nearest.cpp)
  target_link_libraries(test_pick_nearest_after PRIVATE webolution_app webolution_objects)
  target_link_libraries(test_pick_nearest_after PRIVATE webolution_app_nomain)
  foreach(route IN ITEMS upright-left upright-right upright-between upright-overlap inverted-left inverted-right inverted-between)
    add_test(NAME pick_nearest_${route}_after COMMAND test_pick_nearest_after ${route}
      "${CMAKE_CURRENT_BINARY_DIR}/pick-nearest-after/${route}")
    set_tests_properties(pick_nearest_${route}_after PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  add_test(NAME pick_nearest_units COMMAND test_pick_nearest_after units)
  set_tests_properties(pick_nearest_units PROPERTIES TIMEOUT 30)
  add_test(NAME pick_nearest_reference COMMAND ${CMAKE_COMMAND}
    "-DGAME_DIR=${WEBOLUTION_GAME_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/pick_nearest_reference.cmake")
  set_tests_properties(pick_nearest_reference PROPERTIES TIMEOUT 30)
endif()
