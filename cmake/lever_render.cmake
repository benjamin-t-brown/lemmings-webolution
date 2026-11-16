if(TARGET fluid_kinds_support)
  set(lever_src "${CMAKE_SOURCE_DIR}/src")
  if(EXISTS "${lever_src}/app/BackdropScene.cpp")
    add_executable(test_lever_render_after src/__test__/test_lever_render.cpp
      "${lever_src}/app/BackdropScene.cpp" "${lever_src}/app/main.cpp" src/app/TubeScene.cpp)
    target_include_directories(test_lever_render_after BEFORE PRIVATE "${lever_src}")
    target_link_libraries(test_lever_render_after PRIVATE fluid_kinds_support webolution_objects)
    target_compile_definitions(test_lever_render_after PRIVATE LEVER_RENDER_after=1)
    add_test(NAME lever_render_after COMMAND test_lever_render_after
      "${CMAKE_SOURCE_DIR}/build/lever-render-001/after")
    set_tests_properties(lever_render_after PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    add_test(NAME lever_render_app_after COMMAND test_lever_render_after
      "${CMAKE_SOURCE_DIR}/build/lever-render-001/app-after" app)
    set_tests_properties(lever_render_app_after PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
endif()
