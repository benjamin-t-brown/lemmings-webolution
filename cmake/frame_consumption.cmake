if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_frame_consumption src/__test__/test_frame_consumption.cpp)
  target_link_libraries(test_frame_consumption PRIVATE
    webolution_app webolution_app_nomain webolution_objects)
  target_compile_definitions(test_frame_consumption PRIVATE WEBOLUTION_HAVE_SDL2=1)
  add_test(NAME frame_consumption COMMAND test_frame_consumption
    "${CMAKE_CURRENT_BINARY_DIR}/frame-consumption-test")
  set_tests_properties(frame_consumption PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  add_executable(test_prepared_frame src/__test__/test_prepared_frame.cpp)
  target_link_libraries(test_prepared_frame PRIVATE webolution_app webolution_app_nomain webolution_objects)
  add_test(NAME prepared_frame COMMAND test_prepared_frame "${WEBOLUTION_GAME_DIR}")
  set_tests_properties(prepared_frame PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy")
endif()
