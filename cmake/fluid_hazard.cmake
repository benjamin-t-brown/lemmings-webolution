if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before" AND NOT EXISTS "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/before-source/World.cpp")
      continue()
    endif()
    add_executable(test_fluid_hazard_${side} src/__test__/test_fluid_hazard.cpp
      src/app/main.cpp src/app/BackdropScene.cpp)
    target_link_libraries(test_fluid_hazard_${side} PRIVATE scene_demo_support
      webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
    target_include_directories(test_fluid_hazard_${side} PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_compile_definitions(test_fluid_hazard_${side} PRIVATE FLUID_HAZARD_${side}=1)
    if(side STREQUAL "before")
      # Same production objects except the authentic, task-entry World implementation.
      target_sources(test_fluid_hazard_before PRIVATE
        "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,sim.World.cpp.obj>"
        "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/before-source/src/sim/World.cpp")
      target_include_directories(test_fluid_hazard_before PRIVATE
        "${CMAKE_SOURCE_DIR}/src/sim")
    else()
      target_sources(test_fluid_hazard_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
    endif()
    add_test(NAME fluid_hazard_${side} COMMAND test_fluid_hazard_${side}
      "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/${side}")
    set_tests_properties(fluid_hazard_${side} PROPERTIES TIMEOUT 240
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  if(TARGET test_fluid_hazard_before)
    add_test(NAME fluid_hazard_identity COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${CMAKE_SOURCE_DIR}/build/fluid-hazard-001"
      -P "${CMAKE_SOURCE_DIR}/cmake/fluid_hazard_identity.cmake")
    set_tests_properties(fluid_hazard_before PROPERTIES DEPENDS fluid_hazard_identity)
    add_test(NAME fluid_hazard_water_control COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${CMAKE_SOURCE_DIR}/build/fluid-hazard-001"
      -P "${CMAKE_SOURCE_DIR}/cmake/fluid_hazard_water_control.cmake")
    set_tests_properties(fluid_hazard_water_control PROPERTIES
      DEPENDS "fluid_hazard_before;fluid_hazard_after")

    # Bounded control for the existing wall-clock gadget driver's varying outcome.
    # Seed is earned private clone-test progress, never a user/installed save.
    if(EXISTS "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/gadget-seed/SavedGame.dat")
      add_executable(test_fluid_hazard_gadget_before
        src/__test__/test_playableexpansion.cpp src/app/main.cpp src/app/BackdropScene.cpp
        "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,sim.World.cpp.obj>"
        "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/before-source/src/sim/World.cpp")
      target_link_libraries(test_fluid_hazard_gadget_before PRIVATE scene_demo_support
        webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
      target_include_directories(test_fluid_hazard_gadget_before PRIVATE
        "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/src/sim")
      target_compile_definitions(test_fluid_hazard_gadget_before PRIVATE PE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
      add_test(NAME fluid_hazard_gadget_seed COMMAND ${CMAKE_COMMAND}
        "-DTASK_DIR=${CMAKE_SOURCE_DIR}/build/fluid-hazard-001"
        -P "${CMAKE_SOURCE_DIR}/cmake/fluid_hazard_gadget_seed.cmake")
      set_tests_properties(fluid_hazard_gadget_seed PROPERTIES DEPENDS fluid_hazard_identity)
      add_test(NAME fluid_hazard_gadget_before COMMAND test_fluid_hazard_gadget_before play 4
        "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/gadget-before")
      set_tests_properties(fluid_hazard_gadget_before PROPERTIES TIMEOUT 1800
        DEPENDS fluid_hazard_gadget_seed
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

      # Keep the historical winning placements as an asserted public-input replay;
      # the original wall-clock assessment and its outcome reporting stay unchanged.
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
        DEPENDS fluid_hazard_gadget_seed
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endif()
endif()
