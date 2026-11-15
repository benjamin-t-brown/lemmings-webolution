if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_frame_consumption src/__test__/test_frame_consumption.cpp)
  target_link_libraries(test_frame_consumption PRIVATE
    webolution_app webolution_app_nomain webolution_objects)
  target_compile_definitions(test_frame_consumption PRIVATE WEBOLUTION_HAVE_SDL2=1)
  add_test(NAME frame_consumption COMMAND test_frame_consumption
    "${CMAKE_CURRENT_BINARY_DIR}/frame-consumption-test")
  set_tests_properties(frame_consumption PROPERTIES TIMEOUT 120
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")

  # Optional worker evidence targets: the authentic task-entry production loop,
  # linked to the same final drivers and unchanged scene/simulation objects.
  # They are not part of ALL or ordinary verification (which would undo the
  # speed gain). Existing frozen artifacts are never replaced on resume.
  set(consumption_entry "${CMAKE_SOURCE_DIR}/build/test-render-skip-001/instrumented-entry-main.cpp")
  if(EXISTS "${consumption_entry}")
    add_library(render_skip_entry_main OBJECT EXCLUDE_FROM_ALL "${consumption_entry}")
    target_link_libraries(render_skip_entry_main PRIVATE webolution_objects)
    target_compile_definitions(render_skip_entry_main PRIVATE
      WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
    foreach(driver IN ITEMS levelsweep hatch_timing_after empty_receiver_before empty_receiver_after)
      if(NOT TARGET test_${driver})
        continue()
      endif()
      get_target_property(consumption_sources test_${driver} SOURCES)
      get_target_property(consumption_links test_${driver} LINK_LIBRARIES)
      get_target_property(consumption_defines test_${driver} COMPILE_DEFINITIONS)
      get_target_property(consumption_includes test_${driver} INCLUDE_DIRECTORIES)
      list(REMOVE_ITEM consumption_sources "$<TARGET_OBJECTS:webolution_app_nomain>")
      list(REMOVE_ITEM consumption_links webolution_app_nomain)
      add_executable(render_skip_reference_${driver} EXCLUDE_FROM_ALL ${consumption_sources})
      target_link_libraries(render_skip_reference_${driver} PRIVATE ${consumption_links} render_skip_entry_main)
      if(consumption_defines)
        target_compile_definitions(render_skip_reference_${driver} PRIVATE ${consumption_defines})
      endif()
      if(consumption_includes)
        target_include_directories(render_skip_reference_${driver} PRIVATE ${consumption_includes})
      endif()
    endforeach()
  endif()
endif()
