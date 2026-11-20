// Check the exit balloon's installed mesh and drive the installed route through SDL input.
#include "app/AppEntry.h"
#include "app/BackdropScene.h"
#include "app/RuntimePick.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "assets/LevelIndex.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"
#include "lemmings/world_coordinates.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace webo;
namespace fs = std::filesystem;
namespace {
#ifdef EXIT_BALLOON_BEFORE
constexpr bool before = true;
#else
constexpr bool before = false;
#endif
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok && ++failures < 30) std::cerr << "FAIL: " << message << '\n';
}
std::string bytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct Snapshot {
  fs::path path;
  bool exists;
  std::string data;
  explicit Snapshot(fs::path p) : path(std::move(p)), exists(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return exists == fs::exists(path) && data == bytes(path); }
};
void push(SDL_Event e) { check(SDL_PushEvent(&e) == 1, "queue SDL input"); }
void key(SDL_Scancode code) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.scancode = code;
  e.key.keysym.sym = SDL_GetKeyFromScancode(code); push(e);
}
void click(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.button = SDL_BUTTON_LEFT;
  e.button.x = x; e.button.y = y; push(e); e.type = SDL_MOUSEBUTTONUP; push(e);
}
void quit() { SDL_Event e{}; e.type = SDL_QUIT; push(e); }
void png(const fs::path& p, const Image& image) { check(writePng(p, image).has_value(), "write " + p.string()); }
std::array<double,12> exitMatrix(const app::BackdropScene& scene) {
  std::array<double,12> result{};
  scene.visitRoots([&](const auto& source, const auto& node) {
    if (source.owner == "entities_426c6b") result = node.matrix;
  });
  return result;
}
std::string state(const app::LevelFrameView& f) {
  const auto& w = f.world;
  std::ostringstream s;
  s << std::setprecision(17) << f.frame << ',' << w.levelClockMs() << ',' << w.tickCount()
    << ',' << w.explicitUpdateCount() << ',' << w.spawned() << ',' << w.saved() << ','
    << w.dead() << ',' << w.finished() << ',' << w.won() << ',' << w.terrain().version()
    << ',' << int(f.endPhase) << ',' << f.endTimerSeconds << ',' << f.paused;
  for (int stock : w.setup().skills) s << ',' << stock;
  for (const auto& l : w.lemmings()) {
    s << ';' << l.x << ',' << l.y << ',' << int(l.state) << ',' << int(l.facing)
      << ',' << l.fallDistance << ',' << l.frame << ',' << l.bricksLeft << ',' << l.bashCycles
      << ',' << l.fuseMs << ',' << l.fasterMs << ',' << l.speedPadLatched
      << ',' << l.canClimb << ',' << l.canFloat << ',' << l.verticalDir << ',' << l.sheetType
      << ',' << l.stepBudgetSeconds;
    for (auto p : l.teleportPadsLatched) s << ':' << p;
  }
  return s.str();
}
// Mask blue balloon cloth, excluding the HUD, bottom scenery, cylinder seam and selected Climber icon.
std::array<double,3> blueBalloon(const Image& image) {
  double xSum = 0, ySum = 0, count = 0;
  for (int y = 80; y < 350; ++y) for (int x = 420; x < 640; ++x) {
    const auto i = (y * image.width + x) * 4;
    const auto r = image.rgba[i], g = image.rgba[i+1], b = image.rgba[i+2];
    if (b > 40 && b > 1.5*r && b > 1.25*g) { xSum += x; ySum += y; ++count; }
  }
  return count ? std::array{xSum/count, ySum/count, count} : std::array{0.0, 0.0, 0.0};
}
int run(const fs::path& out) {
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  const auto local = save::localAppDataDirectory(); if (!local) return 2;
  Snapshot installed(game / "SavedGame.dat"), normal(save::cloneSavePathUnder(*local));
  auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) return 2;
  const auto level = sim::resolveRetailLevel(*lib, "MM_CLIMB_002");
  if (!level) return 2;
  const auto blob = lib->read(level->file); if (!blob) return 2;
  const auto compiled = lemmings::read_compiled_level({reinterpret_cast<const uint8_t*>(blob->data()), blob->size()});
  auto scene = app::BackdropScene::build(*lib, *blob, "MM_CLIMB_002");
  check(bool(scene), "installed scene builds"); if (!scene) return 1;
  auto presentation = scene->entranceExitPresentation(*blob);
  check(bool(presentation), "installed exit binds"); if (!presentation) return 1;
  const auto initial = exitMatrix(*scene);
  const auto& entity = compiled.gadgets.entities_426c6b.at(0);
  std::array<double,3> cylinder{};
  for (int i = 0; i < 3; ++i) cylinder[i] = std::bit_cast<double>(entity.double_bits[i]);
  // The file alternates the two radial layers; 0x43D09A separates them
  // into the runtime's two 32-point arrays (also used by ActorPose.cpp).
  auto rows = compiled.world.first_blocks;
  for (std::size_t r=0;r<rows.size();++r) for (int x=0;x<32;++x) for(int layer=0;layer<2;++layer)
    rows[r][layer*32+x] = compiled.world.first_blocks[r][x*2+layer];
  const lemmings::WorldCoordinateMesh mesh{int(compiled.world.leading_value),rows};
  const auto start = lemmings::map_world_coordinates(mesh, cylinder);
  for (int i = 0; i < 3; ++i)
    check(std::abs(start[i] - initial[9+i]) < 1e-6, "stored translation agrees with recovered map: " + std::to_string(start[i]) + " vs " + std::to_string(initial[9+i]));
  std::ofstream path(out / "model-path.csv");
  path << std::setprecision(17) << "seconds,cylinder_x,cylinder_y,radial_blend,world_x,world_y,world_z,radius\n";
  for (int k = 0; k <= 12; ++k) {
    const double t = k * 0.5;
    scene->setEntranceExitPresentation(*presentation, 1, t);
    const auto actual = exitMatrix(*scene);
    auto moved = cylinder; moved[0] += t*0.5; moved[1] += t*5; moved[2] += t*0.5;
    const auto expected = lemmings::map_world_coordinates(mesh, moved);
    path << t << ',' << moved[0] << ',' << moved[1] << ',' << moved[2] << ','
         << actual[9] << ',' << actual[10] << ',' << actual[11] << ',' << std::hypot(actual[9],actual[11]) << '\n';
    if (before) {
      check(actual[9] == initial[9] && actual[11] == initial[11], "authentic baseline is purely vertical");
      check(std::abs(actual[10]-initial[10]-5*t) < 1e-8, "baseline vertical rate");
    } else {
      for (int i = 0; i < 3; ++i)
        check(std::abs(actual[9+i]-expected[i]) < 1e-6, "exit follows recovered coordinate mapping");
      if (k) {
        check(std::hypot(actual[9]-initial[9],actual[11]-initial[11]) > t*5, "departure is substantially nonvertical");
        check(std::hypot(actual[9],actual[11]) > std::hypot(initial[9],initial[11]), "radius increases");
        check(actual[10] > initial[10], "balloon also rises");
        check(std::atan2(actual[11],actual[9]) != std::atan2(initial[11],initial[9]), "angle changes around cylinder");
      }
      scene->setEntranceExitPresentation(*presentation, 1, t);
      check(exitMatrix(*scene) == actual, "absolute samples do not drift");
    }
  }
  scene->setEntranceExitPresentation(*presentation, 1, 0);
  check(exitMatrix(*scene) == initial, "zero departure resets the stored pose");

  // Use the original's right-edge perch with a stationary camera.
  // The comparison uses an approximate camera pose.
  const double tau = 2*3.1415927410125732;
  const double yaw = std::fmod((std::atan2(initial[11],initial[9])+0.4)/tau*256+128+256,256);
  const auto cameraFile = out / "camera.json";
  std::ofstream(cameraFile) << "{\"viewport\":[640,480],\"yaw_units\":32}";
  std::ofstream trace(out/"simulation.txt"), frames(out/"frames.csv");
  frames << "frame,pending_seconds,phase,saved,yaw,blue_x,blue_y,blue_pixels\n";
  int pending = 0, pauses = 0, rotationStage = 0;
  bool success = false, pauseRequested = false;
  double firstX = 0, firstY = 0, firstPixels = 0, xAtTwo = 0, yAtTwo = 0;
  double blueAtFour = -1, blueAtFive = -1, lastTime = -1;
  Image frozen;
  auto front = [&](const app::FrontendSession& s, int) {
    if (s.screen() == shell::Screen::Menu || s.screen() == shell::Screen::Confirm) key(SDL_SCANCODE_RETURN);
    else if (s.screen() == shell::Screen::Map) click(206,204);
    else if (s.screen() == shell::Screen::Results) {
      check(pending > 20 && lastTime >= 5.8, "results follow the full existing departure interval");
      check(s.outcome() && s.outcome()->success && s.outcome()->saved == 10, "natural production route wins 10/10");
      success = true; png(out/"results.png",s.composeFrame()); quit();
    }
  };
  auto observer = [&](const app::LevelFrameView& f) {
    const auto line = state(f); trace << line << '\n';
    for (const auto& a : f.actors) {
      const auto& l = f.world.lemmings()[a.index];
      if (!l.active() || l.canClimb || !app::firstLevelClimberSupport(l,sim::Skill::Climber,f.world.terrain()).value_or(false)) continue;
      const int x = int((a.placement.left+a.placement.right)/2), y = int((a.placement.top+a.placement.bottom)/2);
      if (x >= 90 && x < 640 && y >= 0 && y < 480) { click(x,y); break; }
    }
    // Assign all ten Climbers in the established gameplay view, then use
    // public right-drag input to expose the same right-edge perch as the reference.
    if (rotationStage == 0 && f.world.spawned() == 10 &&
        std::all_of(f.world.lemmings().begin(),f.world.lemmings().end(),[](const auto& l){return l.canClimb;})) {
      SDL_Event e{}; e.type=SDL_MOUSEBUTTONDOWN; e.button.button=SDL_BUTTON_RIGHT;
      e.button.x=620; e.button.y=460; push(e);
      e={}; e.type=SDL_MOUSEMOTION; e.motion.x=620; e.motion.y=460;
      e.motion.xrel=int(std::lround(-std::remainder(yaw-f.displayedPose.yawUnits,256.0)*2.4)); push(e);
      rotationStage=1;
    } else if (rotationStage == 1) {
      SDL_Event e{}; e.type=SDL_MOUSEBUTTONUP; e.button.button=SDL_BUTTON_RIGHT;
      e.button.x=620; e.button.y=460; push(e); rotationStage=2;
      png(out/"settled-camera.png",f.image);
    }
    if (f.endPhase != app::LevelEndPhase::Pending) return;
    ++pending;
    const double t = 7-f.endTimerSeconds;
    const auto blue = blueBalloon(f.image);
    frames << std::setprecision(17) << f.frame << ',' << t << ',' << int(f.endPhase) << ',' << f.world.saved()
           << ',' << f.displayedPose.yawUnits << ',' << blue[0] << ',' << blue[1] << ',' << blue[2] << '\n';
    check(rotationStage==2 && std::abs(std::remainder(f.displayedPose.yawUnits-yaw,256.0)) < 0.21, "camera stays still at requested view during departure");
    if (pending == 1) { firstX=blue[0]; firstY=blue[1]; firstPixels=blue[2]; check(blue[2]>200,"balloon visible at perch"); }
    if (std::abs(t-2)<1e-6) { xAtTwo=blue[0]; yAtTwo=blue[1]; }
    if (std::abs(t-4.5)<0.051) blueAtFour=blue[2];
    if (std::abs(t-5.5)<0.051) blueAtFive=blue[2];
    if (!pauseRequested && t >= 0.5) { key(SDL_SCANCODE_P); pauseRequested=true; }
    if (f.paused) {
      if (pauses++ == 0) frozen=f.image;
      // The selected panel icon animates on UI time; compare the scene to its right.
      for (int y=0;y<480;++y) for(int x=90;x<640;++x) {
        const auto i=(y*640+x)*4;
        if (!std::equal(frozen.rgba.begin()+i,frozen.rgba.begin()+i+4,f.image.rgba.begin()+i)) {
          check(false,"pause freezes the complete scene"); y=480; break;
        }
      }
      check(t == lastTime,"pause freezes departure clock");
      if (pauses == 3) key(SDL_SCANCODE_P);
    }
    if (!f.paused && (pending == 1 || std::abs(t*2-std::round(t*2)) < 1e-6))
      png(out/("pending-"+std::to_string(int(std::round(t*1000)))+".png"),f.image);
    lastTime=t;
  };
  std::vector<std::string> args{"exit-balloon", "--frontend", "--native", "--frames", "2200",
      "--game-dir", game.string(), "--camera", cameraFile.string()};
  std::vector<char*> raw; for(auto& arg:args) raw.push_back(arg.data());
  // Use an isolated clone save.
  const auto saveFile=out/"SavedGame.dat";
  const auto data=save::SavedGame{}.serialize();
  {std::ofstream file(saveFile,std::ios::binary);file.write(reinterpret_cast<const char*>(data.data()),data.size());}
  app::ApplicationRunOptions options; options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(saveFile,game));
  check(app::runApplication(int(raw.size()),raw.data(),front,observer,options)==0,"production application exits");
  check(success && pauses==3,"win, results and pending pause completed");
  if (!before) {
    // Check the departure path in world space and use pixels to distinguish it from vertical travel.
    check(xAtTwo-firstX > 20,"production balloon travels sideways in reference-like view");
    check(std::abs(yAtTwo-firstY) < 60,"rise remains bounded while sideways motion is visible");
    check(blueAtFour >= 0 && blueAtFour < firstPixels*0.25,"most balloon cloth has left the view at 4.5 seconds");
    check(blueAtFive == 0,"balloon leaves the view before results");
  }
  check(installed.unchanged() && normal.unchanged(),"installed and normal saves untouched");
  std::ofstream(out/"summary.txt") << std::setprecision(17)
      << "before=" << before << "\nsuccess=" << success << "\npending_frames=" << pending
      << "\npaused_frames=" << pauses << "\nlast_departure_seconds=" << lastTime
      << "\ninitial_blue_x=" << firstX << "\ninitial_blue_y=" << firstY
      << "\nblue_x_at_two=" << xAtTwo << "\nblue_y_at_two=" << yAtTwo
      << "\nblue_pixels_at_four_point_five=" << blueAtFour
      << "\nblue_pixels_at_five_point_five=" << blueAtFive
      << "\ninstalled_save_unchanged=" << installed.unchanged()
      << "\nnormal_save_unchanged=" << normal.unchanged() << "\nfailures=" << failures << '\n';
  return failures ? 1 : 0;
}
int assets(const fs::path& out) {
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box"); if(!lib)return 2;
  int levels=0,exits=0;
  std::ofstream census(out/"installed-paths.csv");
  census << "level,exit,seconds,x,y,z,radius\n" << std::setprecision(17);
  for(const auto& file:levelFileNames(*lib)) {
    const auto blob=lib->read(file); if(!blob)return 2;
    const auto level=lemmings::read_compiled_level({reinterpret_cast<const uint8_t*>(blob->data()),blob->size()});
    auto scene=app::BackdropScene::build(*lib,*blob,file);
    check(bool(scene),"installed scene "+file); if(!scene)continue;
    const auto presentation=scene->entranceExitPresentation(*blob);
    check(bool(presentation),"installed presentation "+file); if(!presentation)continue;
    check(presentation->exits.size()==level.gadgets.entities_426c6b.size(),"every installed exit bound");
    ++levels; exits+=int(presentation->exits.size());
    // Independent installed root-position check catches wrong FILE/RUNTIME
    // row layout. It supplements the real win; it is not a gameplay sweep.
    const lemmings::WorldCoordinateMesh mesh{presentation->exitCoordinateHeight,presentation->exitCoordinateRows};
    for(const auto& exit:presentation->exits) {
      const auto start=lemmings::map_world_coordinates(mesh,exit.cylinderPosition);
      for(int j=0;j<3;++j)check(std::abs(start[j]-exit.matrix[9+j])<1e-6,"installed stored pose matches mapped coordinate");
    }
    for(double t:{0.0,0.5,3.0,6.0}) {
      scene->setEntranceExitPresentation(*presentation,1,t);
      scene->visitRoots([&](const auto& source,const auto& node){
        if(source.owner!="entities_426c6b")return;
        const auto& m=node.matrix;
        census << file << ',' << source.entityIndex << ',' << t << ',' << m[9] << ',' << m[10] << ',' << m[11] << ',' << std::hypot(m[9],m[11]) << '\n';
        check(std::all_of(m.begin(),m.end(),[](double v){return std::isfinite(v);}),"installed path remains finite");
      });
    }
  }
  check(levels==102 && exits==111,"all installed level/exit data measured");
  std::ofstream(out/"summary.txt") << "levels=" << levels << "\nexits=" << exits << "\nfailures=" << failures << '\n';
  return failures?1:0;
}
}
int main(int argc,char** argv) {
  if((argc!=2 && argc!=3) || !std::getenv("WEBOLUTION_GAME_DIR") || !std::getenv("SDL_VIDEODRIVER") || std::string(std::getenv("SDL_VIDEODRIVER"))!="dummy") return 2;
  fs::create_directories(argv[1]);
  if(argc==3 && std::string(argv[2])=="assets")return assets(argv[1]);
  return run(argv[1]);
}
