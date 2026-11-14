foreach(name IN ITEMS MM_W_012-world.csv MM_W_012-world-all.csv MM_W_012-app.csv MM_W_012-world-summary.txt)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files
    "${TASK_DIR}/before/${name}" "${TASK_DIR}/after/${name}" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Water-lemming production control changed: ${name}")
  endif()
endforeach()
