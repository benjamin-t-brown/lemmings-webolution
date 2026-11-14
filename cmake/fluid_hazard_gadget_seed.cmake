file(SHA256 "${TASK_DIR}/gadget-seed/SavedGame.dat" actual)
if(NOT actual STREQUAL "17126a34c498eaf70b61fb55748f650560b2d20063928ed1e66aba131c775261")
  message(FATAL_ERROR "Earned private gadget-control seed changed")
endif()
set(root "${TASK_DIR}/gadget-before")
file(MAKE_DIRECTORY "${root}/runs/seed/save/LemmingsWebolution")
configure_file("${TASK_DIR}/gadget-seed/SavedGame.dat"
  "${root}/runs/seed/save/LemmingsWebolution/SavedGame.dat" COPYONLY)
file(WRITE "${root}/current-run.txt" "seed\n")
