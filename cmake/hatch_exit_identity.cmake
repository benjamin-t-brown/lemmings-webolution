# Authentic task-entry sources used for the paired clone demonstrations.
foreach(item IN ITEMS
    "src/app/main.cpp|afe957f2f2b96c4cf8f5e935db3ab6e6922a2136c66317f3ef1a854ebe560184"
    "src/app/BackdropScene.cpp|a8eea83f241e2239cbd45418651f1519ce8d25ab8986670a4cfc4be62d7a2705")
  string(REPLACE "|" ";" fields "${item}")
  list(GET fields 0 name)
  list(GET fields 1 expected)
  file(SHA256 "${BASELINE}/${name}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Hatch/exit authentic baseline changed: ${name}: ${actual}")
  endif()
endforeach()
message(STATUS "Hatch/exit task-entry source identities match")
