# Production sources, grouped by domain. Add every new .cpp here.
#
# Convention (shared with Carcer):
#   Name.h   public interface for a sibling Name.cpp
#   Name.hpp header-only, no sibling implementation
# A production .cpp requires a same-directory .h, except src/main.cpp and
# standalone programs under src/tools/ and src/__test__/.

set(WEBOLUTION_SOURCES
  # assets -- reading the original game's data files
  src/assets/Box.cpp
  src/assets/Lrz.cpp
  src/assets/Bitmap.cpp
  src/assets/Png.cpp
  src/assets/GameData.cpp
  src/assets/Sprite.cpp
  src/assets/LevelIndex.cpp
  src/assets/Cylinder.cpp
  src/assets/CollisionGrid.cpp
  src/assets/LightMap.cpp
  src/assets/CellMesh.cpp
  src/assets/SurfaceMesh.cpp
  src/assets/LevelObjects.cpp
  src/assets/LevelStats.cpp
  src/assets/PolyList.cpp
  src/assets/SceneNode.cpp
  src/app/SceneMeshSource.cpp
  src/app/ActorVisibility.cpp
  src/app/LemmingMotion.cpp
  src/app/HatchAnimation.cpp
  src/assets/Wav.cpp

  # gfx -- renderer-independent sprite orientation selection
  src/gfx/SpriteView.cpp

  # sim -- the game itself, independent of rendering
  src/sim/Terrain.cpp
  src/sim/World.cpp
  src/sim/RetailLevel.cpp

  # shell -- progress / unlock (no SDL)
  src/shell/Progress.cpp

  # save -- original-layout SavedGame.dat codec (pure) and storage adapter
  src/save/SavedGame.cpp
  src/save/SaveStore.cpp

)

list(TRANSFORM WEBOLUTION_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Reuse only the recovered bitmap-envelope decoder at the asset boundary.
# The recovery subproject and its optional original-byte oracles stay separate.
set(WEBOLUTION_TEXTURE_DECODER_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/texture_resource.cpp
)

# Serialized SPR reading only; original runtime allocation and rendering stay
# outside this compatibility boundary. level_stream.cpp (the shared bounded
# string primitive) is compiled here and nowhere else, so the compiled-level
# target below links this one rather than listing it again.
set(WEBOLUTION_SPRITE_RESOURCE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/sprite_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/level_stream.cpp
)

# Serialized compiled-level (.LVL) reading only, for the hatch entity list
# production level loading needs (src/assets/LevelObjects.cpp ->
# lemmings::read_compiled_level). Exactly the translation units
# read_compiled_level needs and no more: box_archive.cpp is here only because
# load_compiled_level sits in the same translation unit as read_compiled_level
# and references it -- production archive reads stay on src/assets/Box.cpp.
# The recovery subproject, its oracle targets and original runtime allocation,
# object registration, transforms and gameplay classes stay outside this
# compatibility boundary.
set(WEBOLUTION_COMPILED_LEVEL_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/compiled_level.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/compiled_level_header.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/level_name_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/world_geometry_prefix.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/world_coordinates.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/object_base_prefix.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/level_entity_prefix.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/level_gadget_records.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/level_model.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings/src/box_archive.cpp
)

# audio -- SDL2 mixer and the sim-cue adapter. Added to webolution_objects (and
# SDL2 linked to it) by CMakeLists.txt whenever SDL2 is available, so every
# executable that links the shared objects, including src/app/main.cpp, gets one
# production copy. The cue table in CuePlayback.cpp routes only cues with
# retained static play-site evidence.
set(WEBOLUTION_AUDIO_SOURCES
  src/audio/Mixer.cpp
  src/audio/CuePlayback.cpp
)
list(TRANSFORM WEBOLUTION_AUDIO_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Presentation sources for the runnable target. The flat-first renderer and
# placeholder shell moved to old/src/gfx in the 2026-10-18 reset; the 3D-first
# replacement lands here. Audio is wired through src/app/main.cpp via
# src/audio/CuePlayback (one Mixer owned by runApplication).
set(WEBOLUTION_GFX_SOURCES
  # app -- the A4 presentation layer: camera, rasteriser, tube scene, entry point
  src/app/Camera.cpp
  src/app/CameraIntro.cpp
  src/app/CameraInput.cpp
  src/app/SimClock.cpp
  src/app/RuntimeHud.cpp
  src/app/LevelEndTransition.cpp
  src/app/EntryStartCue.cpp
  src/app/LevelMenu.cpp
  src/app/RuntimeSkillPanel.cpp
  src/app/Pick.cpp
  src/app/RuntimePick.cpp
  src/app/RuntimeCursor.cpp
  src/app/CursorPolicy.cpp
  src/app/Raster.cpp
  src/app/SceneRaster.cpp
  src/app/SceneClip.cpp
  src/app/ActorSprite.cpp
  src/app/ActorPose.cpp
  src/app/ActorTerrain.cpp
  src/app/ActorAnim.cpp
  src/app/LemmingAnim.cpp
  src/app/LemmingCountdown.cpp
  src/app/TubeScene.cpp
  src/app/MapScene.cpp
  src/app/FrontendSession.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendWindow.cpp
  src/shell/Frontend.cpp
  src/app/BackdropScene.cpp
  src/app/SceneBillboardSource.cpp
  # scene-branch-impl-001: CRT rand/srand LCG for branch-1 scenery first-draw frames.
  src/app/CrtRand.cpp
  # C1 fixture renderer, cached for --sim runtime composition as well.
  src/app/SkillPanel.cpp
  # C2g: HUD counters and timer. MenuRender.cpp (font table, textWidth) is not in the shared
  # object library, so it is compiled into each executable that uses it.
  src/shell/HudText.cpp
  # C2i: pause caption panel.
  src/shell/PauseCaption.cpp
  src/shell/MenuRender.cpp
  src/app/main.cpp
)
list(TRANSFORM WEBOLUTION_GFX_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
