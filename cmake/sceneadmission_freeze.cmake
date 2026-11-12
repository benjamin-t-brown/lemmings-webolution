# scene-admission-001: authentic task-entry copies of BackdropScene.cpp/.h, kept
# under the ignored build/scene-admission-001/frozen/ and never swapped into src/.
#
# HEAD is not the accepted pre-task state in this dirty checkout, so identity is
# the SHA-256 recorded in the supervisor entry manifest
# (build/agent-runs/20261031T064853-24188-dcc18873/before-source-hashes.json).
# A copy is made at most once, and only while the working file still has the
# entry hash; after the production edit the hashes differ and nothing is copied
# or rewritten. sceneadmission_identity.cmake fails the test on any mismatch.

set(SA_ENTRY_RUN "20261031T064853-24188-dcc18873")
set(SA_HASH_BackdropScene_cpp "acd0bfc8424fa4e4a8096a6f051f44eac31825d3049a4c2ae48a3bf90194b35c")
set(SA_HASH_BackdropScene_h "6f530a25654a8ca278314475ced83fe4a7e8e326324adea4ec26d1e3dfc7c11c")

function(sceneadmission_freeze src_dir frozen_dir)
  if(NOT EXISTS "${frozen_dir}/BackdropScene.cpp" AND EXISTS "${src_dir}/app/BackdropScene.cpp")
    file(SHA256 "${src_dir}/app/BackdropScene.cpp" _h)
    if(_h STREQUAL "${SA_HASH_BackdropScene_cpp}")
      file(COPY "${src_dir}/app/BackdropScene.cpp" DESTINATION "${frozen_dir}")
    endif()
  endif()
  if(NOT EXISTS "${frozen_dir}/app/BackdropScene.h" AND EXISTS "${src_dir}/app/BackdropScene.h")
    file(SHA256 "${src_dir}/app/BackdropScene.h" _h)
    if(_h STREQUAL "${SA_HASH_BackdropScene_h}")
      file(COPY "${src_dir}/app/BackdropScene.h" DESTINATION "${frozen_dir}/app")
    endif()
  endif()
endfunction()
