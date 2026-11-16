if(TARGET fluid_kinds_support)
  add_library(antigrav_support INTERFACE)
  target_link_libraries(antigrav_support INTERFACE fluid_kinds_support webolution_objects)
  set(ag_src "${CMAKE_SOURCE_DIR}/src")
  if(EXISTS "${ag_src}/app/TubeScene.cpp")
    add_executable(test_antigrav_after src/__test__/test_antigrav_presentation.cpp
      "${ag_src}/app/TubeScene.cpp" "${ag_src}/app/ActorSprite.cpp"
      "${ag_src}/app/main.cpp" src/app/BackdropScene.cpp)
    target_include_directories(test_antigrav_after BEFORE PRIVATE "${ag_src}")
    target_include_directories(test_antigrav_after PRIVATE "${CMAKE_SOURCE_DIR}/src/app")
    target_link_libraries(test_antigrav_after PRIVATE antigrav_support)
    target_compile_definitions(test_antigrav_after PRIVATE WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
    # release-rate-control-001: legacy antigrav_after compares frames
    # against a pre-RATE panel. Retire that check; retain original evidence.
  endif()
endif()
