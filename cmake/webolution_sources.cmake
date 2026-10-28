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
  src/assets/Wav.cpp

  # gfx -- renderer-independent sprite orientation selection
  src/gfx/SpriteView.cpp

  # sim -- the game itself, independent of rendering
  src/sim/Terrain.cpp
  src/sim/World.cpp
  src/sim/RetailLevel.cpp

  # shell -- progress / unlock (no SDL)
  src/shell/Progress.cpp

)

list(TRANSFORM WEBOLUTION_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Presentation sources for the runnable target. The flat-first renderer and
# placeholder shell moved to old/src/gfx in the 2026-10-18 reset; the 3D-first
# replacement lands here. src/audio/Mixer.cpp is kept and correct but is not
# wired into the entry point yet (it would need an SDL audio device).
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
  src/app/TubeScene.cpp
  src/app/MapScene.cpp
  src/app/FrontendSession.cpp
  src/app/ResultsScene.cpp
  src/app/FrontendWindow.cpp
  src/shell/Frontend.cpp
  src/app/BackdropScene.cpp
  src/app/SceneBillboardSource.cpp
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
