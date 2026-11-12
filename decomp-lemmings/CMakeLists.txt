cmake_minimum_required(VERSION 3.28)
project(decomp_lemmings LANGUAGES CXX)

set(LEMMINGS_REFERENCE_EXE "" CACHE FILEPATH
    "Path to the exact user-owned Lemmings Revolution.exe for offline native checks")
set(LEMMINGS_ARCHIVE "" CACHE FILEPATH
    "Path to the user-owned lemmings.box for compiled level fixture checks")
set(LEMMINGS_PATCH_ARCHIVE "" CACHE FILEPATH
    "Optional path to the user-owned patch.box for compiled level fixture checks")

add_library(lemmings_recovered STATIC
    src/allocator.cpp
    src/gadget_bounds.cpp
    src/actor_list.cpp
    src/gadget_subboxes.cpp
    src/native_int_array.cpp
    src/collision_grid.cpp
    src/cylinder_coordinate.cpp
    src/trigger_region.cpp
    src/terrain_mask.cpp
    src/level_stream.cpp
    src/compiled_level_header.cpp
    src/level_name_tables.cpp
    src/box_archive.cpp
    src/world_geometry_prefix.cpp
    src/object_base_prefix.cpp
    src/level_entity_prefix.cpp
    src/level_model.cpp
    src/level_gadget_records.cpp
    src/compiled_level.cpp
    src/model_update.cpp
    src/actor_geometry.cpp
    src/emitter_update.cpp
    src/actor_update.cpp
    src/world_coordinates.cpp
    src/animation_playback.cpp
    src/actor_falling.cpp
    src/actor_landing.cpp
    src/world_state.cpp
    src/actor_walking.cpp
    src/actor_walking_collision.cpp
    src/sprite_resource.cpp
    src/actor_animation.cpp
    src/actor_motion.cpp
    src/actor_gadget_contact.cpp
    src/actor_climbing.cpp
    src/actor_terminal.cpp
    src/actor_bouncing.cpp
    src/actor_topping.cpp
    src/actor_finalization.cpp
    src/actor_terrain_probe.cpp
    src/actor_drowning.cpp
    src/actor_region_terminal.cpp
    src/actor_construction.cpp
    src/actor_shadow.cpp
    src/actor_forced.cpp
    src/actor_base.cpp
    src/game_random.cpp
    src/world_entity_pass.cpp
    src/actor_shadow_segments.cpp
    src/actor_registry.cpp
    src/actor_destruction.cpp
    src/actor_shadow_construction.cpp
    src/actor_explosion.cpp
    src/model_construction.cpp
    src/actor_flapper.cpp
    src/world_update.cpp
    src/model_destruction.cpp
    src/terrain_blast.cpp
    src/actor_resource_selection.cpp
    src/resource_cache.cpp
    src/resource_cache_find.cpp
    src/resource_registration.cpp
    src/resource_cache_cleanup.cpp
    src/resource_cache_load.cpp
    src/resource_manager_construction.cpp
    src/sprite_factory.cpp
    src/sprite_piece_runtime.cpp
    src/sprite_resource_destruction.cpp
    src/file_stream_runtime.cpp
    src/file_stream_open.cpp
    src/sprite_runtime_read.cpp
    src/texture_resource.cpp
    src/texture_creation.cpp
    src/texture_pixel_format.cpp
    src/sprite_render_geometry.cpp
    src/sprite_render_shapes.cpp
    src/sprite_render_submission.cpp
    src/sprite_render_clip.cpp
    src/sprite_render_draw.cpp
    src/resource_table_load.cpp
    src/actor_spawn_monitor.cpp
)
target_include_directories(lemmings_recovered PUBLIC include)
target_compile_features(lemmings_recovered PUBLIC cxx_std_23)

add_executable(lemmings_inspect tools/lemmings_inspect.cpp)
target_link_libraries(lemmings_inspect PRIVATE lemmings_recovered)
add_executable(lemmings_motion tools/lemmings_motion.cpp)
target_link_libraries(lemmings_motion PRIVATE lemmings_recovered)
add_executable(lemmings_resources tools/lemmings_resources.cpp)
target_link_libraries(lemmings_resources PRIVATE lemmings_recovered)
add_executable(lemmings_textures tools/lemmings_textures.cpp)
target_link_libraries(lemmings_textures PRIVATE lemmings_recovered)

include(CTest)
if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME resource_lifecycle_boundaries
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/check_resource_lifecycle_boundaries.py"
                --program "$<TARGET_FILE:lemmings_resources>")
    endif()
    foreach(_name sprite_resource actor_animation actor_helpers actor_gadget_contact actor_climbing actor_terminal actor_bouncing actor_topping actor_finalization actor_terrain_probe actor_drowning actor_region_terminal actor_construction actor_shadow actor_forced actor_base game_random world_entity_pass actor_shadow_segments actor_registry actor_destruction actor_shadow_construction actor_explosion model_construction actor_flapper world_update model_destruction terrain_blast actor_resource_selection resource_cache resource_cache_find resource_registration resource_cache_cleanup resource_cache_load resource_manager_construction sprite_factory sprite_piece_runtime sprite_resource_destruction file_stream_runtime file_stream_runtime_open sprite_runtime_read texture_resource sprite_render_geometry sprite_render_shapes sprite_render_submission sprite_render_clip sprite_render_draw sprite_render_pipeline resource_table_load actor_spawn_monitor texture_creation texture_pixel_format file_stream_open)
        add_executable(test_${_name} tests/test_${_name}.cpp)
        target_link_libraries(test_${_name} PRIVATE lemmings_recovered)
        add_test(NAME ${_name} COMMAND test_${_name})
    endforeach()
    add_executable(sprite_resource_probe tools/sprite_resource_probe.cpp)
    target_link_libraries(sprite_resource_probe PRIVATE lemmings_recovered)
    add_executable(test_actor_walking tests/test_actor_walking.cpp)
    target_link_libraries(test_actor_walking PRIVATE lemmings_recovered)
    add_test(NAME actor_walking COMMAND test_actor_walking)
    add_executable(test_actor_walking_collision tests/test_actor_walking_collision.cpp)
    target_link_libraries(test_actor_walking_collision PRIVATE lemmings_recovered)
    add_test(NAME actor_walking_collision COMMAND test_actor_walking_collision)
    add_executable(test_animation_playback tests/test_animation_playback.cpp)
    target_link_libraries(test_animation_playback PRIVATE lemmings_recovered)
    add_test(NAME animation_playback COMMAND test_animation_playback)
    add_executable(test_actor_falling tests/test_actor_falling.cpp)
    target_link_libraries(test_actor_falling PRIVATE lemmings_recovered)
    add_test(NAME actor_falling COMMAND test_actor_falling)
    add_executable(test_actor_landing tests/test_actor_landing.cpp)
    target_link_libraries(test_actor_landing PRIVATE lemmings_recovered)
    add_test(NAME actor_landing COMMAND test_actor_landing)
    add_executable(test_world_coordinates tests/test_world_coordinates.cpp)
    target_link_libraries(test_world_coordinates PRIVATE lemmings_recovered)
    add_test(NAME world_coordinates COMMAND test_world_coordinates)
    add_executable(test_actor_update tests/test_actor_update.cpp)
    target_link_libraries(test_actor_update PRIVATE lemmings_recovered)
    add_test(NAME actor_update COMMAND test_actor_update)
    add_executable(test_actor_geometry tests/test_actor_geometry.cpp)
    target_link_libraries(test_actor_geometry PRIVATE lemmings_recovered)
    add_test(NAME actor_geometry COMMAND test_actor_geometry)
    add_executable(test_emitter_update tests/test_emitter_update.cpp)
    target_link_libraries(test_emitter_update PRIVATE lemmings_recovered)
    add_test(NAME emitter_update COMMAND test_emitter_update)
    add_executable(test_model_update tests/test_model_update.cpp)
    target_link_libraries(test_model_update PRIVATE lemmings_recovered)
    add_test(NAME model_update COMMAND test_model_update)
    add_executable(test_compiled_level tests/test_compiled_level.cpp)
    target_link_libraries(test_compiled_level PRIVATE lemmings_recovered)
    add_test(NAME compiled_level COMMAND test_compiled_level)
    add_executable(test_level_gadget_records tests/test_level_gadget_records.cpp)
    target_link_libraries(test_level_gadget_records PRIVATE lemmings_recovered)
    add_test(NAME level_gadget_records COMMAND test_level_gadget_records)
    add_executable(test_gadget_bounds tests/test_gadget_bounds.cpp)
    target_link_libraries(test_gadget_bounds PRIVATE lemmings_recovered)
    add_test(NAME gadget_bounds COMMAND test_gadget_bounds)

    add_executable(test_actor_list tests/test_actor_list.cpp)
    target_link_libraries(test_actor_list PRIVATE lemmings_recovered)
    add_test(NAME actor_list COMMAND test_actor_list)

    add_executable(test_gadget_subboxes tests/test_gadget_subboxes.cpp)
    target_link_libraries(test_gadget_subboxes PRIVATE lemmings_recovered)
    add_test(NAME gadget_subboxes COMMAND test_gadget_subboxes)

    add_executable(test_native_int_array tests/test_native_int_array.cpp)
    target_link_libraries(test_native_int_array PRIVATE lemmings_recovered)
    add_test(NAME native_int_array COMMAND test_native_int_array)

    add_executable(test_collision_grid tests/test_collision_grid.cpp)
    target_link_libraries(test_collision_grid PRIVATE lemmings_recovered)
    add_test(NAME collision_grid COMMAND test_collision_grid)

    add_executable(test_cylinder_coordinate tests/test_cylinder_coordinate.cpp)
    target_link_libraries(test_cylinder_coordinate PRIVATE lemmings_recovered)
    add_test(NAME cylinder_coordinate COMMAND test_cylinder_coordinate)

    add_executable(test_trigger_region tests/test_trigger_region.cpp)
    target_link_libraries(test_trigger_region PRIVATE lemmings_recovered)
    add_test(NAME trigger_region COMMAND test_trigger_region)

    add_executable(test_terrain_mask tests/test_terrain_mask.cpp)
    target_link_libraries(test_terrain_mask PRIVATE lemmings_recovered)
    add_test(NAME terrain_mask COMMAND test_terrain_mask)

    add_executable(test_level_stream tests/test_level_stream.cpp)
    target_link_libraries(test_level_stream PRIVATE lemmings_recovered)
    add_test(NAME level_stream COMMAND test_level_stream)

    add_executable(test_compiled_level_header tests/test_compiled_level_header.cpp)
    target_link_libraries(test_compiled_level_header PRIVATE lemmings_recovered)
    add_test(NAME compiled_level_header COMMAND test_compiled_level_header)

    add_executable(level_header_probe tools/level_header_probe.cpp)
    target_link_libraries(level_header_probe PRIVATE lemmings_recovered)
    add_executable(level_prefix_probe tools/level_prefix_probe.cpp)
    target_link_libraries(level_prefix_probe PRIVATE lemmings_recovered)
    add_executable(box_index_probe tools/box_index_probe.cpp)
    target_link_libraries(box_index_probe PRIVATE lemmings_recovered)
    add_executable(box_lookup_probe tools/box_lookup_probe.cpp)
    target_link_libraries(box_lookup_probe PRIVATE lemmings_recovered)
    if(LEMMINGS_ARCHIVE)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        set(_level_archives "${LEMMINGS_ARCHIVE}")
        if(LEMMINGS_PATCH_ARCHIVE)
            list(APPEND _level_archives "${LEMMINGS_PATCH_ARCHIVE}")
        endif()
        add_test(NAME installed_compiled_levels
            COMMAND test_compiled_level ${_level_archives})
        add_test(NAME installed_sprite_resources
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/check_installed_sprite_resources.py"
                --probe "$<TARGET_FILE:sprite_resource_probe>" ${_level_archives})
        set(_resource_archives --archive "${LEMMINGS_ARCHIVE}")
        if(LEMMINGS_PATCH_ARCHIVE)
            list(APPEND _resource_archives --patch "${LEMMINGS_PATCH_ARCHIVE}")
        endif()
        add_test(NAME installed_resource_lifecycle
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/check_installed_resource_lifecycle.py"
                --program "$<TARGET_FILE:lemmings_resources>" ${_resource_archives})
        add_test(NAME installed_texture_payloads
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/check_installed_texture_payloads.py"
                --program "$<TARGET_FILE:lemmings_textures>" ${_resource_archives})
        add_test(NAME installed_level_headers
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/check_installed_level_headers.py"
                --probe "$<TARGET_FILE:level_header_probe>"
                --archive ${_level_archives})
        add_test(NAME installed_level_prefixes
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/check_installed_level_prefixes.py"
                --probe "$<TARGET_FILE:level_prefix_probe>"
                --archive ${_level_archives})
        add_test(NAME installed_box_index
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/check_installed_box_index.py"
                --probe "$<TARGET_FILE:box_index_probe>"
                --archive ${_level_archives})
        if(LEMMINGS_PATCH_ARCHIVE)
            add_test(NAME installed_box_lookup
                COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_SOURCE_DIR}/tools/check_installed_box_lookup.py"
                    --probe "$<TARGET_FILE:box_lookup_probe>"
                    --patch "${LEMMINGS_PATCH_ARCHIVE}"
                    --base "${LEMMINGS_ARCHIVE}")
        endif()
    endif()
endif()

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    foreach(_name actor_surface actor_animation actor_helpers actor_gadget_contact sprite_piece actor_climbing actor_terminal actor_bouncing actor_topping actor_finalization actor_terrain_probe actor_drowning actor_region_terminal actor_construction actor_shadow actor_forced actor_base game_random world_entity_pass actor_shadow_segments actor_registry actor_destruction actor_shadow_construction actor_explosion model_construction actor_flapper world_update model_destruction terrain_blast actor_resource_selection resource_cache resource_cache_find resource_registration resource_cache_cleanup resource_cache_load resource_manager_construction sprite_factory sprite_piece_runtime sprite_resource_destruction file_stream_runtime file_stream_runtime_open sprite_runtime_read texture_resource sprite_render_geometry sprite_render_shapes sprite_render_submission sprite_render_clip sprite_render_draw sprite_render_pipeline resource_table_load actor_spawn_monitor texture_creation texture_pixel_format file_stream_open)
        add_executable(native_${_name}_oracle tools/native_${_name}_oracle.cpp)
        target_link_libraries(native_${_name}_oracle PRIVATE lemmings_recovered)
        target_link_options(native_${_name}_oracle PRIVATE -Wl,--image-base,0x10000000)
    endforeach()
    add_executable(native_actor_walking_oracle tools/native_actor_walking_oracle.cpp)
    target_link_libraries(native_actor_walking_oracle PRIVATE lemmings_recovered)
    target_link_options(native_actor_walking_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_actor_walking_collision_oracle tools/native_actor_walking_collision_oracle.cpp)
    target_link_libraries(native_actor_walking_collision_oracle PRIVATE lemmings_recovered)
    target_link_options(native_actor_walking_collision_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_animation_oracle tools/native_animation_oracle.cpp)
    target_link_libraries(native_animation_oracle PRIVATE lemmings_recovered)
    target_link_options(native_animation_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_actor_falling_oracle tools/native_actor_falling_oracle.cpp)
    target_link_libraries(native_actor_falling_oracle PRIVATE lemmings_recovered)
    target_link_options(native_actor_falling_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_actor_landing_oracle tools/native_actor_landing_oracle.cpp)
    target_link_libraries(native_actor_landing_oracle PRIVATE lemmings_recovered)
    target_link_options(native_actor_landing_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_world_coordinates_oracle tools/native_world_coordinates_oracle.cpp)
    target_link_libraries(native_world_coordinates_oracle PRIVATE lemmings_recovered)
    target_link_options(native_world_coordinates_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_actor_update_oracle tools/native_actor_update_oracle.cpp)
    target_link_libraries(native_actor_update_oracle PRIVATE lemmings_recovered)
    target_link_options(native_actor_update_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_emitter_oracle tools/native_emitter_oracle.cpp)
    target_link_libraries(native_emitter_oracle PRIVATE lemmings_recovered)
    target_link_options(native_emitter_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_actor_geometry_oracle tools/native_actor_geometry_oracle.cpp)
    target_link_libraries(native_actor_geometry_oracle PRIVATE lemmings_recovered)
    target_link_options(native_actor_geometry_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_model_update_oracle tools/native_model_update_oracle.cpp)
    target_link_libraries(native_model_update_oracle PRIVATE lemmings_recovered)
    target_link_options(native_model_update_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_entity_base_oracle tools/native_entity_base_oracle.cpp)
    target_link_libraries(native_entity_base_oracle PRIVATE lemmings_recovered)
    target_link_options(native_entity_base_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_point_oracle tools/native_point_oracle.cpp)
    target_link_libraries(native_point_oracle PRIVATE lemmings_recovered)
    add_executable(native_subbox_oracle tools/native_subbox_oracle.cpp)
    target_link_libraries(native_subbox_oracle PRIVATE lemmings_recovered)
    target_link_options(native_subbox_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_grid_oracle tools/native_grid_oracle.cpp)
    target_link_libraries(native_grid_oracle PRIVATE lemmings_recovered)
    target_link_options(native_grid_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_y_oracle tools/native_y_oracle.cpp)
    target_link_libraries(native_y_oracle PRIVATE lemmings_recovered)
    target_link_options(native_y_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_x_oracle tools/native_x_oracle.cpp)
    target_link_libraries(native_x_oracle PRIVATE lemmings_recovered)
    target_link_options(native_x_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_bounds_oracle tools/native_bounds_oracle.cpp)
    target_link_libraries(native_bounds_oracle PRIVATE lemmings_recovered)
    target_link_options(native_bounds_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_actor_list_oracle tools/native_actor_list_oracle.cpp)
    target_link_libraries(native_actor_list_oracle PRIVATE lemmings_recovered)
    target_link_options(native_actor_list_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_collection_oracle tools/native_collection_oracle.cpp)
    target_link_libraries(native_collection_oracle PRIVATE lemmings_recovered)
    target_link_options(native_collection_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_trigger_oracle tools/native_trigger_oracle.cpp)
    target_link_libraries(native_trigger_oracle PRIVATE lemmings_recovered)
    target_link_options(native_trigger_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_terrain_oracle tools/native_terrain_oracle.cpp)
    target_link_libraries(native_terrain_oracle PRIVATE lemmings_recovered)
    target_link_options(native_terrain_oracle PRIVATE -Wl,--image-base,0x10000000)
    add_executable(native_level_stream_oracle tools/native_level_stream_oracle.cpp)
    target_link_libraries(native_level_stream_oracle PRIVATE lemmings_recovered)
    target_link_options(native_level_stream_oracle PRIVATE -Wl,--image-base,0x10000000)
    if(BUILD_TESTING AND LEMMINGS_REFERENCE_EXE)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        foreach(_name actor_surface actor_animation actor_helpers actor_gadget_contact sprite_piece actor_climbing actor_terminal actor_bouncing actor_topping actor_finalization actor_terrain_probe actor_drowning actor_region_terminal actor_construction actor_shadow actor_forced actor_base game_random world_entity_pass actor_shadow_segments actor_registry actor_destruction actor_shadow_construction actor_explosion model_construction actor_flapper world_update model_destruction terrain_blast actor_resource_selection resource_cache resource_cache_find resource_registration resource_cache_cleanup resource_cache_load resource_manager_construction sprite_factory sprite_piece_runtime sprite_resource_destruction file_stream_runtime file_stream_runtime_open sprite_runtime_read texture_resource sprite_render_geometry sprite_render_shapes sprite_render_submission sprite_render_clip sprite_render_draw sprite_render_pipeline resource_table_load actor_spawn_monitor texture_creation texture_pixel_format file_stream_open)
            add_test(NAME native_${_name}
                COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/run_native_${_name}_oracle.py"
                    --exe "${LEMMINGS_REFERENCE_EXE}" --program "$<TARGET_FILE:native_${_name}_oracle>")
        endforeach()
        add_test(NAME native_actor_update
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_actor_update_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_actor_update_oracle>")
        add_test(NAME native_actor_walking
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/run_native_actor_walking_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}" --program "$<TARGET_FILE:native_actor_walking_oracle>")
        add_test(NAME native_actor_walking_collision
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/run_native_actor_walking_collision_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}" --program "$<TARGET_FILE:native_actor_walking_collision_oracle>")
        add_test(NAME native_animation
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/run_native_animation_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}" --program "$<TARGET_FILE:native_animation_oracle>")
        add_test(NAME native_actor_falling
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/run_native_actor_falling_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}" --program "$<TARGET_FILE:native_actor_falling_oracle>")
        add_test(NAME native_actor_landing
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/run_native_actor_landing_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}" --program "$<TARGET_FILE:native_actor_landing_oracle>")
        add_test(NAME native_world_coordinates
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_world_coordinates_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_world_coordinates_oracle>"
                --archive ${_level_archives})
        add_test(NAME native_emitter
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_emitter_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_emitter_oracle>")
        add_test(NAME native_actor_geometry_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_actor_geometry_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_actor_geometry_oracle>")
        add_test(NAME native_model_update_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_model_update_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_model_update_oracle>")
        add_test(NAME native_entity_base_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_entity_base_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_entity_base_oracle>")
        add_test(NAME native_point_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_point_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_point_oracle>")
        add_test(NAME native_subbox_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_subbox_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_subbox_oracle>")
        add_test(NAME native_grid_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_grid_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_grid_oracle>")
        add_test(NAME native_y_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_y_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_y_oracle>")
        add_test(NAME native_x_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_x_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_x_oracle>")
        add_test(NAME native_bounds_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_bounds_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_bounds_oracle>")
        add_test(NAME native_actor_list_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_actor_list_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_actor_list_oracle>")
        add_test(NAME native_collection_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_collection_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_collection_oracle>")
        add_test(NAME native_trigger_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_trigger_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_trigger_oracle>")
        add_test(NAME native_terrain_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_terrain_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_terrain_oracle>")
        add_test(NAME native_level_stream_oracle
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/run_native_level_stream_oracle.py"
                --exe "${LEMMINGS_REFERENCE_EXE}"
                --program "$<TARGET_FILE:native_level_stream_oracle>")
    endif()
endif()
