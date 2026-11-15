// U4/U2: installed assets, real level render fixtures and public application routes.
// Source-built clone evidence only. Never executes original instructions.
#include "app/AppEntry.h"
#include "app/LemmingAnim.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool b, const std::string& s) { if (!b) { ++failures; std::cerr << "FAIL: " << s << '\n'; } }
std::string bytes(const fs::path& p) { std::ifstream f(p, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}}; }
struct Saved {
  fs::path p; bool exists; std::string data;
  Saved(fs::path v): p(v), exists(fs::exists(p)), data(bytes(p)) {}
  bool unchanged() const { return exists == fs::exists(p) && data == bytes(p); }
};
void png(const fs::path& p, const Image& i) { check(bool(writePng(p, i)), "write " + p.string()); }
Image blank(int w, int h) { Image i; i.width=w; i.height=h; i.rgba.assign(std::size_t(w)*h*4,255); return i; }
void cropInto(Image& dst, const Image& src, int cx, int cy, int col, int row, int w=64, int h=80, int scale=2) {
  for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
    int sx=cx-w/2+x,sy=cy-h/2+y;
    if(sx<0||sy<0||sx>=src.width||sy>=src.height)continue;
    for(int dy=0;dy<scale;++dy)for(int dx=0;dx<scale;++dx)for(int c=0;c<4;++c)
      dst.rgba[(std::size_t(row*h*scale+y*scale+dy)*dst.width+col*w*scale+x*scale+dx)*4+c]=
          src.rgba[(std::size_t(sy)*src.width+sx)*4+c];
  }
}
std::uint64_t hash(const Image& i) { std::uint64_t h=1469598103934665603ull; for(auto b:i.rgba) {h^=b;h*=1099511628211ull;} return h; }
void assets(const AssetLibrary& lib,const fs::path& out) {
  std::ofstream table(out/"assets.tsv"); table<<"asset\tclip\tentries\tdirections\trate\ttextures\n";
  for(const auto& name:spriteSheetNames(lib)) {
    if(name.find("LEMMING")==std::string::npos && name.find("BUBBLE")==std::string::npos)continue;
    auto sheet=loadSpriteSheet(lib,name);check(bool(sheet),"read "+name);if(!sheet)continue;
    for(const auto& s:sheet->sprites) {
      std::set<std::string> textures;
      for(const auto& t:s.animations)for(const auto& f:t.frames)textures.insert(sheetAssetName(f.sheet));
      table<<name<<'\t'<<s.name<<'\t'<<s.animations.size()<<'\t'<<(s.animations.empty()?0:s.animations[0].frames.size())<<'\t'<<s.rate<<'\t';
      for(const auto& t:textures)table<<t<<';';
      table<<'\n';
      if((name.find("HI.SPR")==std::string::npos && s.name!="bubbles") || (s.name!="drown"&&s.name!="walk"&&s.name!="bubbles"))continue;
      Image strip=blank(int(s.animations.size())*64,96);
      for(std::size_t n=0;n<s.animations.size();++n) {
        if(s.animations[n].frames.empty())continue;
        const auto& f=s.animations[n].frames[0]; auto texture=loadTexture(lib,sheetAssetName(f.sheet));
        check(bool(texture),"read frame texture");if(!texture)continue;
        app::ActorSpritePlacement p; p.left=n*64+8;p.top=12;p.right=p.left+f.width;p.bottom=p.top+f.height;
        check(bool(app::drawActorSprite(strip,f,*texture,p,false)),"draw installed frame");
      }
      png(out/(fs::path(name).filename().string()+"-"+s.name+".png"),strip);
    }
  }
}
void push(SDL_Event e) { check(SDL_PushEvent(&e)==1,"queue public input"); }
void key(SDL_Scancode c) { SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.scancode=c;push(e); }
void motion(int x,int y) { SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=x;e.motion.y=y;push(e); }
void click(int x,int y) { SDL_Event e{};e.type=SDL_MOUSEBUTTONDOWN;e.button.button=SDL_BUTTON_LEFT;e.button.x=x;e.button.y=y;push(e);e.type=SDL_MOUSEBUTTONUP;push(e); }
void application(const fs::path& game,const fs::path& out,const fs::path& prior,const std::string& level,bool skill=false) {
  fs::create_directories(out);
  std::ofstream(out/"camera.json")<<"{\"viewport\":[640,480],\"yaw_units\":"<<(level=="MM_BUILD_BLOCK_A"?176:level=="MM_W_012"?128:level=="M20_MT_1"?140:32)<<",\"level_eye_height\":"<<(level=="MM_BUILD_BLOCK_A"?40:level=="M20_MT_1"?64:96)<<"}\n";
  const int frames=level=="MM_BUILD_BLOCK_A"?146:level=="M20_MT_1"?95:80;
  std::vector<std::string> args{"type-sprites","--game-dir",game.string(),"--level",level,"--sim","--native","--camera",(out/"camera.json").string(),"--frames",std::to_string(frames),"--no-audio"};
  std::vector<char*> argv;for(auto& a:args)argv.push_back(a.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  Saved privateSave(out/"SavedGame.dat");
  std::ofstream trace(out/"frames.csv"),simtrace(out/"simulation.txt"),ident(out/"hashes.txt");
  std::ifstream oldsim(prior/"simulation.txt"),oldhash(prior/"hashes.txt");
  trace<<"frame,ms,index,state,type,clip,entry,x,y,drawn_y,left,top,bottom\n";
  Image strip=blank(20*128,160);int drownStart=-1,cx=320,cy=90,samples=0;
  Image stateStrip=blank(3*128,160);
  bool falling=false,walking=false,skilled=false,clicked=false,hovered=false;int phase=0;
  double firstDrownY=0.0,lastDrownY=0.0;
  const auto observe=[&](const app::LevelFrameView& f) {
    std::ostringstream state;state<<std::setprecision(17)<<f.frame<<' '<<f.world.levelClockMs()<<' '<<f.world.dead()<<' '<<f.world.saved();
    for(const auto& l:f.world.lemmings()) { state<<' '<<l.x<<','<<l.y<<','<<int(l.state)<<','<<l.sheetType<<','<<l.frame; }
    simtrace<<state.str()<<'\n';std::string old;
    if(!prior.empty()) {check(bool(std::getline(oldsim,old))&&old==state.str(),"simulation unchanged "+level+" frame "+std::to_string(f.frame));}
    ident<<hash(f.image)<<'\n';
    if(!prior.empty()) {
      check(bool(std::getline(oldhash,old)),"prior frame hash exists");
      // Build and Block contains flames seeded from wall time in production;
      // whole-frame identity is meaningful on the ordinary-only Climber route.
      // Ordinary actors in mixed levels are compared by the fixed-pose scene
      // checks below, without weakening any existing project assertion.
      if(level=="MM_CLIMB_002")
        check(old==std::to_string(hash(f.image)),"ordinary non-drown frame identical "+level+" frame "+std::to_string(f.frame));
    }
    for(const auto& a:f.actors) {
      const auto& l=f.world.lemmings()[a.index];
      trace<<f.frame<<','<<f.world.levelClockMs()<<','<<a.index<<','<<sim::stateName(l.state)<<','<<l.sheetType<<','<<a.clip<<','<<a.temporalIndex<<','<<l.x<<','<<l.y<<','<<a.drawnSample[1]<<','<<a.placement.left<<','<<a.placement.top<<','<<a.placement.bottom<<'\n';
      if(a.index==0 && a.placement.top>5 && a.placement.bottom<440) {
        const int cx=int(a.placement.originX),cy=int(a.placement.originY)-8;
        if(l.state==sim::State::Walker&&!walking) {png(out/"walker.png",f.image);cropInto(stateStrip,f.image,cx,cy,0,0);walking=true;}
        if(l.state==sim::State::Faller&&!falling) {png(out/"faller.png",f.image);cropInto(stateStrip,f.image,cx,cy,1,0);falling=true;}
        if(l.state==sim::State::Blocker&&!skilled) {png(out/"blocker.png",f.image);cropInto(stateStrip,f.image,cx,cy,2,0);skilled=true;}
        if(f.hoveredActor==0 && f.markedActor==0 && !hovered) {png(out/"hover.png",f.image);hovered=true;}
        if(skill && !clicked && l.state==sim::State::Walker && a.placement.left>100 && a.placement.right<620) {
          int x=int((a.placement.left+a.placement.right)/2),y=int((a.placement.top+a.placement.bottom)/2);
          if(phase==0){key(SDL_SCANCODE_P);key(SDL_SCANCODE_4);motion(x,y);phase=1;}
          else if(f.paused&&f.hoveredActor&&*f.hoveredActor==0){click(x,y);key(SDL_SCANCODE_P);motion(620,460);clicked=true;}
          else motion(x,y);
        }
      }
    }
    if(level=="MM_BUILD_BLOCK_A"&&!f.world.lemmings().empty()) {
      const auto& l=f.world.lemmings()[0];
      auto a=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& a){return a.index==0;});
      if(drownStart<0&&l.state==sim::State::Drown) {
        drownStart=f.frame;
        if(a!=f.actors.end()){cx=int(a->placement.originX);cy=int(a->placement.originY)+12;firstDrownY=a->drawnSample[1];}
        png(out/"first-contact.png",f.image);
      }
      if(drownStart>=0&&f.frame-drownStart<20) {
        cropInto(strip,f.image,cx,cy,f.frame-drownStart,0);++samples;
        if(a!=f.actors.end())lastDrownY=a->drawnSample[1];
        png(out/("drown-"+std::to_string(f.frame-drownStart)+".png"),f.image);
      }
    }
  };
  check(app::runApplication(int(argv.size()),argv.data(),{},observe,options)==0,"public app "+level);
  if(level=="MM_BUILD_BLOCK_A") {
    check(samples==20,"20 natural drowning samples");png(out/"drown-strip.png",strip);
#ifndef TYPE_SPRITES_BEFORE
    check(firstDrownY-lastDrownY>10.0,"production drowning visibly sinks while simulation matches baseline");
#endif
  }
  else {check(walking,"natural walking visible "+level);check(falling,"natural falling visible "+level);}
  if(skill)check(clicked&&skilled&&hovered,"water blocker and type-specific hover through public input");
  png(out/"state-strip.png",stateStrip);
  check(privateSave.unchanged(),"private standalone save unchanged");
}
void fixtures(const AssetLibrary& lib,const fs::path& out,const fs::path& prior) {
  std::ofstream trace(out/"fixtures.tsv");
  std::ofstream hashes(out/"fixture-hashes.txt");std::ifstream oldhash(prior/"fixture-hashes.txt");
  for(const auto& id:{"MM_W_012","M20_MT_1"}) {
    auto setup=sim::loadRetailLevel(lib,id);check(bool(setup),"installed level");if(!setup)continue;
    std::ofstream data(out/(std::string(id)+"-hatches.txt"));
    for(auto h:setup->entrances)data<<h.x<<','<<h.y<<','<<h.sheetType<<'\n';
    check(std::any_of(setup->entrances.begin(),setup->entrances.end(),[&](const auto& h){return h.sheetType==(std::string(id)=="MM_W_012"?1:2);}),"installed hatch type");
    auto resolved=sim::resolveRetailLevel(lib,id);auto blob=lib.read(resolved->file);auto mesh=findCylinderMesh(*blob);auto idx=indexLevel(*blob);
    auto scene=app::TubeScene::build(lib,*mesh,*blob,idx->grid0);check(bool(scene),"scene");if(!scene)continue;
    app::CameraParams cp;cp.yawUnits=std::string(id)=="MM_W_012"?128:setup->entrances[0].x;cp.levelEyeHeight=std::string(id)=="MM_W_012"?96:64;
    auto camera=app::buildCamera(cp);
    const sim::State states[]{sim::State::Walker,sim::State::Faller,sim::State::Climber,sim::State::Hoister,sim::State::Floater,sim::State::Blocker,sim::State::Builder,sim::State::Shrugger,sim::State::Basher,sim::State::Miner,sim::State::Digger,sim::State::Drown,sim::State::Exploder,sim::State::Teleporting};
    Image sheet=blank(3*128,int(std::size(states))*160);
    for(std::size_t n=0;n<std::size(states);++n) {
      std::array<std::uint64_t,3> typeHashes{};
      for(int type=0;type<3;++type) {
      auto placed=*setup;placed.totalLemmings=0;sim::World w(std::move(placed));
      // Acid pose is a naturally observed falling sample below the hatch.
      sim::Lemming l;l.x=setup->entrances[0].x;l.y=std::string(id)=="M20_MT_1"?37:setup->entrances[0].y+6;l.state=states[n];l.sheetType=type;w.place(l);
      app::LemmingAnimTracker tracker;
      const double dt=states[n]==sim::State::Hoister?0.9:0.15;
#ifdef TYPE_SPRITES_BEFORE
      tracker.advance(w,*scene->lemmingSprites(),dt);
#else
      tracker.advance(w,std::array<const SpriteSheet*,3>{scene->lemmingSprites(0),scene->lemmingSprites(1),scene->lemmingSprites(2)},dt);
      if(states[n]==sim::State::Hoister)
        check(tracker.poseOf(0).temporalIndex==(type==0?13:14),"type's own hoist length drives its clock");
#endif
      app::SceneOptions opts;opts.drawCellRoute=true;opts.drawWall=false;
      opts.injectedWalkers=app::injectedWalkersFromWorld(w,w.terrain().height(),&tracker);
#ifndef TYPE_SPRITES_BEFORE
      check(opts.injectedWalkers[0].sheetType==type,"real level adapter preserves species");
      const auto* sprites=scene->lemmingSprites(type);check(sprites&&sprites->find(tracker.poseOf(0).clip),"per-type clip present");
      const auto* ordinary=scene->lemmingSprites();
      check(type==0||sprites->find("walk")->animations[0].frames[0].sheet!=ordinary->find("walk")->animations[0].frames[0].sheet,"different physical texture per type");
#endif
      app::Raster raster(camera.width,camera.height);scene->render(raster,camera,opts,&lib);
      auto boxes=scene->injectedWalkerScreenBoxes(camera,opts,lib,raster);
      check(boxes[0].has_value(),"placed installed-level state visible "+std::string(id));
      if(boxes[0]) {
        Image image=blank(camera.width,camera.height);const auto& fb=raster.framebuffer();
        for(std::size_t i=0;i<image.pixelCount();++i)for(int c=0;c<3;++c)image.rgba[i*4+c]=fb.rgb[i*3+c];
        cropInto(sheet,image,int(boxes[0]->originX),int(boxes[0]->originY)-8,type,int(n));
        typeHashes[type]=hash(image);hashes<<typeHashes[type]<<'\n';
        if(!prior.empty()) {
          std::string old;check(bool(std::getline(oldhash,old)),"prior fixture hash exists");
          check((old==std::to_string(typeHashes[type]))==(type==0),"only special types change installed-level state pixels");
        }
      }
      trace<<id<<'\t'<<type<<'\t'<<sim::stateName(states[n])<<'\t'<<tracker.poseOf(0).clip<<'\t'<<tracker.poseOf(0).temporalIndex<<'\t'<<bool(boxes[0])<<'\n';
    }
    #ifndef TYPE_SPRITES_BEFORE
      check(typeHashes[0]!=typeHashes[1]&&typeHashes[0]!=typeHashes[2]&&typeHashes[1]!=typeHashes[2],"rendered sheets differ per species for every state");
    #endif
    }
    png(out/(std::string(id)+"-type-states.png"),sheet);
  }
}
}
int main(int argc,char** argv) {
  if(argc!=4||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  const fs::path game=std::getenv("WEBOLUTION_GAME_DIR"),out=argv[2],prior=std::string(argv[3])=="-"?fs::path{}:fs::path(argv[3]);
  fs::create_directories(out);
  Saved installed(game/"SavedGame.dat");auto local=save::localAppDataDirectory();if(!local)return 2;Saved normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  assets(*lib,out);fixtures(*lib,out,prior);
  for(const auto& id:{"MM_W_012","MM_BUILD_BLOCK_A","MM_CLIMB_002","M20_MT_1"})
    application(game,out/id,prior.empty()?fs::path{}:prior/id,id);
  application(game,out/"water-skill",prior.empty()?fs::path{}:prior/"water-skill","MM_W_012",true);
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves untouched");
  std::ofstream(out/"saves-proof.txt")<<"installed_unchanged="<<installed.unchanged()<<"\nnormal_unchanged="<<normal.unchanged()<<'\n';
  return failures?1:0;
}
