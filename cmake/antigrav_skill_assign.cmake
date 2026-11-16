if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  set(ags_root "${CMAKE_SOURCE_DIR}/build/antigrav-skill-assign-001")
  add_executable(test_antigrav_skill_after src/__test__/test_antigrav_skill_assign.cpp)
  target_include_directories(test_antigrav_skill_after PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/src/sim")
  target_link_libraries(test_antigrav_skill_after PRIVATE webolution_app webolution_app_nomain
    webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
  target_sources(test_antigrav_skill_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
  add_test(NAME antigrav_skill_after COMMAND test_antigrav_skill_after
    "${CMAKE_CURRENT_BINARY_DIR}/antigrav-skill-after")
  set_tests_properties(antigrav_skill_after PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
endif()
