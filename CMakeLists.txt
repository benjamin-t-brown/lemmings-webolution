cmake_minimum_required(VERSION 3.28)

project(webolution
  VERSION 0.1.0
  DESCRIPTION "Open reimplementation of Lemmings Revolution"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

include(cmake/webolution_sources.cmake)

if(EMSCRIPTEN)
  add_compile_options(-fexceptions -pthread)
endif()

add_library(webolution_texture_decoder STATIC ${WEBOLUTION_TEXTURE_DECODER_SOURCES})
target_include_directories(webolution_texture_decoder PUBLIC
  ${WEBOLUTION_RETAIL_DIR}/include)

add_library(webolution_sprite_resource STATIC ${WEBOLUTION_SPRITE_RESOURCE_SOURCES})
target_include_directories(webolution_sprite_resource PUBLIC
  ${WEBOLUTION_RETAIL_DIR}/include)

# The compiled-level reader reuses the sprite reader's bounded stream parser.
add_library(webolution_compiled_level STATIC ${WEBOLUTION_COMPILED_LEVEL_SOURCES})
target_include_directories(webolution_compiled_level PUBLIC
  ${WEBOLUTION_RETAIL_DIR}/include)
target_link_libraries(webolution_compiled_level PUBLIC webolution_sprite_resource)

# ---- shared object library ------------------------------------------------
# One compile of the production sources, reused by every executable.

add_library(webolution_objects OBJECT ${WEBOLUTION_SOURCES})
target_include_directories(webolution_objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(webolution_objects PUBLIC
  webolution_texture_decoder webolution_sprite_resource webolution_compiled_level)

target_compile_options(webolution_objects PUBLIC
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>)

# GCC's std::print/println live in libstdc++exp until it migrates to the main
# library. Harmless to link when unused; required the moment anyone uses them.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(webolution_objects PUBLIC stdc++exp)
endif()

# ---- SDL2 -----------------------------------------------------------------
# Emscripten supplies its own SDL2 via -sUSE_SDL=2, so it is not searched for.

if(NOT EMSCRIPTEN)
  find_package(SDL2 QUIET)
  if(NOT SDL2_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(SDL2 IMPORTED_TARGET sdl2)
    endif()
  endif()
endif()

set(WEBOLUTION_HAVE_SDL2 FALSE)
if(EMSCRIPTEN OR SDL2_FOUND)
  set(WEBOLUTION_HAVE_SDL2 TRUE)
endif()

# Production audio: the mixer and the cue adapter are compiled into the shared
# object library, so SDL2 reaches every executable that links it. Emscripten
# supplies SDL2 headers and its port library through -sUSE_SDL=2.
if(WEBOLUTION_HAVE_SDL2)
  target_sources(webolution_objects PRIVATE ${WEBOLUTION_AUDIO_SOURCES})
  if(EMSCRIPTEN)
    target_compile_options(webolution_objects PUBLIC -sUSE_SDL=2)
  else()
    if(TARGET SDL2::SDL2)
      target_link_libraries(webolution_objects PUBLIC SDL2::SDL2)
    elseif(TARGET PkgConfig::SDL2)
      target_link_libraries(webolution_objects PUBLIC PkgConfig::SDL2)
    endif()
  endif()
endif()

include(cmake/webolution_app.cmake)

# ---- the game -------------------------------------------------------------
# The CPU renderer presents through SDL2 or writes a PNG directly (--shot).

add_executable(webolution src/app/main.cpp)
target_link_libraries(webolution PRIVATE webolution_app)
target_link_libraries(webolution PRIVATE webolution_objects)

if(WEBOLUTION_HAVE_SDL2)
  target_compile_definitions(webolution PRIVATE WEBOLUTION_HAVE_SDL2=1)
  if(TARGET SDL2::SDL2)
    target_link_libraries(webolution PRIVATE SDL2::SDL2)
  elseif(TARGET PkgConfig::SDL2)
    target_link_libraries(webolution PRIVATE PkgConfig::SDL2)
  endif()
else()
  target_compile_definitions(webolution PRIVATE WEBOLUTION_HAVE_SDL2=0)
endif()

if(EMSCRIPTEN)
  set(web_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/web")
  target_sources(webolution_objects PRIVATE "${web_dir}/WebMixer.cpp")
  target_sources(webolution PRIVATE "${web_dir}/Browser.cpp")
  set_target_properties(webolution PROPERTIES SUFFIX ".js")
  target_link_options(webolution PRIVATE
    -fexceptions -pthread -sUSE_SDL=2 -sASYNCIFY -sASYNCIFY_STACK_SIZE=262144
    # Preload before Start. The browser main thread waits for raster bands
    # synchronously and cannot handle thread startup messages at the barrier.
    "-sPTHREAD_POOL_SIZE=Math.max(0,Math.min(8,_emscripten_num_logical_cores()||1)-1)"
    -sPTHREAD_POOL_SIZE_STRICT=2
    -sSTACK_SIZE=4194304 -sALLOW_MEMORY_GROWTH -sENVIRONMENT=web,worker,node
    -sEXIT_RUNTIME=1 -lidbfs.js -lnodefs.js
    -sEXPORTED_FUNCTIONS=_main,_webolution_extract_rar20 -sEXPORTED_RUNTIME_METHODS=ccall,FS,IDBFS,ENV
    --pre-js "${web_dir}/node-mount.js"
    "--js-library=${web_dir}/web-audio.js")
  set(web_files "${web_dir}/webolution.html" "${web_dir}/web-game-data.mjs" "${web_dir}/isolate.js")
  set_property(TARGET webolution APPEND PROPERTY LINK_DEPENDS
    "${web_dir}/node-mount.js" "${web_dir}/web-audio.js" ${web_files})
  add_custom_command(TARGET webolution POST_BUILD
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/webolution.wasm" "${CMAKE_CURRENT_BINARY_DIR}/webolution.html"
      "${CMAKE_CURRENT_BINARY_DIR}/web-game-data.mjs" "${CMAKE_CURRENT_BINARY_DIR}/isolate.js"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${web_files} "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM)
endif()

option(WEBOLUTION_BUILD_TESTS "Build verification programs" ON)
if(WEBOLUTION_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/__test__")
  include(cmake/webolution_tests.cmake)
endif()
