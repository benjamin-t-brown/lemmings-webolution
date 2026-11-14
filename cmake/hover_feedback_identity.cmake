# Authentic pre-hover-feedback application; includes accepted prior working edits.
file(SHA256 "${BASELINE}/src/app/main.cpp" identity)
if(NOT identity STREQUAL "de30cd251215178eaf62ee591e36cc0fa16e29468306a5b45926d7ca3d5a6ff7")
  message(FATAL_ERROR "hover-feedback-001 authentic application baseline changed")
endif()
message(STATUS "Hover feedback task-entry application identity matches")
