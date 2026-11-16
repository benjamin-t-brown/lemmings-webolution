# Task-entry World.cpp matches the worker's immutable supervisor manifest:
# build/agent-runs/20261115T224344-39368-47694f59/before-source-hashes.json.
file(SHA256 "${TASK_DIR}/before-source/src/sim/World.cpp" actual)
set(expected "df8af703d6d383a79a6b31d661d9d324effea5034127228471db74aca3ca89b0")
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "antigrav skill task-entry World.cpp identity mismatch")
endif()
