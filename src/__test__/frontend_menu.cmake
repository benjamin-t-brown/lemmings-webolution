# The app menu must preserve the retained MenuRenderDump composition, in both
# headless and real-loop/dummy-driver paths. No original or desktop is used.
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
function(run)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "frontend command failed (${result}): ${out}\n${err}")
  endif()
endfunction()
file(REMOVE "${OUTPUT_DIR}/fixture.png" "${OUTPUT_DIR}/app.png" "${OUTPUT_DIR}/window.png")
run("${DUMP}" "${GAME_DIR}" "${OUTPUT_DIR}/fixture.png")
run("${APP}" --game-dir "${GAME_DIR}" --frontend --shot "${OUTPUT_DIR}/app.png")
run("${CMAKE_COMMAND}" -E env SDL_VIDEODRIVER=dummy "${APP}" --game-dir "${GAME_DIR}"
  --frontend --frames 1 --window-shot "${OUTPUT_DIR}/window.png")
run("${CMAKE_COMMAND}" -E compare_files "${OUTPUT_DIR}/fixture.png" "${OUTPUT_DIR}/app.png")
run("${CMAKE_COMMAND}" -E compare_files "${OUTPUT_DIR}/fixture.png" "${OUTPUT_DIR}/window.png")
