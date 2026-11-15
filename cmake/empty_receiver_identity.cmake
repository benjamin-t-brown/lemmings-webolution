foreach(pair IN ITEMS
    "World.cpp|edf7520a6513b7da465e49f58c342d3c4517c52dfb69a4d59b12b7ac1554f3d1"
    "World.h|6b5850232bdd783d71f3c6a225aaa08963678039c9841d1206c8cf8c08ecaab7"
    "RetailLevel.cpp|67042e6b154c84103fdb2b1cf1cf0d9ee24479be450f59a443f2835fdbeeef20")
  string(REPLACE "|" ";" fields "${pair}")
  list(GET fields 0 name)
  list(GET fields 1 expected)
  file(SHA256 "${TASK_DIR}/before-source/src/sim/${name}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Authentic task-entry receiver source changed: ${name}")
  endif()
endforeach()
if(EXISTS "${TASK_DIR}/before-source/gadget-replay.cpp")
  file(SHA256 "${TASK_DIR}/before-source/gadget-replay.cpp" replay_hash)
  if(NOT replay_hash STREQUAL "278a5ad5897c6cc53114869661a28fd42bcb9c282359bdebe2589b8e27c51f72")
    message(FATAL_ERROR "Authentic historical gadget replay changed")
  endif()
endif()
