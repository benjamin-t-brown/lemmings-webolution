if(WEBOLUTION_HAVE_SDL2 AND NOT EMSCRIPTEN AND DEFINED WEBOLUTION_GAME_DIR)
  set(fluid_support ${WEBOLUTION_GFX_SOURCES})
  list(REMOVE_ITEM fluid_support "${CMAKE_SOURCE_DIR}/src/app/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/BackdropScene.cpp" "${CMAKE_SOURCE_DIR}/src/app/TubeScene.cpp")
  add_library(fluid_kinds_support STATIC ${fluid_support}
    src/app/SceneVertexPack.cpp src/app/SceneDiffuse.cpp src/app/SceneDepth.cpp src/app/SceneDispatch.cpp)
  target_link_libraries(fluid_kinds_support PUBLIC webolution_objects)
  target_compile_definitions(fluid_kinds_support PUBLIC WEBOLUTION_HAVE_SDL2=1 WEBOLUTION_APP_NO_MAIN=1)
  set(fluid_src "${CMAKE_SOURCE_DIR}/src")
  if(EXISTS "${fluid_src}/app/BackdropScene.cpp")
    add_executable(test_fluid_kinds_after src/__test__/test_fluid_kinds.cpp
      "${fluid_src}/app/BackdropScene.cpp" "${fluid_src}/app/TubeScene.cpp")
    target_include_directories(test_fluid_kinds_after BEFORE PRIVATE "${fluid_src}")
    target_link_libraries(test_fluid_kinds_after PRIVATE fluid_kinds_support webolution_objects)
    target_compile_definitions(test_fluid_kinds_after PRIVATE FLUID_KINDS_after=1)
    target_sources(test_fluid_kinds_after PRIVATE src/app/main.cpp)
    add_test(NAME fluid_kinds_app COMMAND test_fluid_kinds_after public
      "${CMAKE_SOURCE_DIR}/build/fluid-kinds-001/app")
    set_tests_properties(fluid_kinds_app PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
    add_test(NAME fluid_kinds_after COMMAND test_fluid_kinds_after after
      "${CMAKE_SOURCE_DIR}/build/fluid-kinds-001/after")
    set_tests_properties(fluid_kinds_after PROPERTIES TIMEOUT 180
      ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  endif()
endif()
