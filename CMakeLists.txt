cmake_minimum_required(VERSION 3.28)

project(webolution
  VERSION 1.0.1
  DESCRIPTION "Open reimplementation of Lemmings Revolution"
  LANGUAGES CXX)

set(WEBOLUTION_VERSION "" CACHE STRING "Override the build version shown to players")
option(WEBOLUTION_MAIN_BUILD "Number builds from the first-parent history of main" OFF)
if(WEBOLUTION_VERSION STREQUAL "")
  set(WEBOLUTION_VERSION "${PROJECT_VERSION}")
  if(WEBOLUTION_MAIN_BUILD)
    find_package(Git REQUIRED)
    # The first commit on main after this baseline builds as 1.0.1.
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-list --first-parent --count
        d8f53e7d85d917090790c65d963e18223db0a799..HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE main_commits
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY)
    if(main_commits GREATER 0)
      math(EXPR build_patch "${PROJECT_VERSION_PATCH} + ${main_commits} - 1")
      set(WEBOLUTION_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${build_patch}")
    endif()
  endif()
endif()
if(NOT WEBOLUTION_VERSION MATCHES "^[0-9A-Za-z][0-9A-Za-z.+-]*$")
  message(FATAL_ERROR "WEBOLUTION_VERSION must contain only version characters")
endif()
add_compile_definitions(WEBOLUTION_VERSION_STRING="${WEBOLUTION_VERSION}")

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

# The native rasterizer uses std::thread and condition variables.
if(NOT EMSCRIPTEN)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(webolution_objects PUBLIC Threads::Threads)
endif()

# ---- SDL2 -----------------------------------------------------------------
# Emscripten supplies its own SDL2 via -sUSE_SDL=2, so it is not searched for.

if(NOT EMSCRIPTEN)
  # SDL_RenderGetWindow and SDL_HINT_VIDEODRIVER first appeared in SDL 2.0.22.
  set(WEBOLUTION_MIN_SDL2_VERSION 2.0.22)
  find_package(SDL2 ${WEBOLUTION_MIN_SDL2_VERSION} QUIET CONFIG)
  if(NOT TARGET SDL2::SDL2)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(SDL2 IMPORTED_TARGET "sdl2>=${WEBOLUTION_MIN_SDL2_VERSION}")
    endif()
  endif()
endif()

set(WEBOLUTION_HAVE_SDL2 FALSE)
if(EMSCRIPTEN OR TARGET SDL2::SDL2 OR TARGET PkgConfig::SDL2)
  set(WEBOLUTION_HAVE_SDL2 TRUE)
endif()
set_source_files_properties(src/app/GameDataLocation.cpp PROPERTIES
  COMPILE_DEFINITIONS WEBOLUTION_HAVE_SDL2=$<BOOL:${WEBOLUTION_HAVE_SDL2}>)

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
set(webolution_icon_png "${CMAKE_CURRENT_SOURCE_DIR}/src/app/icon/webolution.png")

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

if(NOT EMSCRIPTEN AND (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux"))
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/release/main/LICENSE")
    set(release_docs "${CMAKE_CURRENT_SOURCE_DIR}/release/main")
  else()
    set(release_docs "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()
  install(PROGRAMS "$<TARGET_FILE:webolution>" DESTINATION .
    RENAME "lemmings_webolution${CMAKE_EXECUTABLE_SUFFIX}")
endif()

if(WIN32 AND NOT EMSCRIPTEN)
  enable_language(RC)
  set(file_version "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
  if(WEBOLUTION_VERSION MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)([-+]|$)")
    set(version_components "${CMAKE_MATCH_1};${CMAKE_MATCH_2};${CMAKE_MATCH_3}")
    set(fits_file_version TRUE)
    foreach(component IN LISTS version_components)
      if(component GREATER 65535)
        set(fits_file_version FALSE)
      endif()
    endforeach()
    if(fits_file_version)
      list(JOIN version_components "," file_version)
      string(APPEND file_version ",0")
    endif()
  endif()
  configure_file(src/app/Version.rc.in webolution-version.rc @ONLY)
  target_sources(webolution PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/webolution-version.rc" src/app/webolution.rc)
  set_source_files_properties(src/app/webolution.rc PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/app/AppIconResource.h;${CMAKE_CURRENT_SOURCE_DIR}/src/app/icon/webolution.ico")
  if(WEBOLUTION_HAVE_SDL2)
    target_sources(webolution PRIVATE src/app/WindowsWindowIcon.cpp)
    target_compile_definitions(webolution PRIVATE WEBOLUTION_WINDOWS_ICON=1)
  endif()

  get_filename_component(toolchain_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
  string(CONFIGURE [=[
    if(POLICY CMP0207)
      cmake_policy(SET CMP0207 NEW)
    endif()
    set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL "objdump")
    set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND "@CMAKE_OBJDUMP@")
    file(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES "$<TARGET_FILE:webolution>"
      DIRECTORIES "@toolchain_bin@"
      RESOLVED_DEPENDENCIES_VAR runtime_dlls
      PRE_EXCLUDE_REGEXES "^api-ms-" "^ext-ms-"
      POST_EXCLUDE_REGEXES "[/\\\\][Ss][Yy][Ss][Tt][Ee][Mm]32[/\\\\]")
    if(runtime_dlls)
      file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE FILE FILES ${runtime_dlls})
    endif()
  ]=] runtime_install_code @ONLY)
  install(CODE "${runtime_install_code}")
  install(FILES "${release_docs}/packaging/windows/README.txt" "${release_docs}/LICENSE"
    DESTINATION .)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  install(FILES "${release_docs}/packaging/linux/README.txt" "${release_docs}/LICENSE"
    DESTINATION .)
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
  set(loading_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/app/icon/loading")
  list(APPEND web_files "${loading_dir}/basher.gif" "${loading_dir}/blocker.gif")
  set_property(TARGET webolution APPEND PROPERTY LINK_DEPENDS
    "${web_dir}/node-mount.js" "${web_dir}/web-audio.js" ${web_files} "${webolution_icon_png}")
  add_custom_command(TARGET webolution POST_BUILD
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/webolution.wasm" "${CMAKE_CURRENT_BINARY_DIR}/webolution.html"
      "${CMAKE_CURRENT_BINARY_DIR}/web-game-data.mjs" "${CMAKE_CURRENT_BINARY_DIR}/isolate.js"
      "${CMAKE_CURRENT_BINARY_DIR}/favicon.png"
      "${CMAKE_CURRENT_BINARY_DIR}/basher.gif" "${CMAKE_CURRENT_BINARY_DIR}/blocker.gif"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${web_files} "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${webolution_icon_png}" "${CMAKE_CURRENT_BINARY_DIR}/favicon.png"
    VERBATIM)
endif()

option(WEBOLUTION_BUILD_TESTS "Build verification programs" ON)
if(WEBOLUTION_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/__test__")
  include(cmake/webolution_tests.cmake)
endif()
