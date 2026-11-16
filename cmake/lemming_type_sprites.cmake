if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  set(lts_root "${CMAKE_SOURCE_DIR}/build/lemming-type-sprites-001")
  add_executable(test_lemming_type_sprites_after src/__test__/test_lemming_type_sprites.cpp)
  target_link_libraries(test_lemming_type_sprites_after PRIVATE webolution_app webolution_app_nomain webolution_objects)
  target_compile_definitions(test_lemming_type_sprites_after PRIVATE WEBOLUTION_HAVE_SDL2=1)
  # release-rate-control-001: legacy after full frames omit RATE.
endif()
