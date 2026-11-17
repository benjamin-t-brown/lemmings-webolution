if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2)
  set(co_root "${CMAKE_SOURCE_DIR}/build/climber-occlusion-001")
  set(co_scene src/app/TubeScene.cpp)
  set(co_compare "-")
  # basher-occlusion-001: retire legacy climber_occlusion_after's facing-dependent allowance.
  add_executable(test_climber_occlusion_app_after src/__test__/test_climber_occlusion_app.cpp ${co_scene}
    src/app/main.cpp)
  target_link_libraries(test_climber_occlusion_app_after PRIVATE actor_occlusion_support webolution_objects)
  target_compile_definitions(test_climber_occlusion_app_after PRIVATE WEBOLUTION_APP_NO_MAIN=1 WEBOLUTION_HAVE_SDL2=1)
  # release-rate-control-001: legacy app-after full frames omit RATE.
endif()
