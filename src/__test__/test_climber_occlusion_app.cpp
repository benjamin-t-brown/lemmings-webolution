// Public SDL route through runApplication. No simulation mutation or original process.
#include "app/AppEntry.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace webo;
namespace fs=std::filesystem;
namespace {
int failures=0;
void check(bool b,const std::string& m){if(!b){++failures;if(failures<25)std::cerr<<"FAIL: "<<m<<'\n';}}
std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct Saved {fs::path p;bool exists;std::string data;Saved(fs::path v):p(v),exists(fs::exists(p)),data(bytes(p)){}bool unchanged()const{return exists==fs::exists(p)&&data==bytes(p);}};
void png(const fs::path& p,const Image& im){check(bool(writePng(p,im)),"retain "+p.string());}
void push(SDL_Event e){check(SDL_PushEvent(&e)==1,"public SDL input queued");}
void key(SDL_Scancode c){SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.scancode=c;push(e);}
void motion(int x,int y,int dx=0){SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=x;e.motion.y=y;e.motion.xrel=dx;push(e);}
void button(bool down,Uint8 b,int x,int y){SDL_Event e{};e.type=down?SDL_MOUSEBUTTONDOWN:SDL_MOUSEBUTTONUP;e.button.button=b;e.button.x=x;e.button.y=y;push(e);}
void right(bool down){button(down,SDL_BUTTON_RIGHT,620,460);}
void click(int x,int y){button(true,SDL_BUTTON_LEFT,x,y);button(false,SDL_BUTTON_LEFT,x,y);}
void quit(){SDL_Event e{};e.type=SDL_QUIT;push(e);}
std::string state(const app::LevelFrameView& f){std::ostringstream s;s<<std::setprecision(17)<<f.frame<<' '<<f.world.levelClockMs()<<' '<<f.paused<<' '<<f.displayedPose.yawUnits<<' '<<f.world.setup().skills[0];for(const auto& l:f.world.lemmings())s<<' '<<l.x<<','<<l.y<<','<<int(l.state)<<','<<int(l.facing)<<','<<l.canClimb;return s.str();}
void application(const AssetLibrary& lib,const fs::path& game,const std::string& level,
    const fs::path& out,const fs::path& baseline,bool before){
  fs::create_directories(out);const bool paired=!baseline.empty();
  const double initial=level=="MM_CLIMB_002"?32.0:200.0;
  std::ofstream(out/"camera.json")<<"{\"yaw_units\":"<<initial<<"}\n";
  auto resolved=sim::resolveRetailLevel(lib,level);check(bool(resolved),"resolve app level");if(!resolved)return;
  auto blob=lib.read(resolved->file);auto mesh=findCylinderMesh(*blob);auto idx=indexLevel(*blob);
  auto scene=app::TubeScene::build(lib,*mesh,*blob,idx->grid0);check(bool(scene),"raw box geometry");if(!scene)return;
  app::ApplicationRunOptions options;options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  Saved privateSave(out/"SavedGame.dat");
  std::vector<std::string> args{"climber-occlusion-app","--level",level,"--camera",(out/"camera.json").string(),"--sim","--native","--frames","250","--no-audio","--game-dir",game.string()};
  std::vector<char*> argv;for(auto& a:args)argv.push_back(a.data());
  std::ofstream simTrace(out/"simulation.txt",std::ios::binary),trace(out/"rotation.csv");std::ifstream prior(baseline/"simulation.txt");
  trace<<std::setprecision(17)<<"view,frame,ms,yaw_units,yaw_degrees,box,hover,raw_left,raw_top,raw_right,raw_bottom\n";
  bool clicked=false,pauseRequested=false,grabbed=false;int climbStart=-1,step=-1,query=0;
  int frames=0,noClimberFrames=0,rotations=0,hiddenChecks=0,frontChecks=0;double startYaw=0;
  std::uint64_t frozenMs=0;std::optional<app::LevelActorView> frozen;
  const auto observer=[&](const app::LevelFrameView& f){
    ++frames;const auto serial=state(f);simTrace<<serial<<'\n';if(paired){std::string wanted;check(bool(std::getline(prior,wanted))&&wanted==serial,"identical per-frame world/clock/camera/stock");}
    const bool noClimber=std::none_of(f.world.lemmings().begin(),f.world.lemmings().end(),[](const auto& l){return l.alive()&&l.state==sim::State::Climber;});
    if(noClimber){++noClimberFrames;const auto path="no-climber-"+std::to_string(f.frame)+".png";png(out/path,f.image);if(paired)check(bytes(out/path)==bytes(baseline/path),"full application frame with no climber is PNG-byte-identical");}
    if(f.frame==0){key(SDL_SCANCODE_1);motion(620,460);}
    if(f.world.lemmings().empty())return;
    const auto& l=f.world.lemmings()[0];
    const auto actor=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& a){return a.index==0;});
    if(!clicked&&l.state==sim::State::Walker&&actor!=f.actors.end()){
      int x=int((actor->placement.left+actor->placement.right)/2),y=int((actor->placement.top+actor->placement.bottom)/2);
      if(x>=100&&x<640&&y>=0&&y<480){motion(x,y);if(f.hoveredActor==0){click(x,y);clicked=true;motion(620,460);}}
    }
    if(l.state==sim::State::Climber&&climbStart<0)climbStart=l.y;
    if(clicked&&l.state==sim::State::Climber&&!pauseRequested&&l.y<climbStart-4){pauseRequested=true;key(SDL_SCANCODE_P);}
    if(!f.paused)return;
    if(!frozen){if(level=="MM_FLOAT_CLIMB2")key(SDL_SCANCODE_2);check(actor!=f.actors.end(),"front-facing paused contact sprite visible");if(actor==f.actors.end())return;frozen=*actor;frozenMs=f.world.levelClockMs();startYaw=frozen->drawnSample[0];}
    check(f.world.levelClockMs()==frozenMs&&l.state==sim::State::Climber&&l.canClimb,"public pause freezes naturally assigned climber");
    app::SceneOptions raw;raw.drawCellRoute=true;raw.drawWall=false;raw.composeSkipActorVisibility=true;
    app::SceneOptions::InjectedNativeWalker w;w.plus8X=frozen->drawnSample[0];w.plus8Y=frozen->drawnSample[1];w.plus8Z=frozen->drawnSample[2];w.clip=frozen->clip;w.walkTemporalIndex=frozen->temporalIndex;w.facing=int(l.facing);w.verticalDir=l.verticalDir;raw.injectedWalkers={w};
    const auto rawBox=scene->injectedWalkerScreenBoxes(app::buildCamera(f.displayedPose),raw,lib)[0];check(bool(rawBox),"raw projected paused actor");if(!rawBox)return;
    const int px=int((rawBox->left+rawBox->right)/2),py=int((rawBox->top+rawBox->bottom)/2);
    if(query){
      if(query==1){query=2;return;}
      if(step==4){
        check(before?(actor!=f.actors.end()):(actor==f.actors.end()&&f.hoveredActor!=0),"public displayed/pick-box list obeys covered climber visibility");
        if(level=="MM_FLOAT_CLIMB2")check(before?(f.hoveredActor==0):(f.hoveredActor!=0),"public Floater hover follows covered climber visibility");
        ++hiddenChecks;
      }else {
        check(actor!=f.actors.end(),"public exposed climber has displayed pick box");
        if(level=="MM_FLOAT_CLIMB2")check(f.hoveredActor==0,"public Floater hover selects exposed climber");
        // Just Climb has only Climber stock: the accepted picker intentionally
        // excludes an actor which already has that permanent capability.
        else check(f.hoveredActor!=0,"public Climber selection preserves existing capability exclusion");
        ++frontChecks;
      }
      png(out/("hover-view-"+std::to_string(step)+".png"),f.image);query=0;right(true);grabbed=true;++step;
    }else if(step>=0){
      const double target=startYaw+step*256.0/48.0;
      check(std::abs(std::remainder(f.displayedPose.yawUnits-target,256.0))<=0.21,"public integer mouse rotation within half a mouse count of requested evenly spaced angle");
      png(out/("view-"+std::to_string(step)+".png"),f.image);
      trace<<step<<','<<f.frame<<','<<f.world.levelClockMs()<<','<<f.displayedPose.yawUnits<<','<<f.displayedPose.yawUnits*360/256<<','<<(actor!=f.actors.end())<<','<<(f.hoveredActor?int(*f.hoveredActor):-1)<<','<<rawBox->left<<','<<rawBox->top<<','<<rawBox->right<<','<<rawBox->bottom<<'\n';
      ++rotations;
      if(step==4||step==40){right(false);grabbed=false;motion(px,py);query=1;return;}
      ++step;
    }else step=0;
    if(step==48){if(grabbed)right(false);quit();return;}
    if(!grabbed){right(true);grabbed=true;}
    const double target=startYaw+step*256.0/48.0;
    motion(620,460,int(std::lround(-std::remainder(target-f.displayedPose.yawUnits,256.0)*2.4)));
  };
  check(app::runApplication(int(argv.size()),argv.data(),{},observer,options)==0,"source-built production application exits normally");
  check(clicked&&pauseRequested&&rotations==48&&hiddenChecks==1&&frontChecks==1,"public skill, climb, full rotation and hidden/front hover route completes");
  check(noClimberFrames>0,"application no-climber frames exercised");check(privateSave.unchanged(),"private direct-level save untouched");
  if(paired){std::string extra;check(!std::getline(prior,extra),"paired application frame counts identical");}
  std::ofstream(out/"summary.txt")<<"frames="<<frames<<"\nno_climber_frames="<<noClimberFrames<<"\nrotation_views="<<rotations<<"\nhidden_hover_checks="<<hiddenChecks<<"\nfront_hover_checks="<<frontChecks<<"\nprivate_save_unchanged="<<privateSave.unchanged()<<"\nfailures="<<failures<<'\n';
}
}
int main(int argc,char** argv){
  if(argc!=4||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  const bool before=std::string(argv[1])=="before";const fs::path out=argv[2],baseline=std::string(argv[3])=="-"?fs::path{}:fs::path(argv[3]),game=std::getenv("WEBOLUTION_GAME_DIR");
  auto local=save::localAppDataDirectory();if(!local)return 2;Saved installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  for(const std::string id:{"MM_CLIMB_002","MM_FLOAT_CLIMB2"})application(*lib,game,id,out/id,baseline.empty()?fs::path{}:baseline/id,before);
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves untouched");
  std::ofstream(out/"summary.txt")<<"installed_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  std::cout<<"climber application "<<argv[1]<<": "<<failures<<" failures\n";return failures?1:0;
}
