# Authentic task-entry application, preserved before hatch-timing-001 edits.
# Shared World/RuntimeSim retain their old public entry and default behavior.
file(SHA256 "${BASELINE}/src/app/main.cpp" identity)
if(NOT identity STREQUAL "c1bdf27d0fe9374492006e987a6dd6f0d16887cddb26fcfbbf99c3433ee0b032")
  message(FATAL_ERROR "hatch-timing-001 authentic application baseline changed")
endif()
message(STATUS "Hatch timing task-entry application identity matches")
