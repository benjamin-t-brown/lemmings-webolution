// Seconds-long PCM/event invariants, not a scripted gameplay route.
#include "audio/CuePlayback.h"
#include "app/ActorSprite.h"
#include "sim/RetailLevel.h"
#include "assets/Box.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace webo::audio {
struct PositionalSoundTestAccess {
  static Mixer& mixer(CuePlayback& p) { return p.mixer_; }
  static void pause(Mixer& m) { SDL_PauseAudioDevice(m.device_, 1); }
  static std::vector<std::int16_t> render(Mixer& m, int frames) {
    std::vector<std::int16_t> out(frames*m.channels());
    m.fill(out.data(), frames);
    return out;
  }
};
}

using namespace webo;
using Access = audio::PositionalSoundTestAccess;

void projection() {
  for (double yaw : {0.0, 32.0, 90.0, 255.999}) for (int zoom : {0, 1}) {
    app::CameraParams params;
    params.yawUnits = yaw; params.zoomShift = zoom;
    const auto camera = app::buildCamera(params);
    const double r = std::hypot(camera.eyeWorld[0], camera.eyeWorld[2]);
    const std::array<double, 3> near{60*camera.eyeWorld[0]/r, 50, 60*camera.eyeWorld[2]/r};
    const std::array<double, 3> far{-near[0], 50, -near[2]};
    const auto front = audio::spatialSoundGains(camera, near);
    const auto back = audio::spatialSoundGains(camera, far);
    assert(std::abs(std::max(front.left, front.right)-1.0f) < 1e-6);
    assert(std::abs(std::max(back.left, back.right)-0.25f) < 1e-6);
    for (int column=0; column<256; ++column) {
      const double a = column*2*3.141592653589793/256;
      const std::array<double, 3> point{60*std::cos(a), 50, 60*std::sin(a)};
      const auto gains = audio::spatialSoundGains(camera, point);
      // Independent sprite placement checks the same physical screen origin.
      SpriteFrame f; f.width=4; f.height=4; f.right=4; f.bottom=4;
      auto placed = app::actorSpritePlacement(f, point, camera, false);
      assert(placed);
      if (placed->originX < camera.cx-0.01) assert(gains.left > gains.right);
      if (placed->originX > camera.cx+0.01) assert(gains.right > gains.left);
      assert(gains.left >= 0 && gains.right >= 0 && gains.left <= 1 && gains.right <= 1);
    }
  }
  app::CameraParams p;
  p.yawUnits=0; const auto a=audio::spatialSoundGains(app::buildCamera(p), {15,50,-55});
  p.yawUnits=256; const auto b=audio::spatialSoundGains(app::buildCamera(p), {15,50,-55});
  assert(std::abs(a.left-b.left)<1e-6 && std::abs(a.right-b.right)<1e-6);
}

void pcm() {
  audio::CuePlayback playback;
  auto& mixer = Access::mixer(playback);
  assert(mixer.open()); Access::pause(mixer);
  Sound tone{22050, 1, std::vector<std::int16_t>(22050, 10000)};
  assert(mixer.load("Tone", tone));
  for (auto overlap : {audio::Mixer::Overlap::Add, audio::Mixer::Overlap::Average,
                       audio::Mixer::Overlap::RestartPlaying}) {
    mixer.stopAll();
    mixer.play("Tone", 1, overlap, {0.5f, 0.125f});
    const auto first = Access::render(mixer, 100);
    // Moving the camera computes future-event gains only; existing PCM is
    // unchanged across callback boundaries, with no gain step/click.
    app::CameraParams params; params.yawUnits=160;
    const auto future = audio::spatialSoundGains(app::buildCamera(params), {20,50,-55});
    assert(future.left != 0.5f || future.right != 0.125f);
    const auto tail = Access::render(mixer, 100);
    assert(first == tail);
    for (std::size_t i=0; i<tail.size(); i+=2) assert(tail[i]==5000 && tail[i+1]==1250);
  }
  mixer.stopAll(); mixer.play("Tone");
  const auto centred = Access::render(mixer, 100);
  for (auto s : centred) assert(s==10000);
  // The full sim event -> playback -> production mixer route. UI assignment
  // feedback is centred; a hazard event carries the victim's location.
  sim::LevelSetup setup; setup.terrain=sim::Terrain(256,64); setup.skills.fill(10);
  setup.totalLemmings=0;
  sim::World world(std::move(setup)); sim::Lemming actor;
  actor.x=23; actor.y=31; actor.state=sim::State::Walker; world.place(actor);
  assert(world.assign(0, sim::Skill::Climber));
  assert(mixer.load("Action", tone)); mixer.stopAll();
  int positions=0;
  playback.drain(world, [&](const std::array<double,3>& position) {
    ++positions; assert(position[0]==23 && position[1]==31);
    return audio::StereoGains{0.25f,1.0f};
  });
  assert(positions==0 && world.takeSoundEvents().empty());
  const auto assigned=Access::render(mixer,1);
  assert(assigned[0]==10000 && assigned[1]==10000);
  sim::LevelSetup hazards; hazards.terrain=sim::Terrain(256,64); hazards.totalLemmings=0;
  sim::Crab crab; crab.boxOrigin={23,32,0};
  crab.volumes.push_back({{0,0,0},{1,1,1}}); crab.attackSound="Tone";
  hazards.crabs.push_back(crab);
  sim::World attacked(std::move(hazards)); attacked.place(actor);
  attacked.advanceElapsedMs(0);
  assert(attacked.dead()==1); mixer.stopAll();
  playback.drain(attacked, [&](const std::array<double,3>& position) {
    ++positions; assert(position[0]==23 && position[1]==31);
    return audio::StereoGains{0.25f,1.0f};
  });
  assert(positions==1 && attacked.takeSoundEvents().empty());
  const auto hit=Access::render(mixer,1);
  assert(hit[0]==2500 && hit[1]==10000);
  sim::LevelSetup blasts; blasts.terrain=sim::Terrain(256,64); blasts.totalLemmings=0;
  sim::World exploded(std::move(blasts)); actor.state=sim::State::Exploder; actor.frame=100;
  exploded.place(actor); exploded.tick(); assert(exploded.dead()==1);
  assert(mixer.load("Explode",tone)); mixer.stopAll();
  playback.drain(exploded, [&](const std::array<double,3>& position) {
    assert(position[0]==23 && position[1]==exploded.lemmings()[0].y);
    return audio::StereoGains{1.0f,0.0f};
  });
  const auto blast=Access::render(mixer,1); assert(blast[0]==10000 && blast[1]==0);
  // Explicit UI and Let's Go never consume positional state from a prior cue.
  assert(mixer.load("Letsgo", tone)); assert(mixer.load("Changeop", tone));
  mixer.stopAll(); playback.play(sim::SoundCue::LevelStart);
  auto start=Access::render(mixer,1); assert(start[0]==10000 && start[1]==10000);
  mixer.stopAll(); playback.playUiClick();
  auto click=Access::render(mixer,1); assert(click[0]==10000 && click[1]==10000);
}

void flipperVoiceCapacity() {
  audio::CuePlayback playback;
  auto& mixer = Access::mixer(playback);
  assert(mixer.open());
  Access::pause(mixer);
  assert(mixer.load("First", Sound{22050, 1, {1000, 1000}}));
  assert(mixer.load("Regular", Sound{22050, 1, {200, 200}}));
  assert(mixer.load("AntiGrav", Sound{22050, 1, {50, 50}}));
  mixer.play("First");
  for (int i = 0; i < 31; ++i)
    mixer.play("Regular");

  sim::LevelSetup setup;
  setup.terrain = sim::Terrain(80, 60);
  setup.terrain.fillBox(0, 40, 80, 20, sim::Material::Earth);
  setup.terrain.setAntiGrav(50, 40, true);
  setup.totalLemmings = 0;
  sim::World world(std::move(setup));
  sim::Lemming walker;
  walker.x = 50;
  walker.y = 39;
  walker.state = sim::State::Walker;
  world.place(walker);
  world.tick();
  assert(world.lemmings().front().verticalDir == -1);
  playback.drain(world, [](const auto&) { return audio::StereoGains{0.5f, 1.0f}; });
  // The full pool and the positioned flipper clip both contribute.
  assert((Access::render(mixer, 1) == std::vector<std::int16_t>{7225, 7250}));
  mixer.play("Regular");
  // A capped request still evicts the oldest capped voice.
  assert((Access::render(mixer, 1) == std::vector<std::int16_t>{6425, 6450}));
}

// A refusal can touch several protected cells; emit one positioned one-shot,
// without changing the cut, movement or existing transition out of the skill.
void refusalSounds() {
  audio::CuePlayback playback;
  auto& mixer = Access::mixer(playback);
  assert(mixer.open()); Access::pause(mixer);
  Sound tone{22050, 1, std::vector<std::int16_t>(22050, 10000)};
  assert(mixer.load("Scrape", tone)); assert(mixer.load("MineMetal", tone));
  for (const auto state : {sim::State::Basher, sim::State::Miner, sim::State::Digger})
    for (const auto facing : {sim::Facing::Left, sim::Facing::Right})
      for (const bool directional : {false, true}) {
        if (directional && state != sim::State::Digger) continue;
        for (const int v : {1, -1}) {
          if (v == -1 && state != sim::State::Digger) continue;
          sim::LevelSetup setup;
          setup.terrain = sim::Terrain(128, 96);
          setup.totalLemmings = 0;
          // Multiple refusing cells, including the horizontal cylinder seam.
          const int x = facing == sim::Facing::Left ? 0 : 127;
          const int y = 40;
          for (int dx = -4; dx <= 4; ++dx)
            for (int dy = -4; dy <= 4; ++dy) {
              setup.terrain.set(x + dx, y + dy,
                  directional ? sim::Material::Earth : sim::Material::Steel);
              if (directional) {
                if (v > 0) setup.terrain.setNoDigUpright(x + dx, y + dy, true);
                else setup.terrain.setNoDigInverted(x + dx, y + dy, true);
              }
            }
          sim::World world(std::move(setup));
          sim::Lemming actor;
          actor.x = x; actor.y = y; actor.state = state;
          actor.facing = facing; actor.verticalDir = v;
          world.place(actor);
          const auto version = world.terrain().version();
          int requests = 0, positions = 0;
          playback.setObserver([&](std::string_view name) {
            assert(name == (state == sim::State::Miner ? "MineMetal" : "Scrape"));
            ++requests;
          });
          const auto position = [&](const std::array<double, 3>& p) {
            assert(p[0] == x && p[1] == y && p[2] == 0.75);
            ++positions;
            return audio::StereoGains{0.25f, 1.0f};
          };
          mixer.stopAll();
          for (int tick = 0; tick < 16 && world.lemmings()[0].state == state; ++tick) {
            world.tick();
            playback.drain(world, position);
          }
          const auto& stopped = world.lemmings()[0];
          assert(stopped.state == sim::State::Walker);
          assert(stopped.x == x && stopped.y == y && stopped.verticalDir == v);
          assert(stopped.facing == (state == sim::State::Digger ? facing :
              facing == sim::Facing::Left ? sim::Facing::Right : sim::Facing::Left));
          assert(world.terrain().version() == version);
          assert(requests == 1 && positions == 1);
          const auto pcm = Access::render(mixer, 1);
          assert(pcm[0] == 2500 && pcm[1] == 10000);
          playback.drain(world, position);
          world.tick(); playback.drain(world, position);
          assert(requests == 1 && positions == 1);
          assert(world.takeSoundEvents().empty());
        }
      }
}

void installed(const AssetLibrary& lib) {
  for (const char* name : {"MM_BLOCK_BUILD_BASH", "E20_MG_1", "MM_ALL_8_001", "MM_W_012",
                           "MM_E1_4SKILLS", "SHOW_KF3", "E10_KF_2"}) {
    auto report=sim::loadRetailLevelReport(lib,name); assert(report);
    for (const auto& c : report->setup.crabs) {
      std::cout << name << " crab attack='" << c.attackSound << "'\n";
      if (!c.attackSound.empty()) assert(lib.contains(soundAssetName(c.attackSound)));
    }
    for (const auto& h : report->setup.namedHazards) {
      std::cout << name << " hazard attack='" << h.attackSound << "'\n";
      if (!h.attackSound.empty()) {
        auto sound=loadSound(lib,soundAssetName(h.attackSound)); assert(sound && !sound->empty());
      }
    }
  }
  for (const char* name : {"Bubble","Die","Lever","SpeedPad"}) {
    auto sound=loadSound(lib,soundAssetName(name)); assert(sound && !sound->empty());
    std::cout << name << " decoded frames=" << sound->frameCount() << '\n';
  }
  for (const char* name : {"Scrape", "MineMetal"}) {
    const auto entry = soundAssetName(name);
    auto sound = loadSound(lib, entry); assert(sound && !sound->empty());
    std::cout << entry << " rate=" << sound->sampleRate << " channels=" << sound->channels
              << " frames=" << sound->frameCount() << " seconds=" << sound->seconds() << '\n';
  }
}

int main() {
  SDL_SetMainReady(); SDL_SetHintWithPriority(SDL_HINT_AUDIODRIVER,"dummy",SDL_HINT_OVERRIDE);
  projection(); pcm(); flipperVoiceCapacity(); refusalSounds();
  if (const char* directory=std::getenv("WEBOLUTION_GAME_DIR")) {
    const std::filesystem::path root(directory);
    auto lib=AssetLibrary::open(root/"lemmings.box",root/"patch.box"); assert(lib);
    installed(*lib);
  }
  std::cout << "positional sound projection, PCM, events and centred UI: PASS\n";
}
