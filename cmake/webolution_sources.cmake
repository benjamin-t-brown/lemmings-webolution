# Production sources, grouped by domain.
#
# The format readers live under src/assets/retail in the standalone tree.
set(WEBOLUTION_RETAIL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/retail")
if(NOT IS_DIRECTORY "${WEBOLUTION_RETAIL_DIR}")
  set(WEBOLUTION_RETAIL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/decomp-lemmings")
endif()

set(WEBOLUTION_SOURCES
  # assets -- reading the original game's data files
  src/assets/Box.cpp
  src/assets/Rar20.cpp
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
  src/save/SoundSettings.cpp

)

list(TRANSFORM WEBOLUTION_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Decoder for the original bitmap envelope.
set(WEBOLUTION_TEXTURE_DECODER_SOURCES
  ${WEBOLUTION_RETAIL_DIR}/src/texture_resource.cpp
)

# Serialized SPR reading only; original runtime allocation and rendering stay
# outside this compatibility boundary. level_stream.cpp (the shared bounded
# string primitive) is compiled here and nowhere else, so the compiled-level
# target below links this one rather than listing it again.
set(WEBOLUTION_SPRITE_RESOURCE_SOURCES
  ${WEBOLUTION_RETAIL_DIR}/src/sprite_resource.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/level_stream.cpp
)

# Serialized compiled-level (.LVL) reading only, for the hatch entity list
# production level loading needs (src/assets/LevelObjects.cpp ->
# lemmings::read_compiled_level). Exactly the translation units
# read_compiled_level needs and no more: box_archive.cpp is here only because
# load_compiled_level sits in the same translation unit as read_compiled_level
# and references it -- production archive reads stay on src/assets/Box.cpp.
set(WEBOLUTION_COMPILED_LEVEL_SOURCES
  ${WEBOLUTION_RETAIL_DIR}/src/compiled_level.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/compiled_level_header.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/level_name_tables.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/world_geometry_prefix.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/world_coordinates.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/object_base_prefix.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/level_entity_prefix.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/level_gadget_records.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/level_model.cpp
  ${WEBOLUTION_RETAIL_DIR}/src/box_archive.cpp
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

# Presentation sources for the executable.
set(WEBOLUTION_GFX_SOURCES
  # app -- camera, rasteriser, tube scene and entry point
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
  src/app/SoundMenu.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendWindow.cpp
  src/shell/Frontend.cpp
  src/app/BackdropScene.cpp
  src/app/SceneBillboardSource.cpp
  # CRT rand/srand LCG for scenery first-draw frames.
  src/app/CrtRand.cpp
  src/app/SkillPanel.cpp
  # HUD counters and timer.
  src/shell/HudText.cpp
  src/shell/PauseCaption.cpp
  src/shell/MenuRender.cpp
  src/app/main.cpp
)
list(TRANSFORM WEBOLUTION_GFX_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
