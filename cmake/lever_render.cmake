if(TARGET fluid_kinds_support)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(lever_src "${CMAKE_SOURCE_DIR}/build/lever-render-001/frozen-render/src")
    else()
      set(lever_src "${CMAKE_SOURCE_DIR}/src")
    endif()
    if(EXISTS "${lever_src}/app/BackdropScene.cpp")
      add_executable(test_lever_render_${side} src/__test__/test_lever_render.cpp
        "${lever_src}/app/BackdropScene.cpp" "${lever_src}/app/main.cpp" src/app/TubeScene.cpp)
      target_include_directories(test_lever_render_${side} BEFORE PRIVATE "${lever_src}")
      target_link_libraries(test_lever_render_${side} PRIVATE fluid_kinds_support webolution_objects)
      target_compile_definitions(test_lever_render_${side} PRIVATE LEVER_RENDER_${side}=1)
      add_test(NAME lever_render_${side} COMMAND test_lever_render_${side}
        "${CMAKE_SOURCE_DIR}/build/lever-render-001/${side}")
      set_tests_properties(lever_render_${side} PROPERTIES TIMEOUT 180
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
      add_test(NAME lever_render_app_${side} COMMAND test_lever_render_${side}
        "${CMAKE_SOURCE_DIR}/build/lever-render-001/app-${side}" app)
      set_tests_properties(lever_render_app_${side} PROPERTIES TIMEOUT 180
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_lever_render_before)
    add_test(NAME lever_render_identity COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${CMAKE_SOURCE_DIR}/build/lever-render-001"
      -P "${CMAKE_SOURCE_DIR}/cmake/lever_render_identity.cmake")
    set_tests_properties(lever_render_before lever_render_app_before PROPERTIES DEPENDS lever_render_identity)
    target_compile_definitions(test_lever_render_after PRIVATE LEVER_BASELINE_AVAILABLE=1)
    set_tests_properties(lever_render_after PROPERTIES DEPENDS lever_render_before)
    set_tests_properties(lever_render_app_after PROPERTIES DEPENDS lever_render_app_before)
  endif()
endif()
