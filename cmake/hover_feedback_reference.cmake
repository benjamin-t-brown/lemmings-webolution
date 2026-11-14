# Read-only registered-reference byte anchors. Never executes the reference.
set(reference "${GAME_DIR}/Lemmings Revolution.exe")
file(SHA256 "${reference}" identity)
if(NOT identity STREQUAL "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f")
  message(FATAL_ERROR "Hover feedback reference identity mismatch")
endif()
function(anchor offset length expected)
  math(EXPR offset "${offset}")
  file(READ "${reference}" actual OFFSET ${offset} LIMIT ${length} HEX)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Hover feedback byte anchor differs at file offset ${offset}")
  endif()
endfunction()
# VA 454AE7: push selector 2 before the overlay mode setter.
anchor(0x54ae7 2 "6a02")
# VA 454B95: mov ecx,[eax+A0] is the actual integer text source.
anchor(0x54b95 6 "8b88a0000000")
# VA 454C37: mov eax,44 before shifting by screen mode.
anchor(0x54c37 5 "b82c000000")
# VA 4C041C: "%d\0".
anchor(0xc041c 3 "256400")
message(STATUS "Hover reference SHA-256 and four instruction/string anchors match")
