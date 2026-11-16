if(TARGET fluid_kinds_support)
  add_library(panel_icon_support INTERFACE)
  target_link_libraries(panel_icon_support INTERFACE fluid_kinds_support webolution_objects)
  set(panel_src "${CMAKE_SOURCE_DIR}/src")
  if(EXISTS "${panel_src}/app/SkillPanel.cpp")
    add_executable(test_panel_icon_after src/__test__/test_panel_icon_anim.cpp
      "${panel_src}/app/SkillPanel.cpp" "${panel_src}/app/RuntimeSkillPanel.cpp"
      "${panel_src}/app/main.cpp" src/app/BackdropScene.cpp src/app/TubeScene.cpp)
    target_include_directories(test_panel_icon_after BEFORE PRIVATE "${panel_src}")
    target_link_libraries(test_panel_icon_after PRIVATE panel_icon_support)
    target_compile_definitions(test_panel_icon_after PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
    add_test(NAME panel_icon_after COMMAND test_panel_icon_after
      "${CMAKE_SOURCE_DIR}/build/panel-icon-anim-001/after")
    set_tests_properties(panel_icon_after PROPERTIES TIMEOUT 120
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
endif()
