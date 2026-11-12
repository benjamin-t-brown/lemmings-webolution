# scene-branch-impl-001 test: the frozen "before" sources must be the supervisor's task-entry
# files. Reads bytes only; any mismatch fails. Usage:
#   cmake -DFROZEN_DIR=<dir> -DMANIFEST=<before-source-hashes.json> -P scenebranchimpl_identity.cmake
# FROZEN_DIR holds src/assets and src/app copies made before any production edit.
set(SBI_ITEMS
  "src/assets/SceneNode.cpp|9e053b932ef7f38e24bf5f5939c6d29ec36209b724cd61ad7a6ca1f1e7b40485"
  "src/assets/SceneNode.h|9edf809ebb8f2f0f296728c75305933006d5e2ecc94902af4cd9435a7423c8a3"
  "src/app/SceneBillboardSource.cpp|17e8bbcf817f84235d0bf9a8711c18f6ba10249db3514eba5042d5d5f24626ae"
  "src/app/SceneBillboardSource.h|83494ea00acbe1e17ac160f388a6c009635a30cf21490f683ce292c0327ef8e2"
  "src/app/BackdropScene.cpp|682fe6935743656c08b4bd9fb67565b19fdde26153f69c9b0e97c228e8e3ed94"
  "src/app/BackdropScene.h|6f530a25654a8ca278314475ced83fe4a7e8e326324adea4ec26d1e3dfc7c11c"
  "src/app/main.cpp|a2789bdf6ed1f4820371f41dc89c84fe93e513e6950ed4b1e3b7a75ae6656c94")

set(_failed FALSE)
set(_manifest "")
if(EXISTS "${MANIFEST}")
  file(READ "${MANIFEST}" _manifest)
else()
  message(STATUS "manifest ${MANIFEST} not present; comparing against the recorded entry hashes only")
endif()

foreach(item IN LISTS SBI_ITEMS)
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
  message(FATAL_ERROR "scene-branch-impl-001: frozen sources do NOT match the supervisor entry manifest")
endif()
message(STATUS "RESULT: frozen sources match the supervisor entry manifest")
