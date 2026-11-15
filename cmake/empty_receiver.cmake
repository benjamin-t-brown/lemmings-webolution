if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  set(er_root "${CMAKE_SOURCE_DIR}/build/empty-receiver-fall-001")
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before" AND NOT EXISTS "${er_root}/before-source/src/sim/World.cpp")
      continue()
    endif()
    add_executable(test_empty_receiver_${side} src/__test__/test_empty_receiver.cpp)
    target_link_libraries(test_empty_receiver_${side} PRIVATE
      webolution_app webolution_app_nomain webolution_texture_decoder
      webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
    if(side STREQUAL "before")
      target_sources(test_empty_receiver_before PRIVATE
        "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,sim.(World|RetailLevel).cpp.obj>"
        "${er_root}/before-source/src/sim/World.cpp"
        "${er_root}/before-source/src/sim/RetailLevel.cpp")
      target_include_directories(test_empty_receiver_before BEFORE PRIVATE "${er_root}/before-source/src")
      target_include_directories(test_empty_receiver_before PRIVATE "${CMAKE_SOURCE_DIR}/src/sim")
    else()
      target_sources(test_empty_receiver_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
      target_compile_definitions(test_empty_receiver_after PRIVATE EMPTY_RECEIVER_AFTER=1)
    endif()
    add_test(NAME empty_receiver_${side} COMMAND test_empty_receiver_${side} "${er_root}/${side}")
    set_tests_properties(empty_receiver_${side} PROPERTIES TIMEOUT 240
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  if(TARGET test_empty_receiver_before)
    add_test(NAME empty_receiver_identity COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${er_root}" -P "${CMAKE_SOURCE_DIR}/cmake/empty_receiver_identity.cmake")
    set_tests_properties(empty_receiver_before PROPERTIES DEPENDS empty_receiver_identity)
    set_tests_properties(empty_receiver_after PROPERTIES DEPENDS empty_receiver_before)
    # The hatch regression's two former saves must be reproduced with the
    # authentic entry simulation. Its normal before/after variants both use
    # current simulation and differ only in their historical startup main.
    if(EXISTS "${HATCH_TIMING_BASE}/src/app/main.cpp")
      add_executable(test_empty_receiver_hatch_before src/__test__/test_hatch_timing.cpp
        "${HATCH_TIMING_BASE}/src/app/main.cpp"
        "${er_root}/before-source/src/sim/World.cpp"
        "${er_root}/before-source/src/sim/RetailLevel.cpp"
        "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,sim.(World|RetailLevel).cpp.obj>")
      target_include_directories(test_empty_receiver_hatch_before BEFORE PRIVATE "${er_root}/before-source/src")
      target_include_directories(test_empty_receiver_hatch_before PRIVATE "${CMAKE_SOURCE_DIR}/src/sim")
      target_link_libraries(test_empty_receiver_hatch_before PRIVATE webolution_app
        webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
      target_compile_definitions(test_empty_receiver_hatch_before PRIVATE
        HATCH_TIMING_BEFORE=1 EMPTY_RECEIVER_BASELINE=1 WEBOLUTION_APP_NO_MAIN=1 WEBOLUTION_HAVE_SDL2=1)
      add_test(NAME empty_receiver_hatch_before COMMAND test_empty_receiver_hatch_before routes
        "${er_root}/hatch-before")
      set_tests_properties(empty_receiver_hatch_before PROPERTIES TIMEOUT 180 RUN_SERIAL TRUE
        DEPENDS "empty_receiver_identity;hatch_timing_identity"
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
    if(EXISTS "${er_root}/before-source/gadget-replay.cpp" AND
       EXISTS "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/gadget-seed/SavedGame.dat")
      add_executable(test_empty_receiver_gadget_before "${er_root}/before-source/gadget-replay.cpp"
        "${er_root}/before-source/src/sim/World.cpp" "${er_root}/before-source/src/sim/RetailLevel.cpp"
        "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,sim.(World|RetailLevel).cpp.obj>")
      target_include_directories(test_empty_receiver_gadget_before BEFORE PRIVATE "${er_root}/before-source/src")
      target_include_directories(test_empty_receiver_gadget_before PRIVATE "${CMAKE_SOURCE_DIR}/src/sim")
      target_link_libraries(test_empty_receiver_gadget_before PRIVATE webolution_app webolution_app_nomain
        webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
      add_test(NAME empty_receiver_gadget_before COMMAND test_empty_receiver_gadget_before
        "${CMAKE_SOURCE_DIR}/build/fluid-hazard-001/gadget-seed/SavedGame.dat" "${er_root}/gadget-before")
      set_tests_properties(empty_receiver_gadget_before PROPERTIES TIMEOUT 300
        DEPENDS "empty_receiver_identity;fluid_hazard_gadget_seed"
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endif()
endif()
