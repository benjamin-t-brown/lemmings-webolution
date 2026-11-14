# Static read only. Never execute the reference. Address 0x4B1498 is
# file offset 0xb1498 in this exact registered PE identity.
# CTest runs CMake with the asset directory, as the other asset measurements
# do. The reference is input to file(SHA256/READ), never a command argument or
# executable. Keep the verifier's original-code inventory guard unchanged.
set(REFERENCE "${GAME_DIR}/Lemmings Revolution.exe")
file(SHA256 "${REFERENCE}" identity)
if(NOT identity STREQUAL "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f")
  message(FATAL_ERROR "actor depth reference identity differs")
endif()
file(READ "${REFERENCE}" bias OFFSET 726168 LIMIT 8 HEX)
if(NOT bias STREQUAL "613255302aa9333f")
  message(FATAL_ERROR "actor normalized 0.0003 bias bytes differ")
endif()
message(STATUS "Registered reference 0x4B1498 bias bytes verified statically")
