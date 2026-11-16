if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  set(ags_root "${CMAKE_SOURCE_DIR}/build/antigrav-skill-assign-001")
  foreach(side IN ITEMS before after)
    if(side STREQUAL "before" AND NOT EXISTS "${ags_root}/before-source/src/sim/World.cpp")
      continue()
    endif()
    add_executable(test_antigrav_skill_${side} src/__test__/test_antigrav_skill_assign.cpp)
    target_include_directories(test_antigrav_skill_${side} PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/src/sim")
    target_link_libraries(test_antigrav_skill_${side} PRIVATE webolution_app webolution_app_nomain
      webolution_texture_decoder webolution_sprite_resource webolution_compiled_level SDL2::SDL2 stdc++exp)
    if(side STREQUAL "before")
      target_sources(test_antigrav_skill_before PRIVATE
        "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,sim.World.cpp.obj>"
        "${ags_root}/before-source/src/sim/World.cpp")
      target_compile_definitions(test_antigrav_skill_before PRIVATE ANTIGRAV_SKILL_BEFORE=1)
    else()
      target_sources(test_antigrav_skill_after PRIVATE $<TARGET_OBJECTS:webolution_objects>)
    endif()
    add_test(NAME antigrav_skill_${side} COMMAND test_antigrav_skill_${side}
      "${CMAKE_CURRENT_BINARY_DIR}/antigrav-skill-${side}")
    set_tests_properties(antigrav_skill_${side} PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endforeach()
  if(TARGET test_antigrav_skill_before)
    add_test(NAME antigrav_skill_identity COMMAND ${CMAKE_COMMAND}
      "-DTASK_DIR=${ags_root}" -P "${CMAKE_SOURCE_DIR}/cmake/antigrav_skill_identity.cmake")
    set_tests_properties(antigrav_skill_identity PROPERTIES TIMEOUT 30)
    set_tests_properties(antigrav_skill_before PROPERTIES DEPENDS antigrav_skill_identity)
    set_tests_properties(antigrav_skill_after PROPERTIES DEPENDS antigrav_skill_before)
    add_test(NAME antigrav_skill_normal_unchanged COMMAND ${CMAKE_COMMAND} -E compare_files
      "${CMAKE_CURRENT_BINARY_DIR}/antigrav-skill-before/normal-gravity.txt"
      "${CMAKE_CURRENT_BINARY_DIR}/antigrav-skill-after/normal-gravity.txt")
    set_tests_properties(antigrav_skill_normal_unchanged PROPERTIES TIMEOUT 30
      DEPENDS "antigrav_skill_before;antigrav_skill_after")
  endif()
endif()
