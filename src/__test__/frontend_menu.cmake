# The app menu must use the same cursor-composed frame in headless and actual
# window-loop/dummy-driver paths. frontendsession separately checks the unchanged
# base against MenuRender; retain that base dump here for inspection as well.
# No original game or desktop is used.
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
function(run)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "frontend command failed (${result}): ${out}\n${err}")
  endif()
endfunction()
file(REMOVE "${OUTPUT_DIR}/fixture.png" "${OUTPUT_DIR}/base.png" "${OUTPUT_DIR}/app.png" "${OUTPUT_DIR}/window.png"
  "${OUTPUT_DIR}/map-normal-fixture.png" "${OUTPUT_DIR}/map-held-fixture.png")
run("${MENU_DUMP}" "${GAME_DIR}" "${OUTPUT_DIR}/base.png")
run("${DUMP}" "${GAME_DIR}" menu "${OUTPUT_DIR}/fixture.png")
run("${DUMP}" "${GAME_DIR}" map "${OUTPUT_DIR}/map-normal-fixture.png")
run("${DUMP}" "${GAME_DIR}" map-held-fixture "${OUTPUT_DIR}/map-held-fixture.png")
execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files
  "${OUTPUT_DIR}/map-normal-fixture.png" "${OUTPUT_DIR}/map-held-fixture.png"
  RESULT_VARIABLE cursor_same)
if(NOT EXISTS "${OUTPUT_DIR}/map-normal-fixture.png" OR NOT EXISTS "${OUTPUT_DIR}/map-held-fixture.png")
  message(FATAL_ERROR "cursor fixture PNG missing")
endif()
if(NOT cursor_same EQUAL 1)
  message(FATAL_ERROR "held-cursor fixture did not change normal-cursor map")
endif()
run("${APP}" --game-dir "${GAME_DIR}" --frontend --shot "${OUTPUT_DIR}/app.png")
run("${CMAKE_COMMAND}" -E env SDL_VIDEODRIVER=dummy "${APP}" --game-dir "${GAME_DIR}"
  --frontend --frames 1 --window-shot "${OUTPUT_DIR}/window.png")
run("${CMAKE_COMMAND}" -E compare_files "${OUTPUT_DIR}/fixture.png" "${OUTPUT_DIR}/app.png")
run("${CMAKE_COMMAND}" -E compare_files "${OUTPUT_DIR}/fixture.png" "${OUTPUT_DIR}/window.png")
