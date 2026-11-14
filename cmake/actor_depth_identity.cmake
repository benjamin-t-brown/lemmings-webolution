# Authentic actor-depth-001 entry source; do not refresh from modified files.
file(SHA256 "${BASELINE}/src/app/TubeScene.cpp" actual)
if(NOT actual STREQUAL "1ebb174a55f8a18ef3157d11b2e622dda829890af135c05b4e8a09ea1a066b77")
  message(FATAL_ERROR "actor-depth-001 authentic baseline identity changed")
endif()
file(SHA256 "${BASELINE}/src/app/main.cpp" actualMain)
if(NOT actualMain STREQUAL "7b0d9095f084a70d30b1fc2ec14338788dc2885775fc568375ecf33a69ecc7bf")
  message(FATAL_ERROR "actor-depth-001 authentic application baseline identity changed")
endif()
