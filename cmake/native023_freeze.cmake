# native-023: authentic task-entry copies of the three production files, kept
# under the ignored build/native-023/frozen/ and never swapped into src/.
#
# HEAD is not the accepted pre-task state in this dirty checkout, so the copies
# are identified by SHA-256 against the supervisor's entry manifest
# (build/agent-runs/20261031T050823-22828-d5a958b6/before-source-hashes.json).
# The copies were recovered from Git HEAD plus the retained before-diff hunks and
# SHA-256 matched to that manifest (build/native-023/baseline-recovery.json).
# Nothing here regenerates or rewrites them: native023_freeze only reports state,
# and native023_identity fails when any hash differs.

set(N023_ENTRY_RUN "20261031T050823-22828-d5a958b6")
set(N023_HASH_assets_LevelObjects_cpp "7e1b7d622044feb909778ccbc8977e3af73e563476da292d22eb6a22738638fc")
set(N023_HASH_assets_LevelObjects_h "98f45bb22d25af386b8f355d23828835ad6f02055c96e0e6f31ac1ba552fe47c")
set(N023_HASH_sim_RetailLevel_cpp "1a589a3eb230d0159f0095fd65c13abdd9fff574387eca11010070a5113d83cc")

function(native023_frozen_matches dir result)
  set(_ok TRUE)
  foreach(rel IN ITEMS assets/LevelObjects.cpp assets/LevelObjects.h sim/RetailLevel.cpp)
    string(MAKE_C_IDENTIFIER "${rel}" _id)
    if(NOT EXISTS "${dir}/${rel}")
      set(_ok FALSE)
    else()
      file(SHA256 "${dir}/${rel}" _h)
      if(NOT _h STREQUAL "${N023_HASH_${_id}}")
        set(_ok FALSE)
      endif()
    endif()
  endforeach()
  set(${result} ${_ok} PARENT_SCOPE)
endfunction()

function(native023_freeze dir)
  native023_frozen_matches("${dir}" _ok)
  if(_ok)
    set(NATIVE023_FROZEN_STATE "present (entry hashes match)" PARENT_SCOPE)
  else()
    set(NATIVE023_FROZEN_STATE "missing or entry hashes DIFFER (native023_identity_report fails)" PARENT_SCOPE)
  endif()
endfunction()
