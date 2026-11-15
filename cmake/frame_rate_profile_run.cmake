file(MAKE_DIRECTORY "${OUTPUT}/private-localappdata")
file(WRITE "${OUTPUT}/frames.csv" "")
execute_process(COMMAND "${CMAKE_COMMAND}" -E env
  "SDL_VIDEODRIVER=dummy" "SDL_AUDIODRIVER=dummy"
  "LOCALAPPDATA=${OUTPUT}/private-localappdata" "XDG_DATA_HOME=${OUTPUT}/private-localappdata"
  "WEBOLUTION_FRAME_PROFILE=${OUTPUT}/frames.csv"
  "${CLONE}" --direct-level --game-dir "${GAME_DIR}" --level "${LEVEL}"
  --native --sim-ticks 50 --frames 150 --no-audio --window-shot "${OUTPUT}/frame.png"
  RESULT_VARIABLE result OUTPUT_FILE "${OUTPUT}/stdout.log" ERROR_FILE "${OUTPUT}/stderr.log")
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Production clone demonstration failed: ${result}; ${OUTPUT}")
endif()
file(SHA256 "${OUTPUT}/frame.png" actual)
if(NOT actual STREQUAL EXPECTED)
  message(FATAL_ERROR "Production framebuffer differs from authentic pre-task frame: ${actual} != ${EXPECTED}")
endif()
file(STRINGS "${OUTPUT}/frames.csv" rows)
list(LENGTH rows count)
if(NOT count EQUAL 151)
  message(FATAL_ERROR "Expected header and 150 measured production frames, got ${count}")
endif()
message(STATUS "${LEVEL}: 150 production frames; pre-task PNG SHA256 preserved (${actual})")
