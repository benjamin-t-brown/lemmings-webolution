# empty-receiver-fall-001 deliberately removes the later dry-pit crossing.
# Preserve exact supported-water motion and the pre-pit public app control.
# Full dry/filled outcomes are asserted in empty_receiver_after.
foreach(name IN ITEMS MM_W_012-supported-prefix.csv MM_W_012-app-supported-prefix.csv)
  file(SIZE "${TASK_DIR}/before/${name}" prefix_size)
  if(prefix_size EQUAL 0)
    message(FATAL_ERROR "Water control prefix is empty: ${name}")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files
    "${TASK_DIR}/before/${name}" "${TASK_DIR}/after/${name}" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Water-lemming production control changed: ${name}")
  endif()
endforeach()
