if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  set(gg_root "${CMAKE_SOURCE_DIR}/build/gravity-gadgets-001")
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before" AND NOT EXISTS "${gg_root}/before-source/src/sim/World.cpp")
      continue()
    endif()
    add_executable(test_gravity_gadgets_${side} src/__test__/test_gravity_gadgets.cpp)
    target_include_directories(test_gravity_gadgets_${side} PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(test_gravity_gadgets_${side} PRIVATE
      webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
    if(side STREQUAL "before")
      target_sources(test_gravity_gadgets_before PRIVATE
        "$<FILTER:$<TARGET_OBJECTS:webolution_app>,EXCLUDE,app.BackdropScene.cpp.obj>"
        "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,sim.World.cpp.obj>"
        "${gg_root}/before-source/src/sim/World.cpp"
        "${gg_root}/before-source/src/app/BackdropScene.cpp"
        "${gg_root}/before-source/src/app/main.cpp")
      target_include_directories(test_gravity_gadgets_before BEFORE PRIVATE "${gg_root}/before-source/src")
      target_compile_definitions(test_gravity_gadgets_before PRIVATE GRAVITY_GADGETS_BEFORE=1 WEBOLUTION_APP_NO_MAIN=1 WEBOLUTION_HAVE_SDL2=1)
    else()
      target_sources(test_gravity_gadgets_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
      target_link_libraries(test_gravity_gadgets_after PRIVATE webolution_app webolution_app_nomain)
    endif()
    add_test(NAME gravity_gadgets_${side} COMMAND test_gravity_gadgets_${side} "${gg_root}/${side}")
    set_tests_properties(gravity_gadgets_${side} PROPERTIES TIMEOUT 240
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  if(TARGET test_gravity_gadgets_before)
    target_compile_definitions(test_gravity_gadgets_after PRIVATE GRAVITY_GADGETS_BASELINE_AVAILABLE=1)
    add_test(NAME gravity_gadgets_identity COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${gg_root}" -P "${CMAKE_SOURCE_DIR}/cmake/gravity_gadgets_identity.cmake")
    set_tests_properties(gravity_gadgets_identity PROPERTIES TIMEOUT 60)
    set_tests_properties(gravity_gadgets_before PROPERTIES DEPENDS gravity_gadgets_identity)
    set_tests_properties(gravity_gadgets_after PROPERTIES DEPENDS gravity_gadgets_before)
  endif()
endif()
