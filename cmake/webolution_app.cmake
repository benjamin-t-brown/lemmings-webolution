# The executable owns its entry point. Application code is compiled separately.
set(WEBOLUTION_APP_SOURCES ${WEBOLUTION_GFX_SOURCES}
  "${CMAKE_SOURCE_DIR}/src/app/SceneVertexPack.cpp"
  "${CMAKE_SOURCE_DIR}/src/app/SceneDiffuse.cpp"
  "${CMAKE_SOURCE_DIR}/src/app/SceneDepth.cpp"
  "${CMAKE_SOURCE_DIR}/src/app/SceneDispatch.cpp")
list(REMOVE_ITEM WEBOLUTION_APP_SOURCES "${CMAKE_SOURCE_DIR}/src/app/main.cpp")

add_library(webolution_app OBJECT ${WEBOLUTION_APP_SOURCES})
target_link_libraries(webolution_app PUBLIC webolution_objects)
target_compile_definitions(webolution_app PRIVATE
  WEBOLUTION_HAVE_SDL2=$<BOOL:${WEBOLUTION_HAVE_SDL2}>)
