// Check overlapping actor selection on an installed level through SDL hover and click.
#include "app/AppEntry.h"
#include "app/RuntimePick.h"
#include "assets/Png.h"
#include "sim/World.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures=0;
void check(bool ok,const std::string& why) { if(!ok) {++failures;std::cerr<<"FAIL: "<<why<<'\n';} }
std::string bytes(const fs::path& p) {std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct Preserved {
  fs::path path; bool exists; std::string data;
  explicit Preserved(fs::path p):path(std::move(p)),exists(fs::exists(path)),data(bytes(path)){}
  bool unchanged() const {return exists==fs::exists(path)&&data==bytes(path);}
};
void key(SDL_Scancode code) {SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.scancode=code;check(SDL_PushEvent(&e)==1,"queue key");}
void point(int x,int y,bool click=false) {SDL_Event e{};e.type=click?SDL_MOUSEBUTTONDOWN:SDL_MOUSEMOTION;if(click){e.button.button=SDL_BUTTON_LEFT;e.button.x=x;e.button.y=y;}else{e.motion.x=x;e.motion.y=y;}check(SDL_PushEvent(&e)==1,"queue pointer");}
void quit(){SDL_Event e{};e.type=SDL_QUIT;check(SDL_PushEvent(&e)==1,"queue quit");}
app::PickBox box(const app::ActorSpritePlacement& p){return {int(p.left),int(p.top),int(p.right),int(p.bottom)};}
double distance(const app::ActorSpritePlacement& p,int x,int y){const double dx=(p.left+p.right)/2-x,dy=(p.top+p.bottom)/2-y;return dx*dx+dy*dy;}
// Compute rectangle and skill eligibility independently of the selector.
bool eligible(const app::LevelFrameView& f,const app::LevelActorView& a,int x,int y,bool inverted){
  const auto& l=f.world.lemmings()[a.index]; const auto b=box(a.placement);
  if(b.left>x+12||b.right<x-12||b.top>y+20||b.bottom<y-20||!l.active())return false;
  if(!inverted){
    if(l.canClimb||l.state==sim::State::Climber||l.state==sim::State::Hoister||l.state==sim::State::Faller||l.state==sim::State::Bounce)return false;
    if(l.state==sim::State::Walker&&l.verticalDir==1){const auto&t=f.world.terrain();const int row=std::clamp(l.y+1,0,t.height()-1);if(!t.solid(l.x,row)||t.trampoline(l.x,row))return false;}
  }
  return true;
}
struct Choice{std::size_t nearest=0,native=0;int count=0;double minX=INFINITY,maxX=-INFINITY;bool left=false,right=false,overlap=false;};
Choice choice(const app::LevelFrameView& f,int x,int y,bool inverted){
  Choice c;double nearest=INFINITY,native=INFINITY;int priority=-1;
  std::vector<app::PickBox> boxes;
  for(const auto&a:f.actors){if(!eligible(f,a,x,y,inverted))continue;
    const auto&l=f.world.lemmings()[a.index];++c.count;
    const double cx=(a.placement.left+a.placement.right)/2;
    c.minX=std::min(c.minX,cx);c.maxX=std::max(c.maxX,cx);
    c.left|=l.facing==sim::Facing::Left;c.right|=l.facing==sim::Facing::Right;
    for(const auto&b:boxes){const auto p=box(a.placement);c.overlap|=p.left<b.right&&p.right>b.left&&p.top<b.bottom&&p.bottom>b.top;}boxes.push_back(box(a.placement));
    const double d=distance(a.placement,x,y);if(d<nearest){nearest=d;c.nearest=a.index;}
    double raw=0;for(int k=0;k<3;++k)raw+=(a.drawnSample[k]-f.cameraPickReference[k])*(a.drawnSample[k]-f.cameraPickReference[k]);
    const int p=app::knownPickPriority(l.state).value_or(0);if(p>priority||(p==priority&&raw<native)){priority=p;native=raw;c.native=a.index;}
  }return c;
}
void shot(const fs::path& p,const app::LevelFrameView& f){
  check(writePng(p,f.image).has_value(),"retain clone PNG");
  std::ofstream raw(p.string()+".rgba",std::ios::binary);
  raw.write(reinterpret_cast<const char*>(f.image.rgba.data()),f.image.rgba.size());
}
void units(){
  using app::ScreenPickCandidate;
  int a=0,b=1,c=2;
  ScreenPickCandidate candidates[]={{&a,{90,90,110,110},true,{99.75,100}},
    {&b,{90,90,110,110},true,{100.25,100}},
    {&c,{90,90,110,110},false,{100,100}}};
  check(app::pickNearestScreenLemming(candidates,100,100)->id==&a,"exact distance tie keeps first World index, ineligible centre skipped");
  std::swap(candidates[0],candidates[1]);
  check(app::pickNearestScreenLemming(candidates,100,100)->id==&b,"tie follows supplied stable World order");
  check(app::pickNearestScreenLemming(candidates,99,100)->id==&a,"fractional centre left");
  check(app::pickNearestScreenLemming(candidates,101,100)->id==&b,"fractional centre right");
  candidates[0].bodyCentre={100,108};candidates[1].bodyCentre={102,100};
  check(app::pickNearestScreenLemming(candidates,100,100)->id==&a,"2D distance includes Y instead of horizontal-only distance");
  candidates[0].box={0,0,1,1};candidates[0].bodyCentre={100,100};
  check(app::pickNearestScreenLemming(candidates,100,100)->id==&a,"near centre cannot bypass unchanged hit radius");
  check(!app::pickNearestScreenLemming(candidates,400,400),"all candidates outside hit radius");
  check(!app::pickNearestScreenLemming({},100,100),"empty candidate list");
  for(const auto box: {app::PickBox{112,100,113,101},app::PickBox{87,100,88,101},app::PickBox{100,120,101,121},app::PickBox{100,79,101,80}}){
    ScreenPickCandidate edge{&a,box,true,{100,100}};
    check(app::pickNearestScreenLemming(std::span(&edge,1),100,100)!=nullptr,"inclusive native radius edge retained");
  }
}
int route(const std::string& name,const fs::path& out){
  fs::create_directories(out);const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  const bool inverted=name.starts_with("inverted");const int skill=inverted?3:0;
  auto local=save::localAppDataDirectory();if(!local)return 2;
  Preserved normal(save::cloneSavePathUnder(*local)),installed(game/"SavedGame.dat"),privateSave(out/"SavedGame.dat");
  app::ApplicationRunOptions options;options.frameSeconds=0.02;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  std::vector<std::string> args{"pick-nearest","--native","--sim-ticks",inverted?"45":name.ends_with("overlap")?"98":"100","--level",inverted?"m06_kf_1":"MM_CLIMB_002","--frames","8","--no-audio","--game-dir",game.string()};
  std::vector<char*> argv;for(auto&a:args)argv.push_back(a.data());
  int x=0,y=0,stock=0;Choice chosen;std::size_t highlighted=0;int calls=0;bool found=false;
  std::ofstream trace(out/"selection.csv");trace<<std::setprecision(17)<<"frame,pointer_x,pointer_y,actor,centre_x,centre_y,distance2,facing,gravity,state,eligible,hovered,marked,left,top,right,bottom\n";
  const auto observe=[&](const app::LevelFrameView& f){++calls;
    if(f.frame==0){key(SDL_SCANCODE_P);if(inverted)point(50,148,true);}
    if(f.frame==1){
      check(f.paused&&f.selectedSkillIndex==skill,"pause and select real stocked job");stock=f.world.setup().skills[skill];
      // Prefer a point where the two selection policies disagree.
      double bestScore=INFINITY;
      for(int mismatch=1;mismatch>=0&&!found;--mismatch)
      for(int yy=40;yy<420;++yy)for(int xx=100;xx<630;++xx){
        auto c=choice(f,xx,yy,inverted);if(c.count<(inverted?1:2))continue;
        if(!inverted&&(!c.left||!c.right))continue;
        if(name.ends_with("overlap")&&!c.overlap)continue;
        const auto&l=f.world.lemmings()[c.nearest];if(l.state!=sim::State::Walker||l.verticalDir!=(inverted?-1:1))continue;
        if(inverted&&!f.world.terrain().solid(l.x,l.y-1))continue;
        const bool position=name.ends_with("overlap")?true:name.ends_with("left")?xx<c.minX:name.ends_with("right")?xx>c.maxX:
          inverted?std::abs(xx-(c.minX+c.maxX)/2)<=0.5:xx>c.minX&&xx<c.maxX;
        if(!position||(mismatch&&c.native==c.nearest))continue;
        const auto actor=std::find_if(f.actors.begin(),f.actors.end(),[&](const auto&a){return a.index==c.nearest;});
        const double score=distance(actor->placement,xx,yy);if(score>=bestScore)continue;bestScore=score;
        x=xx;y=yy;chosen=c;found=true;
      }
      check(found,"real installed overlapping crowd has requested cursor position: "+name);
      if(!found){quit();return;}point(x,y);
    }
    for(const auto&a:f.actors){const auto&l=f.world.lemmings()[a.index];const auto&p=a.placement;trace<<f.frame<<','<<f.cursorX<<','<<f.cursorY<<','<<a.index<<','<<(p.left+p.right)/2<<','<<(p.top+p.bottom)/2<<','<<distance(p,f.cursorX,f.cursorY)<<','<<int(l.facing)<<','<<l.verticalDir<<','<<sim::stateName(l.state)<<','<<eligible(f,a,f.cursorX,f.cursorY,inverted)<<','<<(f.hoveredActor==a.index)<<','<<(f.markedActor==a.index)<<','<<p.left<<','<<p.top<<','<<p.right<<','<<p.bottom<<'\n';}
    if(f.frame==2){
      check(f.paused&&f.hoveredActor.has_value(),"public pointer has hover while paused");
      if(!f.hoveredActor){quit();return;}highlighted=*f.hoveredActor;
      check(f.markedActor==highlighted,"feedback brightens exactly hovered actor");
#ifdef PICK_NEAREST_BEFORE
      check(highlighted==chosen.native,"authentic before main follows retained native terrain priority");
#else
      check(highlighted==chosen.nearest,"public hover selects nearest displayed body centre");
#endif
      shot(out/"hover.png",f);point(x,y,true);
    }
    if(f.frame==3){
      const auto&l=f.world.lemmings()[highlighted];
      check(inverted?l.state==sim::State::Blocker:l.canClimb,"highlighted actor receives skill on same-pointer click");
      check(f.world.setup().skills[skill]==stock-1,"one click spends exactly one job");
      check(std::count_if(f.world.lemmings().begin(),f.world.lemmings().end(),[&](const auto&l){return inverted?l.state==sim::State::Blocker:l.canClimb;})==1,"no other actor receives job");
      shot(out/"assigned.png",f);quit();
    }
  };
  check(app::runApplication(int(argv.size()),argv.data(),{},observe,options)==0,"production application exits");
  check(found&&calls==4,"all four public-input route stages observed");
  check(normal.unchanged()&&installed.unchanged()&&privateSave.unchanged(),"normal installed and private saves unchanged");
  std::ofstream(out/"summary.txt")<<"route="<<name<<"\ncursor="<<x<<','<<y<<"\nnearest="<<chosen.nearest<<"\nnative="<<chosen.native<<"\nactual="<<highlighted<<"\ncandidates="<<chosen.count<<"\noverlapping="<<chosen.overlap<<"\nleft_facing="<<chosen.left<<"\nright_facing="<<chosen.right<<"\nsaves_unchanged="<<(normal.unchanged()&&installed.unchanged()&&privateSave.unchanged())<<"\nfailures="<<failures<<'\n';
  return failures?1:0;
}
}
int main(int argc,char**argv){if(argc==2&&std::string(argv[1])=="units"){units();return failures?1:0;}
  if(argc!=3||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  return route(argv[1],argv[2]);}
