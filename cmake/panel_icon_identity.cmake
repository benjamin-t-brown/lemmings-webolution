# Hashes taken from the worker-entry copies before any production edit.
set(names main.cpp RuntimeSkillPanel.cpp RuntimeSkillPanel.h SkillPanel.cpp SkillPanel.h)
set(hashes
  da8a50fa1a7b0f6796d5910d351acb86a12147bd5bfa27100b1da6b16d9f334b
  53ae887b2f0f1029a6613d4d131fcb88db4945d0a7cc782c094d8a261fb8835d
  90e5a4251f8515cd110ea0591d0eadc345552e7ba03dde9e893b6f76ab448d51
  caf7aa62b63660d13f726370b1e421617a273019fb6bad9de15825479f32e9b4
  8b61e698dc1d31bc04c9d37ec247ec1be55472f6837d7514c605c7717915c8b5)
foreach(i RANGE 0 4)
  list(GET names ${i} name)
  list(GET hashes ${i} expected)
  file(SHA256 "${TASK_DIR}/frozen/src/app/${name}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "panel-icon frozen task-entry source changed: ${name}")
  endif()
endforeach()
