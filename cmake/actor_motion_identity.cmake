# Authentic task-entry frame loop: observational baseline, not a recreated flag.
file(SHA256 "${BASELINE}/src/app/main.cpp" actual)
set(expected "fe1454539c44483b5c8b9f0fc8aa4423c6e1f47e63697a27b17af363f20bf30f")
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "P7 baseline frame loop changed: ${actual}")
endif()
message(STATUS "P7 authentic frame-loop SHA256 ${actual}")
