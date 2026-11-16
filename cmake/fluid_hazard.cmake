if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_fluid_hazard_after src/__test__/test_fluid_hazard.cpp
    src/app/main.cpp src/app/BackdropScene.cpp)
  target_link_libraries(test_fluid_hazard_after PRIVATE scene_demo_support
    webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
  target_include_directories(test_fluid_hazard_after PRIVATE "${CMAKE_SOURCE_DIR}/src")
  target_compile_definitions(test_fluid_hazard_after PRIVATE FLUID_HAZARD_after=1)
  target_sources(test_fluid_hazard_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
  add_test(NAME fluid_hazard_after COMMAND test_fluid_hazard_after
    "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/after")
  set_tests_properties(fluid_hazard_after PROPERTIES TIMEOUT 240
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  if(EXISTS "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/gadget-seed/SavedGame.dat")
    # Keep the current-source winning-placement replay independent of the removed before driver.
    add_executable(test_fluid_hazard_gadget_replay
      src/__test__/test_fluid_hazard_gadget_replay.cpp src/app/main.cpp src/app/BackdropScene.cpp
      $<TARGET_OBJECTS:webolution_objects>)
    target_link_libraries(test_fluid_hazard_gadget_replay PRIVATE scene_demo_support
      webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
    target_include_directories(test_fluid_hazard_gadget_replay PRIVATE "${CMAKE_SOURCE_DIR}/src")
    add_test(NAME fluid_hazard_gadget_replay COMMAND test_fluid_hazard_gadget_replay
      "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/gadget-seed/SavedGame.dat"
      "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/gadget-replay")
    set_tests_properties(fluid_hazard_gadget_replay PROPERTIES TIMEOUT 300
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
endif()
