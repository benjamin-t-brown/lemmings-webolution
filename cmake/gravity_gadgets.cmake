if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  set(gg_root "${CMAKE_SOURCE_DIR}/build/gravity-gadgets-001")
  add_executable(test_gravity_gadgets_after src/__test__/test_gravity_gadgets.cpp)
  target_include_directories(test_gravity_gadgets_after PRIVATE "${CMAKE_SOURCE_DIR}/src")
  target_link_libraries(test_gravity_gadgets_after PRIVATE
    webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
  target_sources(test_gravity_gadgets_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
  target_link_libraries(test_gravity_gadgets_after PRIVATE webolution_app webolution_app_nomain)
  # release-rate-control-001: legacy after full frames omit RATE.
endif()
