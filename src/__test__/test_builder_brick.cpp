// Check the carried-to-solid brick handoff and its shared draw, pick and occlusion rules.
#include "app/ActorOcclusion.h"
#include "app/BuilderBrick.h"
#include "app/LemmingAnim.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <numbers>

using namespace webo;
using namespace webo::app;

void checkCarriedRotation(int facing, int gravity) {
  constexpr Vec3 offset{3, 2, 1};
  constexpr Vec3 inward{0, 0, 1};
  const auto expectRotation = [&](double travel, Vec3 expected) {
    const auto rotation = builderBrickRotation(travel, facing, gravity);
    const auto result = rotation.apply(offset, inward);
    assert(std::abs(result.x - expected.x) < 1e-12);
    assert(std::abs(result.y - expected.y) < 1e-12);
    assert(std::abs(result.z - expected.z) < 1e-12);
    const auto side = rotation.apply({offset.z, offset.y, -offset.x}, {1, 0, 0});
    assert(std::abs(side.x - expected.z) < 1e-12);
    assert(std::abs(side.y - expected.y) < 1e-12);
    assert(std::abs(side.z + expected.x) < 1e-12);
  };
  const bool positiveYaw = facing != gravity;
  const double root2 = std::numbers::sqrt2;
  const auto start = builderBrickShape(2.0 / 16.0);
  const auto halfway = builderBrickShape(7.5 / 16.0);
  const auto turning = builderBrickShape(10.25 / 16.0);
  const auto lastCarry = builderBrickShape(std::nextafter(13.0 / 16.0, 0.0));
  assert(start.scale == 1.0 / 3.0 && halfway.scale > start.scale);
  assert(lastCarry.scale > turning.scale && std::abs(lastCarry.scale - 1.0) < 1e-12);
  const Vec3 pulledOut = positiveYaw ? Vec3{2, -1, -3} : Vec3{-2, -1, 3};
  expectRotation(start.travel, pulledOut);
  expectRotation(halfway.travel, pulledOut);
  expectRotation(turning.travel, positiveYaw ? Vec3{2 + root2, root2 - 2, -root2}
                                             : Vec3{1 - root2, root2 + 1, 2 * root2});
  expectRotation(lastCarry.travel, offset);
  expectRotation(1.0, offset);
}

void checkBuilderHeadroom(int facing) {
  using namespace sim;
  Lemming builder;
  builder.x = facing > 0 ? 14 : 1;
  builder.y = 20;
  builder.facing = facing > 0 ? Facing::Right : Facing::Left;
  builder.state = State::Builder;

  const auto placeBrick = [&](int bricks, int ceilingOffset) {
    LevelSetup setup;
    setup.terrain = Terrain(16, 32);
    setup.terrain.set(builder.x + ceilingOffset * facing, builder.y - 4, Material::Earth);
    World world(std::move(setup));
    Lemming actor = builder;
    actor.bricksLeft = bricks;
    world.place(actor);
    for (int i = 0; i < 4; ++i)
      world.tick();
    return world;
  };

  const World above = placeBrick(2, 1);
  assert(above.lemmings()[0].state == State::Builder);
  const World ahead = placeBrick(2, 2);
  const auto& stopped = ahead.lemmings()[0];
  assert(stopped.state == State::Walker && stopped.facing != builder.facing);
  assert(stopped.x == builder.x + facing && stopped.y == builder.y - 1 && stopped.bricksLeft == 1);

  World finished = placeBrick(1, 2);
  assert(finished.lemmings()[0].state == State::Shrugger);
  for (int i = 0; i < 6 && finished.lemmings()[0].state == State::Shrugger; ++i)
    finished.tick();
  assert(finished.lemmings()[0].state == State::Walker);
  finished.tick();
  const auto& walked = finished.lemmings()[0];
  assert(walked.facing == builder.facing && walked.state == State::Walker);
  assert(walked.x == finished.terrain().wrapX(builder.x + 2 * facing) && walked.y == builder.y - 1);
}

int main(int argc, char** argv) {
  assert(builderBrickShape(-1).scale == 0 && builderBrickShape(0.9).scale == 0);
  assert(builderBrickShape(2.0/16).scale > 0);
  assert(builderBrickShape(13.0/16).scale == 0);
  assert(builderBrickShape(0.7).scale > builderBrickShape(0.3).scale);
  for (int facing : {-1, 1}) {
    for (int gravity : {-1, 1}) checkCarriedRotation(facing, gravity);
    checkBuilderHeadroom(facing);
    sim::LevelSetup setup;
    setup.terrain = sim::Terrain(256, 96);
    setup.totalLemmings = 0;
    sim::World world(setup);
    sim::Lemming l;
    l.x = facing > 0 ? 255 : 0; // cross both seam directions
    l.y = 64; l.state = sim::State::Builder;
    l.facing = facing > 0 ? sim::Facing::Right : sim::Facing::Left;
    l.bricksLeft = world.rules().builderBricks;
    world.place(l);
    sim::World grouped(setup);
    grouped.place(l);
    int placed = 0;
    while (world.lemmings()[0].state == sim::State::Builder) {
      const auto before = world.lemmings()[0];
      const double phase = builderPresentationPhase(world, before);
      const auto version = world.terrain().version();
      // Reading a paused frame is stable and has no simulation side effects.
      assert(phase == builderPresentationPhase(world, before));
      assert(version == world.terrain().version());
      world.advanceElapsedMs(1);
      // Four 17 ms presentation passes may be consumed before one draw at
      // fast-forward. Sampling depends on confirmed World state, not draw calls.
      if (world.levelClockMs() % 68 == 0) {
        for (int pass = 0; pass < 4; ++pass) grouped.advanceElapsedMs(17);
        const auto& fast = grouped.lemmings()[0];
        assert(fast.bricksLeft == world.lemmings()[0].bricksLeft);
        assert(builderPresentationPhase(grouped, fast) ==
               builderPresentationPhase(world, world.lemmings()[0]));
      }
      const auto& after = world.lemmings()[0];
      if (after.bricksLeft != before.bricksLeft) {
        ++placed;
        assert(builderBrickShape(phase).scale > 0.99);
        assert(builderBrickShape(builderPresentationPhase(world, after)).scale == 0.0);
        for (int k = 1; k <= world.rules().brickLength; ++k)
          assert(world.terrain().solid(before.x + k * facing, before.y));
      }
      assert(world.levelClockMs() < 15000);
    }
    assert(placed == 12);
  }
  std::cout << "all 12 carried/solid handoffs, both seams, grouped fast-forward sampling: PASS\n";
  if (argc < 2) return 0;
  const std::filesystem::path game(argv[1]);
  auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  assert(lib);
  std::string name;
  for (const auto& n : levelFileNames(*lib))
    if (n.find("MM_BUILD.LVL") != std::string::npos) { name = n; break; }
  auto bytes = lib->read(name); assert(bytes);
  auto mesh = findCylinderMesh(*bytes); auto index = indexLevel(*bytes);
  assert(mesh && index);
  auto scene = TubeScene::build(*lib, *mesh, *bytes, index->grid0); assert(scene);
  const ActorPoseLevel poseLevel{int(mesh->faceCount), index->grid0, mesh->surface, &*mesh};
  for (int type = 0; type < 3; ++type) {
    const auto* sheet = scene->lemmingSprites(type); assert(sheet);
    const auto* clip = sheet->find("build"); assert(clip);
    assert(clip->animations.size() == 16 && clip->rate == 1.0);
    std::cout << "installed build sheet " << type << ": 16 entries, rate 1\n";
  }
  for (int facing : {-1, 1}) for (int gravity : {-1, 1}) {
    CameraParams params; params.yawUnits = 49;
    const auto camera = buildCamera(params);
    SceneOptions options;
    auto& w = options.injectedWalkers.emplace_back();
    w.plus8X = 49; w.plus8Y = 32; w.plus8Z = 0.75;
    w.facing = facing; w.verticalDir = gravity; w.clip = "build";
    w.builderX = 49; w.builderY = 32;
    for (double phase : {0.2, 0.45, 0.7, 0.81249}) {
      w.builderPhase = phase; w.walkTemporalIndex = int(phase * 16);
      Raster empty(640,480); empty.clear(19,23,31);
      auto raw = empty;
      scene->drawInjectedWalkers(raw, camera, options, *lib);
      auto bodyOptions = options; bodyOptions.injectedWalkers[0].builderPhase = -1;
      auto bodyOnly = empty;
      scene->drawInjectedWalkers(bodyOnly, camera, bodyOptions, *lib);
      assert(raw.framebuffer().rgb != bodyOnly.framebuffer().rgb);
      assert(raw.depthBuffer() == empty.depthBuffer());
      const auto boxes = scene->injectedWalkerScreenBoxes(camera, options, *lib, raw);
      const auto bodyBoxes = scene->injectedWalkerScreenBoxes(camera, bodyOptions, *lib, raw);
      assert(boxes[0] && bodyBoxes[0] && boxes[0]->left == bodyBoxes[0]->left &&
             boxes[0]->right == bodyBoxes[0]->right);
      const auto origin = actorPoseFromCylinderSample(poseLevel, {49,32,.75}, facing).modelPosition;
      const ActorOcclusionPlane plane(camera, origin);
      for (double offset : {-0.01, 0.01}) {
        auto frame = empty;
        for (int y = 0; y < 480; ++y) for (int x = 0; x < 640; ++x)
          frame.mutableDepthBuffer()[y*640+x] = plane.depthAt(x+.5,y+.5) + offset;
        const auto depth = frame.depthBuffer();
        scene->drawInjectedWalkers(frame, camera, options, *lib);
        assert(frame.framebuffer().rgb == (offset > 0 ? raw.framebuffer().rgb : empty.framebuffer().rgb));
        assert(frame.depthBuffer() == depth);
        if (offset < 0) {
          auto pipes = options; pipes.pipeSceneDepth = depth;
          auto pipeFrame = empty;
          scene->drawInjectedWalkers(pipeFrame, camera, pipes, *lib);
          assert(pipeFrame.framebuffer().rgb == empty.framebuffer().rgb);
        }
      }
      if (argc > 2) {
        Image image; image.width = 640; image.height = 480;
        image.rgba.resize(640*480*4,255);
        for (std::size_t i = 0; i < image.pixelCount(); ++i)
          std::copy_n(raw.framebuffer().rgb.begin()+i*3,3,image.rgba.begin()+i*4);
        const auto output = std::filesystem::path(argv[2]) /
            ("pose-" + std::to_string(facing) + "-" + std::to_string(gravity) + "-" +
             std::to_string(int(phase*10000)) + ".png");
        assert(writePng(output, image));
        Image crop; crop.width = crop.height = 240;
        crop.rgba.assign(240*240*4,255);
        const int cx = int(boxes[0]->originX), cy = int(boxes[0]->originY) - 8;
        for (int y = 0; y < 240; ++y) for (int x = 0; x < 240; ++x) {
          const int sx = cx - 30 + x/4, sy = cy - 30 + y/4;
          if (sx >= 0 && sx < image.width && sy >= 0 && sy < image.height)
            std::copy_n(image.rgba.begin()+(sy*640+sx)*4,4,crop.rgba.begin()+(y*240+x)*4);
        }
        assert(writePng(output.parent_path() / ("crop-" + output.filename().string()), crop));
      }
    }
  }
  std::cout << "installed mesh visible, body/pipe occlusion shared, picking/depth preserved: PASS\n";
}
