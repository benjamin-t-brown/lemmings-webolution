if(TARGET fluid_kinds_support)
  # Reuse application objects, overriding only the two task-entry render files.
  add_library(antigrav_support INTERFACE)
  target_link_libraries(antigrav_support INTERFACE fluid_kinds_support webolution_objects)
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before")
      set(ag_src "${CMAKE_SOURCE_DIR}/build/antigrav-presentation-001/frozen/src")
    else()
      set(ag_src "${CMAKE_SOURCE_DIR}/src")
    endif()
    if(EXISTS "${ag_src}/app/TubeScene.cpp")
      add_executable(test_antigrav_${side} src/__test__/test_antigrav_presentation.cpp
        "${ag_src}/app/TubeScene.cpp" "${ag_src}/app/ActorSprite.cpp"
        "${ag_src}/app/main.cpp" src/app/BackdropScene.cpp)
      target_include_directories(test_antigrav_${side} BEFORE PRIVATE "${ag_src}")
      target_include_directories(test_antigrav_${side} PRIVATE "${CMAKE_SOURCE_DIR}/src/app")
      target_link_libraries(test_antigrav_${side} PRIVATE antigrav_support)
      target_compile_definitions(test_antigrav_${side} PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
      if(side STREQUAL "before")
        target_compile_definitions(test_antigrav_before PRIVATE ANTIGRAV_BEFORE=1)
      endif()
      add_test(NAME antigrav_${side} COMMAND test_antigrav_${side}
        "${CMAKE_SOURCE_DIR}/build/antigrav-presentation-001/${side}")
      set_tests_properties(antigrav_${side} PROPERTIES TIMEOUT 240
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    endif()
  endforeach()
  if(TARGET test_antigrav_before)
    target_compile_definitions(test_antigrav_after PRIVATE ANTIGRAV_BASELINE_AVAILABLE=1)
    add_test(NAME antigrav_identity COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${CMAKE_SOURCE_DIR}/build/antigrav-presentation-001"
      -P "${CMAKE_SOURCE_DIR}/cmake/antigrav_identity.cmake")
    set_tests_properties(antigrav_before PROPERTIES DEPENDS antigrav_identity)
    set_tests_properties(antigrav_after PROPERTIES DEPENDS antigrav_before)
  endif()
endif()
