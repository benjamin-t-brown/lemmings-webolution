# Source-built production demonstrations; no installed executable is launched.
if(WEBOLUTION_HAVE_SDL2 AND DEFINED WEBOLUTION_GAME_DIR)
  add_executable(test_texture_cache src/__test__/test_texture_cache.cpp)
  target_link_libraries(test_texture_cache PRIVATE webolution_objects)
  add_test(NAME frame_profile_texture_cache COMMAND test_texture_cache)
  set_tests_properties(frame_profile_texture_cache PROPERTIES TIMEOUT 30
    ENVIRONMENT "WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  foreach(level IN ITEMS MM_CLIMB_002 MM_W_012)
    if(level STREQUAL "MM_CLIMB_002")
      set(expected 61933b158580b1c869dd545500c7592b02c26cc36bd67ae2d93ec79cec4413ba)
    else()
      set(expected 3633704696154420f7a8edf02eb20fe109c6e32669867e20a0f8a1feca8c4b40)
    endif()
    add_test(NAME frame_profile_${level} COMMAND ${CMAKE_COMMAND}
      "-DCLONE=$<TARGET_FILE:webolution>" "-DGAME_DIR=${WEBOLUTION_GAME_DIR}"
      "-DLEVEL=${level}" "-DEXPECTED=${expected}"
      "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/frame-rate-profile/${level}"
      -P "${CMAKE_SOURCE_DIR}/cmake/frame_rate_profile_run.cmake")
    set_tests_properties(frame_profile_${level} PROPERTIES TIMEOUT 60 RUN_SERIAL TRUE)
  endforeach()
endif()

# Retained pre-task clone sources supply an independent differential oracle.
# A fresh checkout still runs both fixed-hash production demonstrations above.
set(frp_reference "${CMAKE_SOURCE_DIR}/build/frame-rate-profile-001/entry-source/src")
if(EXISTS "${frp_reference}/app/Raster.cpp")
  file(READ "${CMAKE_SOURCE_DIR}/build/frame-rate-profile-001/entry-sha256.json" frp_manifest)
  foreach(name IN ITEMS Raster.cpp Raster.h SceneRaster.cpp SceneRaster.h)
    file(SHA256 "${frp_reference}/app/${name}" frp_actual)
    string(JSON frp_expected ERROR_VARIABLE frp_key_error GET "${frp_manifest}" "src/app/${name}")
    if(frp_key_error)
      string(JSON frp_expected GET "${frp_manifest}" "src\\app\\${name}")
    endif()
    if(NOT frp_actual STREQUAL frp_expected)
      message(FATAL_ERROR "Frame-profile frozen clone source changed: ${name}")
    endif()
  endforeach()
  add_library(frame_profile_reference STATIC "${frp_reference}/app/Raster.cpp" "${frp_reference}/app/SceneRaster.cpp")
  target_include_directories(frame_profile_reference PRIVATE "${frp_reference}")
  target_compile_definitions(frame_profile_reference PRIVATE webo=webo_frame_reference)
  # Identical #pragma-once headers can be deduplicated by GCC across paths.
  # Generate the reference declarations without that directive, leaving the
  # authenticated retained files untouched.
  file(READ "${frp_reference}/app/Raster.h" frp_raster_header)
  file(READ "${frp_reference}/app/SceneRaster.h" frp_scene_header)
  string(REPLACE "#pragma once" "" frp_raster_header "${frp_raster_header}")
  string(REPLACE "#pragma once" "" frp_scene_header "${frp_scene_header}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/frame_profile_reference.h"
    "#define webo webo_frame_reference\n${frp_raster_header}\n${frp_scene_header}\n#undef webo\n")
  add_executable(test_frame_raster_equivalence src/__test__/test_frame_raster_equivalence.cpp src/app/Raster.cpp src/app/SceneRaster.cpp)
  target_include_directories(test_frame_raster_equivalence PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}")
  target_link_libraries(test_frame_raster_equivalence PRIVATE frame_profile_reference)
  add_test(NAME frame_profile_raster_equivalence COMMAND test_frame_raster_equivalence)
  set_tests_properties(frame_profile_raster_equivalence PROPERTIES TIMEOUT 60)
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
