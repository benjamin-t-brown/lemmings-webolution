# fastforward-impl-001 test: the frozen "before" sources must be the supervisor's task-entry
# files (build/agent-runs/<run>/before-source-hashes.json, copied to entry-manifest.json).
# Reads bytes only; any mismatch fails.
# Usage: cmake -DFROZEN_DIR=<dir> -DMANIFEST=<before-source-hashes.json> -P fastforwardimpl_identity.cmake
# FROZEN_DIR holds src/app copies made before any production edit.
set(FFI_ITEMS
  "src/app/main.cpp|ccbd4a62f1ebf9ba5588c3508855286608b72d71f46259fb86b8d578f571c8fa"
  "src/app/RuntimeHud.cpp|14a7d7aaecad21c49d434590f441c5eb6dc444ff81b48bd94065c6571af153f6"
  "src/app/RuntimeHud.h|9848ba082e1c33eecb28860d0e4739c52a080fb190cf26c8e56c3bcfb5974fbc"
  "src/app/RuntimeSkillPanel.cpp|6526d8280f1fc95a86886f3cf8b3c8b83b3cde9e28560fc78071ee644e08db4b"
  "src/app/RuntimeSkillPanel.h|2779294053fed4a1740c005702de6a72602c0988076d10b774eef246166c77f0")

set(_failed FALSE)
set(_manifest "")
if(EXISTS "${MANIFEST}")
  file(READ "${MANIFEST}" _manifest)
else()
  message(STATUS "manifest ${MANIFEST} not present; comparing against the recorded entry hashes only")
endif()

foreach(item IN LISTS FFI_ITEMS)
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
  message(FATAL_ERROR "fastforward-impl-001: frozen sources do NOT match the supervisor entry manifest")
endif()
message(STATUS "RESULT: frozen sources match the supervisor entry manifest")
