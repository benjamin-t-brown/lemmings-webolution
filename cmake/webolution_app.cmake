# Current application translation units have no test-private preprocessor inputs.
# Keep main separate: the executable owns its entry point; drivers use runApplication.
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

add_library(webolution_app_nomain OBJECT "${CMAKE_SOURCE_DIR}/src/app/main.cpp")
target_link_libraries(webolution_app_nomain PUBLIC webolution_objects)
target_compile_definitions(webolution_app_nomain PRIVATE
  WEBOLUTION_HAVE_SDL2=$<BOOL:${WEBOLUTION_HAVE_SDL2}> WEBOLUTION_APP_NO_MAIN=1)

# Share only the source subset requested by a target. A frozen renderer/main or
# unit-test subset must never acquire the current replacement as well. Selecting
# objects (rather than linking all of webolution_app) preserves that boundary and
# avoids introducing unrelated dependencies into the small unit executables.
function(webolution_share_app_sources target)
  get_target_property(sources ${target} SOURCES)
  set(private_sources)
  set(shared_names)
  set(share_main FALSE)
  set(frozen_dirs)
  foreach(source IN LISTS sources)
    if(source MATCHES "^\\$<")
      list(APPEND private_sources "${source}")
      continue()
    endif()
    get_filename_component(absolute "${source}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
    if(absolute IN_LIST WEBOLUTION_APP_SOURCES)
      get_filename_component(name "${absolute}" NAME_WE)
      list(APPEND shared_names "${name}")
    elseif(absolute STREQUAL "${CMAKE_SOURCE_DIR}/src/app/main.cpp" AND NOT target STREQUAL "webolution")
      set(share_main TRUE)
    else()
      list(APPEND private_sources "${source}")
      if(absolute MATCHES "^${CMAKE_SOURCE_DIR}/build/")
        get_filename_component(directory "${absolute}" DIRECTORY)
        list(APPEND frozen_dirs "${directory}")
      endif()
    endif()
  endforeach()
  if(NOT shared_names AND NOT share_main)
    return()
  endif()

  # Frozen include roots can change class layout or inline code even in a .cpp
  # still taken from src/. Conservatively retain the entire target in that case.
  # Support libraries use current headers independently and remain shareable.
  get_target_property(includes ${target} INCLUDE_DIRECTORIES)
  foreach(directory IN LISTS includes)
    if(directory MATCHES "^${CMAKE_SOURCE_DIR}/build/")
      list(APPEND frozen_dirs "${directory}")
    endif()
  endforeach()
  list(REMOVE_DUPLICATES frozen_dirs)
  set(differences)
  foreach(directory IN LISTS frozen_dirs)
    file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${directory}/*.h" "${directory}/*.hpp")
    foreach(header IN LISTS headers)
      file(RELATIVE_PATH relative "${directory}" "${header}")
      set(matched FALSE)
      foreach(prefix IN ITEMS src src/app src/assets src/sim)
        set(current "${CMAKE_SOURCE_DIR}/${prefix}/${relative}")
        if(EXISTS "${current}")
          set(matched TRUE)
          set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${header}" "${current}")
          file(SHA256 "${header}" frozen_hash)
          file(SHA256 "${current}" current_hash)
          file(APPEND "${CMAKE_BINARY_DIR}/webolution-app-sharing-headers.tsv"
            "${target}\t${header}\t${current}\t${frozen_hash}\t${current_hash}\n")
          if(NOT frozen_hash STREQUAL current_hash)
            list(APPEND differences "${header}")
          endif()
          break()
        endif()
      endforeach()
      if(NOT matched)
        list(APPEND differences "${header} (no current counterpart)")
      endif()
    endforeach()
  endforeach()
  if(differences)
    list(REMOVE_DUPLICATES differences)
    file(APPEND "${CMAKE_BINARY_DIR}/webolution-app-sharing.txt"
      "RETAIN ${target}: differing frozen headers: ${differences}\n")
    return()
  endif()

  set_property(TARGET ${target} PROPERTY SOURCES "${private_sources}")
  get_target_property(type ${target} TYPE)
  if(type STREQUAL "STATIC_LIBRARY")
    set_property(TARGET ${target} PROPERTY LINKER_LANGUAGE CXX)
  endif()
  if(shared_names)
    list(JOIN shared_names "|" names)
    target_sources(${target} PRIVATE
      "$<FILTER:$<TARGET_OBJECTS:webolution_app>,INCLUDE,/(${names})[.]cpp[.](obj|o)$>")
  endif()
  if(share_main)
    target_sources(${target} PRIVATE $<TARGET_OBJECTS:webolution_app_nomain>)
  endif()
  # Only usage requirements travel here; the base objects are still selected by
  # the target's existing link declarations (including frozen base variants).
  target_include_directories(${target} PRIVATE $<TARGET_PROPERTY:webolution_objects,INTERFACE_INCLUDE_DIRECTORIES>)
  file(APPEND "${CMAKE_BINARY_DIR}/webolution-app-sharing.txt"
    "SHARE ${target}: ${shared_names}; main=${share_main}\n")
endfunction()

# Called after all task modules, when their frozen include paths are known.
function(webolution_share_registered_app_sources)
  file(WRITE "${CMAKE_BINARY_DIR}/webolution-app-sharing.txt" "Application object sharing\n")
  file(WRITE "${CMAKE_BINARY_DIR}/webolution-app-sharing-headers.tsv"
    "target\tfrozen_header\tcurrent_header\tfrozen_sha256\tcurrent_sha256\n")
  get_property(targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
  foreach(target IN LISTS targets)
    if(target MATCHES "^webolution_app($|_)")
      continue()
    endif()
    get_target_property(type ${target} TYPE)
    if(type MATCHES "^(EXECUTABLE|STATIC_LIBRARY)$")
      webolution_share_app_sources(${target})
    endif()
  endforeach()
endfunction()
