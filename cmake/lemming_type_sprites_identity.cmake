# Task-entry f62b3b8 source, frozen before attempt 1; never refresh from resumed edits.
set(names AppEntry.h LemmingAnim.cpp LemmingAnim.h main.cpp TubeScene.cpp TubeScene.h)
set(hashes
  a35c922f8c4206544624cdcbf348307210a178d9105c549c3aed932778fc9ebc
  541057583795e049248c94272558e361e6b84daa889fbbd1da542fc49523f3e8
  bf9e418fc8cf3c7b661d20d6bbe3f1a947bc8384bab95ab16c1510f71db2d7e5
  5374e3d7553df022593b8c0ee88015e866694b8f42c8c259dc3c7c84ae3def6a
  22d95ac9b4616ba0733ff0a0f531471c60e7495b28da380cf9120c9f0b851d03
  c245702354908e5ab3afbeb73a166c37e094e1144675ab82acd0863b09fa79ba)
foreach(n RANGE 0 5)
  list(GET names ${n} name)
  list(GET hashes ${n} expected)
  file(SHA256 "${BASELINE}/src/app/${name}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "lemming-type-sprites-001 frozen ${name} identity changed")
  endif()
endforeach()
