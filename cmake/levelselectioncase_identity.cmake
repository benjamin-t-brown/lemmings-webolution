# level-selection-case-001 test: the frozen "before" sources must be the task-entry production
# files. The recorded entry hashes are the production hashes in the last retained supervisor
# snapshot before this task (loop-4070ec7c9130425f9242b8cbc09c0fab, review 0004-review-af050c,
# before-source-hashes.json: the accepted playable-baseline-001 state, which changed no production
# file). The frozen copies were written from the working tree before any edit. Reads bytes only; any
# mismatch fails.
# Usage: cmake -DFROZEN_DIR=<dir> -DMANIFEST=<before-source-hashes.json> -P levelselectioncase_identity.cmake
set(LSC_ITEMS
  "src/app/main.cpp|ec816679d261f464f19b947ce0b37f948346899de0f668ca79bcf59b977bddd7"
  "src/sim/RetailLevel.cpp|bd9dd9e724f6fdca564c4dcc9ff88d9749ab35c7e824a29efc1572b3a7982eab")

set(_failed FALSE)
set(_manifest "")
if(EXISTS "${MANIFEST}")
  file(READ "${MANIFEST}" _manifest)
else()
  message(STATUS "manifest ${MANIFEST} not present; comparing against the recorded entry hashes only")
endif()

foreach(item IN LISTS LSC_ITEMS)
  string(REPLACE "|" ";" parts "${item}")
  list(GET parts 0 rel)
  list(GET parts 1 want)
  if(NOT EXISTS "${FROZEN_DIR}/${rel}")
    message(STATUS "FAIL ${rel}: frozen copy missing")
    set(_failed TRUE)
    continue()
  endif()
  file(SHA256 "${FROZEN_DIR}/${rel}" got)
  set(from_manifest "(manifest absent)")
  if(NOT _manifest STREQUAL "")
    string(JSON from_manifest GET "${_manifest}" "${rel}")
    if(NOT from_manifest STREQUAL want)
      message(STATUS "FAIL ${rel}: recorded entry hash ${want} differs from manifest ${from_manifest}")
      set(_failed TRUE)
    endif()
  endif()
  if(got STREQUAL want)
    message(STATUS "ok   frozen ${rel} sha256 ${got} == entry hash (manifest ${from_manifest})")
  else()
    message(STATUS "FAIL frozen ${rel} sha256 ${got} != entry ${want}")
    set(_failed TRUE)
  endif()
endforeach()

if(_failed)
  message(FATAL_ERROR "level-selection-case-001: frozen sources do NOT match the task-entry hashes")
endif()
message(STATUS "RESULT: frozen sources match the task-entry hashes")
