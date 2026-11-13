# Authentic task-entry TubeScene, copied before the P8 edit. Do not recreate
# this file from a resumed production tree to make the paired demo pass.
file(SHA256 "${BASELINE}/src/app/TubeScene.cpp" actual)
set(expected "954eb8113f0349b7a22bb3ffe104ec64f4ba93597bc13f41919538656de21ee3")
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "actor-occlusion-001 authentic baseline identity changed")
endif()
message(STATUS "actor-occlusion-001 authentic task-entry TubeScene identity verified")
