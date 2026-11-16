# Read-only reference measurements, cross-checked with retained D2 instruction
# reads. VA = file offset + 0x400000 for these mapped .text instruction spans.
set(reference "${GAME_DIR}/Lemmings Revolution.exe")
file(SHA256 "${reference}" actual)
if(NOT actual STREQUAL "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f")
  message(FATAL_ERROR "Nearest-pick reference identity mismatch")
endif()
function(anchor offset length expected)
  math(EXPR offset "${offset}")
  file(READ "${reference}" actual OFFSET ${offset} LIMIT ${length} HEX)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Nearest-pick retained instruction anchor mismatch at ${offset}")
  endif()
endfunction()
# 455194..4551AA: stock/skill validity gate and skip-on-zero.
anchor(0x55194 23 "b9d8914c00e8424f000025ff00000085c00f848b000000")
# 4551EE..455203: state priority descending, then strict squared distance;
# equality keeps the earlier actor. These are data reads, never executable code.
anchor(0x551ee 21 "8b4da03b4dfc7f0ddd4594dc5da8dfe0f6c4017433")
message(STATUS "Pinned original uses skill gate, state priority then terrain-target distance")
