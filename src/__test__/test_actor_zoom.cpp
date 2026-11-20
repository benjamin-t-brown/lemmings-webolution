// Check reference-camera geometry across yaw, elevation, level height and intro.
#include "app/ActorSprite.h"
#include "app/ActorOcclusion.h"
#include "app/RuntimePick.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace webo;
using namespace webo::app;

double viewZ(const CameraBasis& camera, const std::array<double, 3>& point) {
  const auto& m = camera.worldToView;
  return m[2]*point[0] + m[5]*point[1] + m[8]*point[2] + m[11];
}

int main() {
  SpriteFrame frame;
  frame.width = 26; frame.height = 38;
  frame.unknown0 = 13; frame.unknown1 = 3;
  frame.right = 26; frame.bottom = 38;
  frame.uRight = frame.uBottom = 1;
  Image sheet;
  sheet.width = 26; sheet.height = 38;
  sheet.rgba.assign(26*38*4, 255);
  int cases = 0;
  for (double yaw : {0.0, 32.0, 129.0, 255.0})
    for (double height : {48.0, 96.0, 160.0})
      for (double intro : {-1.0, 0.0, 0.5, 2.5})
        for (double elevation : {-10.5, 0.0, 47.5}) {
          CameraParams params;
          params.yawUnits = yaw; params.levelEyeHeight = height;
          params.introActive = intro >= 0;
          params.introSeconds = std::max(0.0, intro);
          params.elevation = elevation;
          const auto base = buildCamera(params);
          params.zoomShift = 1;
          const auto zoom = buildCamera(params);
          assert(base.defaultZoomDepthOffset == 0);
          for (double lane : {-40.0, 0.0, 40.0}) {
            // Build the reference camera independently at three depths, including the actor anchor.
            const std::array<double, 3> point{-56.9983 + lane, 32, 6.06948};
            const double z0 = viewZ(base, point), z1 = viewZ(zoom, point);
            assert(z0 > 0 && z1 > 0);
            const double terrainRatio = (base.focalY / z1) / (base.focalY / z0);
            assert(std::abs(zoom.zoomScaleAtDepth(z1) - terrainRatio) < 1e-12);
            for (bool mirror : {false, true}) {
              const auto original = actorSpritePlacement(frame, point, actorProjectionInput(base), mirror);
              const auto unchanged = actorSpritePlacement(frame, point, base, mirror);
              const auto scaled = actorSpritePlacement(frame, point, zoom, mirror);
              const auto fixedZoom = actorSpritePlacement(frame, point, actorProjectionInput(zoom), mirror);
              assert(original && unchanged && scaled && fixedZoom);
              assert(original->left == unchanged->left && original->top == unchanged->top &&
                     original->right == unchanged->right && original->bottom == unchanged->bottom);
              assert(scaled->originX == fixedZoom->originX && scaled->originY == fixedZoom->originY &&
                     scaled->depth == fixedZoom->depth);
              assert(std::abs((scaled->right-scaled->left) / (unchanged->right-unchanged->left) -
                              terrainRatio) < 2e-5);
              assert(std::abs((scaled->bottom-scaled->top) / (unchanged->bottom-unchanged->top) -
                              terrainRatio) < 2e-5);
              for (int gravity : {-1, 1}) {
                const auto box = actorSpriteGravityPlacement(*scaled, gravity);
                assert(std::abs((box.bottom-box.top) - (scaled->bottom-scaled->top)) < 0.001);
                // The rendered rectangle is also the production pick input.
                const ScreenPickCandidate candidate{reinterpret_cast<void*>(1),
                    {int(box.left), int(box.top), int(box.right), int(box.bottom)}, true,
                    {(box.left+box.right)/2, (box.top+box.bottom)/2}};
                assert(pickNearestScreenLemming({&candidate, 1},
                    int(candidate.bodyCentre[0]), int(candidate.bodyCentre[1])) == &candidate);
              }
              ++cases;
            }
          }
        }
  // Drawing and depth retain the physical origin while the screen box grows.
  CameraParams params;
  params.zoomShift = 1;
  const auto camera = buildCamera(params);
  const std::array<double, 3> point{0, 32, 0}; // centre of the zoomed viewport
  const auto box = actorSpritePlacement(frame, point, camera, false);
  assert(box);
  Image image;
  image.width = camera.width; image.height = camera.height;
  image.rgba.assign(image.width*image.height*4, 0);
  assert(drawActorSprite(image, frame, sheet, *box, false));
  int pixels = 0;
  for (int y = 0; y < image.height; ++y)
    for (int x = 0; x < image.width; ++x) {
      const bool covered = x >= box->left && x < box->right && y >= box->top && y < box->bottom;
      assert((image.rgba[(y*image.width+x)*4+3] != 0) == covered);
      pixels += covered;
    }
  assert(pixels > 0);
  assert(std::abs(actorPhysicalDepth(*box,camera.width) - viewZ(camera,point)) < 1e-10);
  const ActorOcclusionPlane plane(camera, point);
  const double z = plane.depthAt(camera.cx,camera.cy);
  assert(std::isfinite(z) && z > 0);
  std::cout << "actor zoom: " << cases << " depth/mirror cases, gravity, picking and raster bounds: PASS\n";
}
