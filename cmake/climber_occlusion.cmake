if(BUILD_TESTING AND WEBOLUTION_HAVE_SDL2)
  set(co_root "${CMAKE_SOURCE_DIR}/build/climber-occlusion-001")
  set(co_scene src/app/TubeScene.cpp)
  set(co_compare "-")
  add_executable(test_climber_occlusion_after src/__test__/test_climber_occlusion.cpp ${co_scene})
  target_link_libraries(test_climber_occlusion_after PRIVATE actor_occlusion_support webolution_objects)
  add_test(NAME climber_occlusion_after COMMAND test_climber_occlusion_after after
    "${co_root}/after" "${co_compare}")
  set_tests_properties(climber_occlusion_after PROPERTIES TIMEOUT 180
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;WEBOLUTION_GAME_DIR=${WEBOLUTION_GAME_DIR}")
  add_executable(test_climber_occlusion_app_after src/__test__/test_climber_occlusion_app.cpp ${co_scene}
    src/app/main.cpp)
  target_link_libraries(test_climber_occlusion_app_after PRIVATE actor_occlusion_support webolution_objects)
  target_compile_definitions(test_climber_occlusion_app_after PRIVATE WEBOLUTION_APP_NO_MAIN=1 WEBOLUTION_HAVE_SDL2=1)
  # release-rate-control-001: legacy app-after full frames omit RATE.
endif()
