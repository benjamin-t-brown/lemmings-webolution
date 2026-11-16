if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  set(er_root "${CMAKE_SOURCE_DIR}/build/empty-receiver-fall-001")
  add_executable(test_empty_receiver_after src/__test__/test_empty_receiver.cpp)
  target_link_libraries(test_empty_receiver_after PRIVATE
    webolution_app webolution_app_nomain webolution_texture_decoder
    webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
  target_sources(test_empty_receiver_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
  target_compile_definitions(test_empty_receiver_after PRIVATE EMPTY_RECEIVER_AFTER=1)
  add_test(NAME empty_receiver_after COMMAND test_empty_receiver_after "${er_root}/after")
  set_tests_properties(empty_receiver_after PROPERTIES TIMEOUT 240
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()
