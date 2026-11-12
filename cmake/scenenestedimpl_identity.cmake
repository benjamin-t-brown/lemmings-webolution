# scene-nested-impl-001 test: the frozen "before" sources must be the supervisor's task-entry
# files (build/agent-runs/<run>/before-source-hashes.json). Reads bytes only; any mismatch fails.
# Usage: cmake -DFROZEN_DIR=<dir> -DMANIFEST=<before-source-hashes.json> -P scenenestedimpl_identity.cmake
# FROZEN_DIR holds src/app and src/assets copies made before any production edit.
set(SNI_ITEMS
  "src/app/BackdropScene.cpp|7d54a42eb88b9ab173e8b9a1af4b7fe934749a64c05e762a447a104c3da91d7f"
  "src/app/BackdropScene.h|50259077fc1a8d34c762c4c2b057c5e6e7ae4277b22393b847da58d67ba57a4c"
  "src/app/SceneDispatch.cpp|92531486e74829550eff4e81f5aa64c95da3259c5cd8a62364031f4660d82db2"
  "src/app/SceneDispatch.h|1cd3338727854598a33bcc5ac0ca5990228e836e20b11a71099116f95a3aafd5"
  "src/assets/SceneNode.cpp|ac09bfeddfca64910f823bc0f23efb458d105b36527e8c9278fd72a0a4ae427d"
  "src/assets/SceneNode.h|752949113b2ffb88e23f54812349d47576ab73684d1d60f1b44c73f11d474019"
  "src/app/main.cpp|ccbd4a62f1ebf9ba5588c3508855286608b72d71f46259fb86b8d578f571c8fa")

set(_failed FALSE)
set(_manifest "")
if(EXISTS "${MANIFEST}")
  file(READ "${MANIFEST}" _manifest)
else()
  message(STATUS "manifest ${MANIFEST} not present; comparing against the recorded entry hashes only")
endif()

foreach(item IN LISTS SNI_ITEMS)
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
    message(STATUS "ok   frozen ${rel} sha256 ${got} == entry manifest ${from_manifest}")
  else()
    message(STATUS "FAIL frozen ${rel} sha256 ${got} != entry ${want}")
    set(_failed TRUE)
  endif()
endforeach()

if(_failed)
  message(FATAL_ERROR "scene-nested-impl-001: frozen sources do NOT match the supervisor entry manifest")
endif()
message(STATUS "RESULT: frozen sources match the supervisor entry manifest")
