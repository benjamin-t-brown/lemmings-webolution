#include "app/ActorOcclusion.h"
#include "app/ActorSprite.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
  using namespace webo::app;
  int checks = 0;
  // Independent known points on an upright plane: transform and project each
  // world point, then recover its depth from its screen coordinates. Include
  // both sides, inner/outer lanes, body heights and pitches. A constant-depth
  // billboard or a foreground bias fails these checks.
  for (double pitch : {0.0, -0.03135770272046, 0.04})
    for (int turn = 0; turn < 48; ++turn) {
      CameraParams p;
      p.yawUnits = turn * 256.0 / 48;
      p.fixedPitchTurns = pitch;
      const auto camera = buildCamera(p);
      const auto& m = camera.worldToView;
      for (double radius : {52.5, 57.5}) {
        const std::array<double, 3> origin{radius, 40, 0};
        const ActorOcclusionPlane plane(camera, origin);
        for (double height : {-3., 0., 3., 8.})
          for (double horizontal : {-2., 0., 2.}) {
            const std::array<double, 3> point{
                origin[0] - m[8] * horizontal, origin[1] + height,
                origin[2] + m[2] * horizontal};
            std::array<double, 3> view{};
            for (int k = 0; k < 3; ++k)
              view[k] = m[k] * point[0] + m[k + 3] * point[1] +
                        m[k + 6] * point[2] + m[k + 9];
            const double x = camera.cx + camera.focalX * view[0] / view[2];
            const double y = camera.cy - camera.focalY * view[1] / view[2];
            const double depth = plane.depthAt(x, y);
            if (std::abs(depth - view[2]) > 1e-9) {
              std::cerr << "upright world point has wrong comparison depth\n";
              return 1;
            }
            ++checks;
          }
      }
    }
  for (int width : {640, 1280}) {
    ActorSpritePlacement placement;
    placement.depth = 300.0 * 0.3 / width - 0.0003;
    if (std::abs(actorPhysicalDepth(placement, width) - 300.0) > 1e-10) {
      std::cerr << "screen projection bias leaked into physical depth\n";
      return 1;
    }
  }
  CameraBasis flat;
  flat.worldToView = {1,0,0, 0,1,0, 0,0,1, 0,0,0};
  flat.focalX = flat.focalY = 200;
  if (std::isfinite(ActorOcclusionPlane(flat, {0,0,-10}).depthAt(319,239))) {
    std::cerr << "behind-camera plane should reject\n";
    return 1;
  }
  std::cout << checks << " independently projected upright points: PASS\n";

  if (argc < 2 || std::string_view(argv[1]).empty()) return 0;
  // Supplemental installed-SPR mask contract, not a gameplay route. A depth
  // surface 0.01 world units in front must hide the complete figure; the same
  // surface behind must preserve it. Picking keeps the actor's box in both
  // cases; highlighting remains masked exactly like drawing.
  const std::filesystem::path game = argv[1];
  auto library = webo::AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) return 1;
  std::string levelPath;
  for (const auto& name : webo::levelFileNames(*library))
    if (name.find("MM_CLIMB_002") != std::string::npos) { levelPath = name; break; }
  auto level = library->read(levelPath);
  if (!level) return 1;
  auto mesh = webo::findCylinderMesh(*level);
  auto index = webo::indexLevel(*level);
  if (!mesh || !index) return 1;
  auto scene = TubeScene::build(*library, *mesh, *level, index->grid0);
  if (!scene) return 1;
  CameraParams params;
  params.yawUnits = 49;
  const auto camera = buildCamera(params);
  const webo::ActorPoseLevel poseLevel{int(mesh->faceCount), index->grid0, mesh->surface, &*mesh};
  const auto pose = webo::actorPoseFromCylinderSample(poseLevel, {49,32,.75}, 1);
  const ActorOcclusionPlane body(camera, pose.modelPosition);
  SceneOptions options;
  options.injectedWalkers = {{49,32,.75,1}};
  Raster empty(640,480);
  empty.clear(19,23,31);
  auto raw = empty;
  auto diagnostic = options;
  diagnostic.composeSkipActorVisibility = true;
  scene->drawInjectedWalkers(raw, camera, diagnostic, *library);
  if (raw.framebuffer().rgb == empty.framebuffer().rgb) return 1;
  const auto originalDepth = empty.depthBuffer();
  for (double offset : {-0.01, 0.01}) {
    auto frame = empty;
    for (int y = 0; y < 480; ++y)
      for (int x = 0; x < 640; ++x)
        frame.mutableDepthBuffer()[y*640+x] = body.depthAt(x+.5,y+.5) + offset;
    const auto depth = frame.depthBuffer();
    scene->drawInjectedWalkers(frame, camera, options, *library);
    const auto boxes = scene->injectedWalkerScreenBoxes(camera, options, *library, frame);
    const auto& expected = offset < 0 ? empty.framebuffer().rgb : raw.framebuffer().rgb;
    if (frame.framebuffer().rgb != expected || !boxes[0] ||
        frame.depthBuffer() != depth) {
      std::cerr << "foreground/contact plane draw/pick/depth contract failed\n";
      return 1;
    }
    webo::Image highlighted;
    highlighted.width = 640;
    highlighted.height = 480;
    highlighted.rgba.assign(640 * 480 * 4, 0);
    const auto originalHighlight = highlighted.rgba;
    if (scene->drawHoveredWalker(highlighted, 0, camera, options, *library, frame) != (offset > 0) ||
        (highlighted.rgba != originalHighlight) != (offset > 0)) {
      std::cerr << "picking must not expose a depth-hidden highlight\n";
      return 1;
    }
    if (offset < 0) {
      // Later fluid may replace scene Z, but cannot erase opaque pipe coverage.
      auto pipeFrame = empty;
      auto pipeOptions = options;
      pipeOptions.pipeSceneDepth = depth;
      scene->drawInjectedWalkers(pipeFrame, camera, pipeOptions, *library);
      const auto pipeBoxes = scene->injectedWalkerScreenBoxes(camera, pipeOptions, *library, pipeFrame);
      if (pipeFrame.framebuffer().rgb != empty.framebuffer().rgb || !pipeBoxes[0] ||
          pipeFrame.depthBuffer() != originalDepth) {
        std::cerr << "pipe depth must survive later scenery depth\n";
        return 1;
      }
    }
  }
  std::cout << "installed sprite foreground/contact/pipe masks and picking: PASS\n";
}
