# Frozen from the actual worker entry, including all incoming main.cpp edits.
foreach(item IN ITEMS
    "src/app/main.cpp|950b25da45ceff3bee94b669ac930e6d59e8509be964eb87b03181305594ebc6"
    "src/app/BackdropScene.cpp|88d2e555c818d75d5c7df786005cf9125dd4ceb8150f38d67a3f12e1505fa284")
  string(REPLACE "|" ";" fields "${item}")
  list(GET fields 0 name)
  list(GET fields 1 expected)
  file(SHA256 "${BASELINE}/${name}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Exit balloon authentic baseline changed: ${name}: ${actual}")
  endif()
endforeach()
