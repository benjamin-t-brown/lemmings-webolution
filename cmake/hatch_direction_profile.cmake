# Both source-built applications use the ordinary performance demonstration's
# CLI inputs. Preserve the authentic all-right golden as well as the corrected
# alternating-release golden; do not overwrite historical sources or images.
file(MAKE_DIRECTORY "${OUTPUT}")
execute_process(COMMAND "${DRIVER}" profile "${OUTPUT}"
  RESULT_VARIABLE result OUTPUT_FILE "${OUTPUT}/stdout.log"
  ERROR_FILE "${OUTPUT}/stderr.log")
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Hatch direction profile-frame driver failed: ${result}; ${OUTPUT}")
endif()
file(SHA256 "${OUTPUT}/frame.png" actual)
if(NOT actual STREQUAL EXPECTED)
  message(FATAL_ERROR "Hatch direction profile-frame mismatch: ${actual} != ${EXPECTED}")
endif()
message(STATUS "${SIDE} release: MM_W_012 profile frame SHA256 ${actual}")
