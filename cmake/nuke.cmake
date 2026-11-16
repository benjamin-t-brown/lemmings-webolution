if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  set(nuke_root "${CMAKE_SOURCE_DIR}/build/nuke-all-001")
  add_executable(test_nuke_after src/__test__/test_nuke.cpp)
  target_link_libraries(test_nuke_after PRIVATE webolution_app webolution_objects SDL2::SDL2)
  target_link_libraries(test_nuke_after PRIVATE webolution_app_nomain)
  add_test(NAME nuke_pointer_after COMMAND test_nuke_after live "${CMAKE_CURRENT_BINARY_DIR}/nuke-after/live")
  set_tests_properties(nuke_pointer_after PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  foreach(mode IN ITEMS units empty partial)
    add_test(NAME nuke_${mode} COMMAND test_nuke_after ${mode} "${CMAKE_CURRENT_BINARY_DIR}/nuke-after/${mode}")
    set_tests_properties(nuke_${mode} PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
endif()
