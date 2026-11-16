# Source-built production demonstrations; no installed executable is launched.
if(WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_texture_cache src/__test__/test_texture_cache.cpp)
  target_link_libraries(test_texture_cache PRIVATE webolution_objects)
  add_test(NAME frame_profile_texture_cache COMMAND test_texture_cache)
  set_tests_properties(frame_profile_texture_cache PROPERTIES TIMEOUT 30
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  # release-rate-control-001: retire the two legacy application frame hashes
  # that omit RATE; the texture-cache check remains.
endif()

# Optional control for measuring disabled instrumentation overhead. Replace only
# the six instrumented units; all other objects, flags and caches are identical.
# Excluded from the default build and verification workload.
if(WEBOLUTION_HAVE_SDL2)
  set(frp_plain "${CMAKE_CURRENT_BINARY_DIR}/frame-profile-compiled-out")
  configure_file("${CMAKE_SOURCE_DIR}/src/__test__/FrameProfileDisabled.h"
    "${frp_plain}/app/FrameProfile.h" COPYONLY)
  set(frp_plain_sources)
  foreach(source IN ITEMS app/main app/Raster app/SceneRaster app/TubeScene app/BackdropScene assets/Bitmap)
    get_filename_component(name "${source}" NAME)
    file(WRITE "${frp_plain}/${name}.cpp" "#include \"${CMAKE_SOURCE_DIR}/src/${source}.cpp\"\n")
    list(APPEND frp_plain_sources "${frp_plain}/${name}.cpp")
  endforeach()
  add_executable(webolution_profile_compiled_out EXCLUDE_FROM_ALL ${frp_plain_sources})
  target_include_directories(webolution_profile_compiled_out BEFORE PRIVATE "${frp_plain}" "${CMAKE_SOURCE_DIR}/src")
  target_compile_definitions(webolution_profile_compiled_out PRIVATE WEBOLUTION_HAVE_SDL2=1)
  target_sources(webolution_profile_compiled_out PRIVATE
    "$<FILTER:$<TARGET_OBJECTS:webolution_app>,EXCLUDE,/(Raster|SceneRaster|TubeScene|BackdropScene)[.]cpp[.](obj|o)$>"
    "$<FILTER:$<TARGET_OBJECTS:webolution_objects>,EXCLUDE,/Bitmap[.]cpp[.](obj|o)$>")
  target_link_libraries(webolution_profile_compiled_out PRIVATE
    webolution_texture_decoder webolution_sprite_resource webolution_compiled_level)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(webolution_profile_compiled_out PRIVATE stdc++exp)
  endif()
  if(TARGET SDL2::SDL2)
    target_link_libraries(webolution_profile_compiled_out PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(webolution_profile_compiled_out PRIVATE PkgConfig::SDL2)
  endif()
endif()
