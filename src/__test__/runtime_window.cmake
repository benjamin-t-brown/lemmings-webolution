# Own-app integration only: no original executable or physical input, and SDL's
# dummy driver cannot take focus. The two paths must compose identical frames.
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
function(run_app)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E env SDL_VIDEODRIVER=dummy
    "${APP}" --game-dir "${GAME_DIR}" ${ARGN}
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "webolution failed (${result}): ${output}\n${error}")
  endif()
endfunction()

foreach(ticks IN ITEMS 0 1)
  if(ticks EQUAL 0)
    set(frames 1)
  else()
    set(frames 30) # deterministic --frames clock: 30/144 seconds = one sim tick
  endif()
  set(window "${OUTPUT_DIR}/window-${ticks}.png")
  set(fixture "${OUTPUT_DIR}/fixture-${ticks}.png")
  # Remove only our own outputs, so a stale image cannot pass a missing-write bug.
  file(REMOVE "${window}" "${fixture}")
  run_app(--native --sim --frames ${frames} --window-shot "${window}")
  run_app(--native --sim-ticks ${ticks} --skill-panel --hud-ticks ${ticks} --cursor-fixture --shot "${fixture}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files "${window}" "${fixture}"
    RESULT_VARIABLE different)
  if(NOT different EQUAL 0)
    message(FATAL_ERROR "runtime frame differs from panel/HUD fixture at tick ${ticks}")
  endif()
endforeach()

# A non-sim window must not gain the runtime panel/HUD.
run_app(--native --frames 1 --window-shot "${OUTPUT_DIR}/no-sim-window.png")
run_app(--native --shot "${OUTPUT_DIR}/no-sim-fixture.png")
execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files
  "${OUTPUT_DIR}/no-sim-window.png" "${OUTPUT_DIR}/no-sim-fixture.png"
  RESULT_VARIABLE different)
if(NOT different EQUAL 0)
  message(FATAL_ERROR "non-sim window changed its composition")
endif()
