# Compare full frames from SDL's dummy window and the independent fixture.
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
    set(opening_ms 6) # first admitted integer delta at the owned 144 Hz clock
  else()
    set(frames 30) # deterministic --frames clock: 30/144 seconds = one sim tick
    set(opening_ms 208)
  endif()
  set(window "${OUTPUT_DIR}/window-${ticks}.png")
  set(fixture "${OUTPUT_DIR}/fixture-${ticks}.png")
  set(closed_fixture "${OUTPUT_DIR}/closed-fixture-${ticks}.png")
  # Clear the output files so every comparison requires a fresh image.
  file(REMOVE "${window}" "${fixture}" "${closed_fixture}")
  run_app(--native --sim --frames ${frames} --window-shot "${window}")
  run_app(--native --sim-ticks ${ticks} --skill-panel --hud-ticks ${ticks} --cursor-fixture --shot "${closed_fixture}")
  # Compose animated doors from the closed fixture at the window's exact clock.
  execute_process(COMMAND "${FIXTURE}" "${GAME_DIR}" ${ticks} ${opening_ms}
    "${closed_fixture}" "${fixture}" RESULT_VARIABLE fixture_result)
  if(NOT fixture_result EQUAL 0)
    message(FATAL_ERROR "animated entry fixture failed (${fixture_result})")
  endif()
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
