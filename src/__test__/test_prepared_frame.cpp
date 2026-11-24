#define SDL_MAIN_HANDLED
#include "app/AppEntry.h"
#include "app/PreparedLevelFrame.h"
#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/TextureCache.h"
#include "sim/RetailLevel.h"

#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <tuple>

using namespace webo;
using namespace webo::app;

namespace {

template <class Check>
void checkDisplayedInput(const std::filesystem::path& game, Check check) {
  constexpr sim::Skill skill = sim::Skill::Blocker;
  constexpr auto stockSlot = static_cast<std::size_t>(skill);
  int frames = 0, x = 0, y = 0, stock = 0;
  std::size_t hatch = 0, actor = 0;
  std::optional<CameraZoomPose> zoom;
  const auto push = [&](SDL_Event event) { return SDL_PushEvent(&event) == 1; };
  const auto key = [&](SDL_Scancode code) {
    SDL_Event event{};
    event.type = SDL_KEYDOWN;
    event.key.keysym.scancode = code;
    event.key.keysym.sym = SDL_GetKeyFromScancode(code);
    return push(event);
  };
  const auto motion = [&](int px, int py) {
    SDL_Event event{};
    event.type = SDL_MOUSEMOTION;
    event.motion.x = px;
    event.motion.y = py;
    return push(event);
  };
  const auto click = [&](int px, int py) {
    SDL_Event event{};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = px;
    event.button.y = py;
    if (!push(event)) return false;
    event.type = SDL_MOUSEBUTTONUP;
    return push(event);
  };
  const auto observer = [&](const LevelFrameView& frame) {
    ++frames;
    if (!frame.queries) {
      check(false, "application supplies a displayed-frame query token");
      return;
    }
    const auto& world = frame.world;
    switch (frame.frame) {
      case 0: {
        const auto hatches = frame.queries->hatches();
        check(!hatches.empty(), "Kriss Kross displays a hatch for SDL input");
        if (hatches.empty()) return;
        const auto& target = hatches.front();
        hatch = target.index;
        x = (target.left + target.right) / 2;
        y = (target.top + target.bottom) / 2;
        check(key(SDL_SCANCODE_4) && key(SDL_SCANCODE_P) && motion(x, y),
              "select Blocker, pause, and move over the displayed hatch");
        break;
      }
      case 1: {
        check(frame.paused && frame.selectedSkillIndex == int(stockSlot), "Blocker selected in paused level");
        check(frame.hoveredHatch == hatch && frame.cursorState == CursorState::Entrance && !frame.hoveredActor,
              "SDL motion produces hover and the entrance cursor for the displayed hatch");
        std::cout << "hatch index=" << hatch << " pointer=" << x << ',' << y << '\n';
        const auto target = std::ranges::find_if(frame.actors, [&](const LevelActorView& candidate) {
          const auto& box = candidate.placement;
          return world.canAssign(candidate.index, skill) && box.left > 100 && box.right < 640 &&
                 box.top >= 0 && box.bottom < 480;
        });
        check(target != frame.actors.end(), "Kriss Kross displays a lemming eligible for Blocker");
        if (target == frame.actors.end()) return;
        actor = target->index;
        stock = world.setup().skills[stockSlot];
        x = static_cast<int>((target->placement.left + target->placement.right) / 2);
        y = static_cast<int>((target->placement.top + target->placement.bottom) / 2);
        check(stock > 0 && motion(x, y), "aim at the chosen actor with positive Blocker stock");
        break;
      }
      case 2:
        check(frame.hoveredActor == actor && !frame.hoveredHatch,
              "actor hover follows the displayed actor box and clears hatch hover");
        check(click(x, y), "click the actor indicated by the displayed frame");
        break;
      case 3: {
        const auto blockers = std::ranges::count_if(world.lemmings(),
            [](const auto& lemming) { return lemming.state == sim::State::Blocker; });
        check(actor < world.lemmings().size() && world.lemmings()[actor].state == sim::State::Blocker &&
              blockers == 1 && world.setup().skills[stockSlot] == stock - 1 && !frame.paused,
              "one skill is spent on the displayed actor and play resumes");
        std::cout << "assignment actor=" << actor << " stock=" << stock << "->" << world.setup().skills[stockSlot] << '\n';
        const auto depth = frame.queries->pointDepth(x, y);
        if (depth)
          zoom = cursorZoomPose(frame.queries->camera(), frame.displayedPose, x + 0.5, y + 0.5, *depth);
        check(zoom && std::abs(zoom->lateralOffset) > 1e-4 && key(SDL_SCANCODE_SPACE),
              "queue zoom over a displayed pixel with a non-default anchor");
        break;
      }
      case 4:
        check(zoom && frame.displayedPose.zoomShift == 1 &&
              std::abs(frame.displayedPose.zoomLateralOffset - zoom->lateralOffset) < 1e-9 &&
              std::abs(frame.displayedPose.elevation - zoom->elevation) < 1e-9,
              "SDL zoom applies the pose anchored to the displayed pixel depth");
        std::cout << std::setprecision(12) << "zoom pointer=" << x << ',' << y
                  << " lateral=" << frame.displayedPose.zoomLateralOffset
                  << " elevation=" << frame.displayedPose.elevation << '\n';
        break;
    }
  };
  std::vector<std::string> args{"prepared-frame", "--direct-level", "--game-dir", game.string(),
      "--level", "E19_KF_2", "--native", "--sim-ticks", "180", "--frames", "5", "--window-scale", "1", "--no-audio"};
  std::vector<char*> raw;
  for (auto& arg : args) raw.push_back(arg.data());
  ApplicationRunOptions options;
  options.scenerySeed = 0x75438021;
  options.frameSeconds = 1.0 / 60.0;
  check(runApplication(int(raw.size()), raw.data(), {}, observer, options) == 0 && frames == 5,
        "bounded SDL application input check completes");
}

}  // namespace

int main(int argc, char** argv) {
  const char* videoDriver = std::getenv("SDL_VIDEODRIVER");
  if (argc != 2 || !videoDriver || std::string_view(videoDriver) != "dummy") return 2;
  const std::filesystem::path game(argv[1]);
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) return 2;
  TextureCacheScope textures(*library);
  auto bytes = library->read("COMPILED\\E19_KF_2.LVL");
  if (!bytes) return 2;
  const auto index = indexLevel(*bytes);
  const auto mesh = findCylinderMesh(*bytes);
  if (!index || !mesh) return 2;
  auto tube = TubeScene::build(*library, *mesh, *bytes, index->grid0);
  auto backdrop = BackdropScene::build(*library, *bytes, "E19_KF_2");
  auto setup = sim::loadRetailLevel(*library, "E19_KF_2");
  auto font = loadTexture(*library, "TEXTURES\\^FONT.BMP");
  auto panel = RetailSkillPanel::create(*library, "E19_KF_2");
  if (!tube || !backdrop || !setup || !font || !panel) return 2;

  int failures = 0;
  const auto check = [&](bool value, const char* description) {
    std::cout << (value ? "PASS " : "FAIL ") << description << '\n';
    failures += !value;
  };
  sim::World world(*setup);
  for (int i = 0; i < 180; ++i) world.tick();
  const auto clock = world.levelClockMs();
  const auto spawned = world.spawned();
  const auto version = world.terrain().version();
  tube->syncTerrain(world.terrain(), world.setup().counterGates);
  tube->syncCounterGates(world.setup().counterGates);
  sim::Lemming particleSource;
  particleSource.x = 32;
  particleSource.y = 48;
  const std::array<std::array<int, 2>, 1> chips{{{32, 48}}};
  tube->emitTerrainDebris(particleSource, chips, clock, world.terrain().height());
  const auto particleCount = tube->deathParticleCount();
  CrtRand random(0x75438021);
  backdrop->setRandom(&random);

  PreparedLevelFrame prepared;
  auto& scene = prepared.scene;
  scene.camera = buildCamera(CameraParams{});
  scene.cachePose = scene.cacheTube = true;
  using Pass = LevelScenePass;
  scene.passes = prepareLevelPassOrder(true, false, false, false);
  check(scene.passes == std::vector{Pass::Primary, Pass::LegacyUnattributed, Pass::Tube, Pass::GadgetEffects,
        Pass::Pipes, Pass::Fluid, Pass::Secondary, Pass::CounterGates, Pass::ExplosionMarks, Pass::Actors,
        Pass::Particles, Pass::TeleportEffects}, "prepared scene records the complete normal pass order");
  const auto diagnostic = prepareLevelPassOrder(true, true, true, true);
  check(diagnostic == std::vector{Pass::Primary, Pass::LegacyUnattributed, Pass::TubeBeforeRoute, Pass::EarlyActors,
        Pass::TubeRoute, Pass::GadgetEffects, Pass::Pipes, Pass::Fluid, Pass::Secondary, Pass::CounterGates,
        Pass::BlueSky, Pass::CompleteDrowningDepth, Pass::ExplosionMarks, Pass::Particles, Pass::TeleportEffects},
        "prepared order includes the diagnostic tube split, sky fill, and completed drowning depth");
  SceneOptions options;
  options.drawWall = false;
  options.drawCellRoute = true;
  options.injectedWalkers = injectedWalkersFromWorld(world, world.terrain().height());
  using Stage = BackdropScene::Stage;
  const auto stage = [&](Stage pass, bool observe = false) {
    return backdrop->prepareStage(scene.camera, *library, pass, false, observe);
  };
  scene.primary = std::make_shared<BackdropScene::PreparedStage>(stage(Stage::Primary));
  scene.legacy = stage(Stage::LegacyUnattributed);
  scene.tube = std::make_shared<TubeScene::PreparedTube>(tube->prepareTube(scene.camera, options, false));
  scene.gadgetEffects = stage(Stage::GadgetEffect);
  scene.pipes = stage(Stage::FluidPipe);
  scene.fluid = stage(Stage::FluidGate);
  scene.secondary = stage(Stage::Secondary, true);
  scene.counterGates = tube->prepareCounterGates(scene.camera, *library, world.setup().counterGates);
  scene.actors = tube->prepareActors(scene.camera, options, *library, true);
  scene.particles = tube->prepareDeathParticles(scene.camera, *library);
  const auto randomState = random.state();
  const auto latches = BackdropScene::telemetry().firstDrawLatches;
  check(backdrop->firstSpriteDrawRandomState() == 0x75438021 && randomState != 0x75438021,
        "secondary preparation resolves the branch-1 first-draw RNG");
  check(particleCount > 0 && !scene.particles.draws.empty(), "prepared records contain live debris");

  EntryStartCue cue(false);
  const auto cueFrame = cue.frame();
  if (!cueFrame) return 2;
  auto entry = cue.preparePresentation(*font, "Let's Go", *cueFrame);
  check(entry.playVoice && !cue.preparePresentation(*font, "Let's Go", *cueFrame).playVoice,
        "only the first prepared entry cue requests voice playback");
  auto preparedPanel = panel->prepare(panel->initialCounts(), 3, true, nullptr);
  const auto plateAt = [](const PreparedSkillPanel::Plate& plate, int i) {
    return plate.left == 18 && plate.top == 12 + 40 * i && plate.width == 72 && plate.height == 32;
  };
  check(preparedPanel.plates.size() == 12 && plateAt(preparedPanel.plates.front(), 0) &&
        plateAt(preparedPanel.plates[9], 9) && plateAt(preparedPanel.plates[10], 3) &&
        plateAt(preparedPanel.plates[11], 8) && preparedPanel.plates[10].texture == preparedPanel.plates[11].texture &&
        preparedPanel.plates[10].texture != preparedPanel.plates.front().texture,
        "skill panel stores base, selected and fast-forward plate rectangles in draw order");
  prepared.layers.draws.emplace_back(std::move(preparedPanel));
  prepared.layers.draws.emplace_back(std::move(entry.draw));
  std::array labels{shell::HudLine{"250", 550, 15}};
  prepared.layers.appendText(*font, labels);

  std::vector<std::uint8_t> mask(640*480);
  for (std::size_t p = 0; p < mask.size(); p += 37) mask[p] = 1;
  using Event = std::tuple<char, std::uint64_t, std::uint64_t, std::uint64_t>;
  const auto draw = [&](SoftwareLevelRaster& software, Raster& target) {
    std::vector<Event> events;
    SceneRasterObserver observer;
    observer.mask = &mask;
    observer.candidate = [&](auto source, auto node, auto, auto) { events.emplace_back('C', source, node, 0); };
    observer.accepted = [&](const SceneRasterWriteEvent& e) {
      events.emplace_back('W', e.source, e.triangle, e.pixel);
    };
    BackdropStats stats;
    software.draw(target, prepared.scene, stats, nullptr, &observer);
    return events;
  };
  Raster first(640, 480), second(640, 480);
  SoftwareLevelRaster software, independent;
  const auto firstEvents = draw(software, first);
  const auto secondEvents = draw(independent, second);
  check(world.levelClockMs() == clock && world.spawned() == spawned && world.terrain().version() == version &&
        tube->deathParticleCount() == particleCount, "two consumers leave the world and particle lifetime alone");
  check(random.state() == randomState && BackdropScene::telemetry().firstDrawLatches == latches,
        "two consumers preserve the RNG and scenery latches");
  check(!firstEvents.empty() && firstEvents == secondEvents, "secondary observer callbacks retain submission order");

  DisplayedFrameQueries queries(scene.camera, first);
  queries.setSceneResults(scene.actors.boxes, backdrop->hatchHoverTargets(scene.camera, first));
  queries.setLayers(prepared.layers);
  check(queries.imageStorage().rgba.empty(), "composed image is lazy");
  const auto& image = queries.composedImage();
  DisplayedFrameQueries other(scene.camera, second);
  other.setLayers(prepared.layers);
  check(first.depthBuffer() == second.depthBuffer() && image.rgba == other.composedImage().rgba,
        "two independent consumers produce the same depth and composed pixels");

  const auto boxes = tube->injectedWalkerScreenBoxes(scene.camera, options, *library, first);
  const auto equalBox = [](const auto& a, const auto& b) {
    const auto values = [](const auto& box) {
      return std::tie(box.originX, box.originY, box.depth, box.left, box.top, box.right, box.bottom);
    };
    return bool(a) == bool(b) && (!a || values(*a) == values(*b));
  };
  check(std::ranges::any_of(boxes, [](const auto& box) { return box.has_value(); }) &&
        std::ranges::equal(queries.actorBoxes(), boxes, equalBox), "actor pick boxes match the placement adapter");
  check(!queries.hatches().empty(), "software depth supplies a visible hatch rectangle");
  if (!queries.hatches().empty()) {
    const auto& h = queries.hatches().front();
    check(RuntimeReleaseRateControl::pressTarget(queries.hatches(), h.left, h.top, 1, false) == 0 &&
          !RuntimeReleaseRateControl::pressTarget(queries.hatches(), h.left, h.top, 1, true) &&
          !hoveredHatch(queries.hatches(), h.right, h.top, h.index),
          "hatch query keeps the half-open rectangle and left-click actor precedence");
  }
  std::size_t actorSamples = 0, sceneSamples = 0;
  bool sameZoomDepths = !queries.pointDepth(-1, 0);
  for (std::size_t p = 0; p < first.depthBuffer().size(); ++p) {
    const auto actor = first.actorDepth().depthAt(p);
    const auto sceneDepth = first.depthBuffer()[p];
    const bool covered = std::isfinite(actor);
    actorSamples += covered;
    sceneSamples += !covered && std::isfinite(sceneDepth);
    sameZoomDepths &= queries.pointDepth(p % 640, p / 640) == (covered ? actor : sceneDepth);
  }
  check(sameZoomDepths && actorSamples && sceneSamples, "zoom queries use actor depth before scene depth");

  tube->advanceDeathParticles(clock + 60000);
  check(tube->deathParticleCount() == 0, "producer debris can expire after frame preparation");
  Raster cached(640, 480);
  const auto cacheEvents = draw(software, cached);
  DisplayedFrameQueries cachedQueries(scene.camera, cached);
  cachedQueries.setLayers(prepared.layers);
  check(firstEvents == cacheEvents && cached.depthBuffer() == first.depthBuffer() &&
        cachedQueries.composedImage().rgba == image.rgba,
        "cached redraw still uses the submitted debris, scenery and cue records");
  checkDisplayedInput(game, check);
  return failures ? 1 : 0;
}
