# Exact task-entry sources; current headers add only the new defaulted fields.
foreach(item IN ITEMS
    "World.cpp|1db22b394a4cbea19ff468b216f568a02afebae48ce3b2cb5b160ff308a4e69d"
    "RetailLevel.cpp|b8fee6c6900b59d0a800de8eb773d71fd57dbcffcd2577830a99762bf9b3f9ef"
    "LevelObjects.cpp|2e347e9fcb46dcc17b41b8b8a800c158e73392146f401ab00e76c471ce1d0ce7")
  string(REPLACE "|" ";" fields "${item}")
  list(GET fields 0 name)
  list(GET fields 1 expected)
  file(SHA256 "${BASELINE}/${name}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Hatch direction authentic baseline changed: ${name}: ${actual}")
  endif()
endforeach()
