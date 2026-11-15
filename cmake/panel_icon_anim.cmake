if(TARGET fluid_kinds_support)
  # The support archive reuses shared application objects; the panel
  # sources below supply the authentic frozen/current overrides.
  add_library(panel_icon_support INTERFACE)
  target_link_libraries(panel_icon_support INTERFACE fluid_kinds_support webolution_objects)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(panel_src "${CMAKE_SOURCE_DIR}/build/panel-icon-anim-001/frozen/src")
    else()
      set(panel_src "${CMAKE_SOURCE_DIR}/src")
    endif()
    if(EXISTS "${panel_src}/app/SkillPanel.cpp")
      add_executable(test_panel_icon_${side} src/__test__/test_panel_icon_anim.cpp
        "${panel_src}/app/SkillPanel.cpp" "${panel_src}/app/RuntimeSkillPanel.cpp"
        "${panel_src}/app/main.cpp" src/app/BackdropScene.cpp src/app/TubeScene.cpp)
      target_include_directories(test_panel_icon_${side} BEFORE PRIVATE "${panel_src}")
      target_link_libraries(test_panel_icon_${side} PRIVATE panel_icon_support)
      target_compile_definitions(test_panel_icon_${side} PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
      if(side STREQUAL "before")
        target_compile_definitions(test_panel_icon_before PRIVATE PANEL_ICON_BEFORE=1)
      endif()
      add_test(NAME panel_icon_${side} COMMAND test_panel_icon_${side}
        "${CMAKE_SOURCE_DIR}/build/panel-icon-anim-001/${side}")
      set_tests_properties(panel_icon_${side} PROPERTIES TIMEOUT 120
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_panel_icon_before)
    target_compile_definitions(test_panel_icon_after PRIVATE PANEL_ICON_BASELINE_AVAILABLE=1)
    add_test(NAME panel_icon_identity COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${CMAKE_SOURCE_DIR}/build/panel-icon-anim-001"
      -P "${CMAKE_SOURCE_DIR}/cmake/panel_icon_identity.cmake")
    set_tests_properties(panel_icon_before PROPERTIES DEPENDS panel_icon_identity)
    set_tests_properties(panel_icon_after PROPERTIES DEPENDS panel_icon_before)
  endif()
endif()
