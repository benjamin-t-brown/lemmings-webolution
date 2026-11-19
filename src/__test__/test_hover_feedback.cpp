// Source-built application only. Public SDL pointer events, private saves.
#include "app/AppEntry.h"
#include "app/RuntimeSkillPanel.h"
#include "PanelIconComparison.h"
#include "app/Pick.h"
#include "app/RuntimePick.h"
#include "assets/Png.h"
#include "assets/Box.h"
#include "sim/World.h"
#include "shell/HudText.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <utility>
using namespace webo;
namespace fs = std::filesystem;
namespace {
#ifdef HOVER_FEEDBACK_BEFORE
constexpr bool before = true;
#else
constexpr bool before = false;
#endif
int failures = 0;
void check(bool ok, const std::string& what) {
  if (!ok && ++failures < 30) std::cerr << "FAIL: " << what << '\n';
}
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary); return {std::istreambuf_iterator<char>(in), {}};
}
struct Saved {
  fs::path path; bool exists; std::string content;
  explicit Saved(fs::path p):path(p),exists(fs::exists(p)),content(bytes(p)) {}
  bool unchanged() const { return exists == fs::exists(path) && content == bytes(path); }
};
struct Command { char kind; int x=0,y=0; };
void send(Command c) {
  SDL_Event e{};
  if (c.kind == 'm') { e.type=SDL_MOUSEMOTION; e.motion.x=c.x; e.motion.y=c.y; }
  else if (c.kind == 'k') {
    e.type=SDL_KEYDOWN; e.key.keysym.scancode=SDL_Scancode(c.x);
    e.key.keysym.sym=SDL_GetKeyFromScancode(SDL_Scancode(c.x));
  } else if (c.kind == 'c') {
    e.type=SDL_MOUSEBUTTONDOWN; e.button.button=SDL_BUTTON_LEFT; e.button.x=c.x; e.button.y=c.y;
  } else e.type=SDL_QUIT;
  check(SDL_PushEvent(&e)==1,"public input queued");
}
app::PickBox box(const app::ActorSpritePlacement& p) {
  return {int(p.left),int(p.top),int(p.right),int(p.bottom)};
}
bool bodyBrightened(const app::LevelFrameView& f, const Image& unmarked,
                    const Image& bracket, const app::ActorSpritePlacement& body) {
  for (int y = std::max(0, int(std::ceil(body.top)));
       y < std::min(f.image.height, int(std::ceil(body.bottom))); ++y) {
    for (int x = std::max(0, int(std::ceil(body.left)));
         x < std::min(f.image.width, int(std::ceil(body.right))); ++x) {
      const int cx = x - (f.cursorX - bracket.width / 2);
      const int cy = y - (f.cursorY - bracket.height / 2);
      if (cx >= 0 && cx < bracket.width && cy >= 0 && cy < bracket.height &&
          bracket.rgba[(std::size_t(cy) * bracket.width + cx) * 4 + 3] != 0) continue;
      const auto at = (std::size_t(y) * f.image.width + x) * 4;
      const auto* lit = f.image.rgba.data() + at;
      const auto* base = unmarked.rgba.data() + at;
      if (lit[0] >= base[0] && lit[1] >= base[1] && lit[2] >= base[2] &&
          !std::equal(lit, lit + 3, base)) return true;
    }
  }
  return false;
}
// V7 displayed-centre oracle replaces the old terrain-target expectation.
// This historical feedback comparison uses a crowd point where both policies
// agree so its frozen pre-feedback main still assigns the same actor. The new
// pick_nearest routes independently demonstrate policy disagreements.
bool overlap(const app::LevelFrameView& f,int& x,int& y,std::size_t& expected,int& n) {
  for (y=45;y<420;y+=2) for(x=110;x<620;x+=2) {
    const app::PickBox cursor{x-16,y-16,x+16,y+16};
    n=0; bool allWalkers=true; double best=INFINITY,oldBest=INFINITY;std::size_t oldExpected=0;
    for(const auto& a:f.actors) {
      const auto& l=f.world.lemmings()[a.index];
      if(!app::pickBoxOverlaps(box(a.placement),cursor) || !l.active() || l.canClimb) continue;
      if(l.state!=sim::State::Walker) { allWalkers=false; break; }
      const auto supported=app::firstLevelClimberSupport(l,sim::Skill::Climber,f.world.terrain());
      if(supported && !*supported) continue;
      ++n;
      double oldD=0;for(int i=0;i<3;++i) oldD+=(a.drawnSample[i]-f.cameraPickReference[i])*(a.drawnSample[i]-f.cameraPickReference[i]);
      if(oldD<oldBest){oldBest=oldD;oldExpected=a.index;}
      const double dx=(a.placement.left+a.placement.right)/2-x,dy=(a.placement.top+a.placement.bottom)/2-y;
      const double d=dx*dx+dy*dy;
      if(d<best) { best=d; expected=a.index; }
    }
    if(allWalkers && n>=2 && expected==oldExpected) return true;
  }
  return false;
}
void state(const fs::path& p,const app::LevelFrameView& f) {
  std::ofstream s(p);
  s<<f.world.spawned()<<','<<f.world.saved()<<','<<f.world.dead()<<','<<f.paused<<','<<f.selectedSkillIndex<<'\n';
  for(auto n:f.world.setup().skills) s<<n<<',';
  s<<'\n';for(const auto& l:f.world.lemmings()) s<<l.x<<','<<l.y<<','<<int(l.state)<<','<<l.canClimb<<','<<l.fuseMs<<'\n';
}
void image(const fs::path& out,const std::string& name,const app::LevelFrameView& f) {
  check(writePng(out/(name+".png"),f.image).has_value(),"PNG retained");
  std::ofstream raw(out/(name+".rgba"),std::ios::binary);
  raw.write(reinterpret_cast<const char*>(f.image.rgba.data()),f.image.rgba.size());
  state(out/(name+".state"),f);
  std::ofstream(out/(name+".pointer"))<<f.cursorX<<' '<<f.cursorY<<'\n';
}
int route(const std::string& mode,const fs::path& out) {
  fs::create_directories(out);
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  const auto bracket = loadTexture(*lib, "TEXTURES\\_CURLEM.BMP");
  if (!bracket) return 2;
  auto local=save::localAppDataDirectory(); if(!local)return 2;
  Saved normal(save::cloneSavePathUnder(*local)),installed(game/"SavedGame.dat");
  save::SavedGame seed; for(auto& r:seed.records)r.c=1;
  const auto data=seed.serialize();
  {std::ofstream s(out/"SavedGame.dat",std::ios::binary);s.write(reinterpret_cast<const char*>(data.data()),data.size());}
  Saved privateSave(out/"SavedGame.dat");
  // argv destination is .../before/actor or .../after/actor.
  const auto paired=out.parent_path().parent_path()/"after"/mode;
  std::map<int,std::vector<Command>> tape;
  struct Shot { std::string name; char kind; int left,top,right,bottom; };
  std::map<int,Shot> shots;
  if(before) {
    std::ifstream ev(paired/"events.txt"); int f; Command c;
    while(ev>>f>>c.kind>>c.x>>c.y)tape[f].push_back(c);
    std::ifstream sh(paired/"shots.txt");Shot s;
    while(sh>>f>>s.name>>s.kind>>s.left>>s.top>>s.right>>s.bottom)shots.emplace(f,s);
    check(!tape.empty()&&!shots.empty(),"authentic baseline replays freshly recorded public event tape");
    if(tape.empty()||shots.empty())return 1;
  }
  std::ofstream events(out/"events.txt"),shotlog(out/"shots.txt"),trace(out/"trace.csv");
  trace<<"frame,spawned,hovered_actor,marked_actor,hovered_hatch,remaining\n";
  auto command=[&](int f,Command c){events<<f<<' '<<c.kind<<' '<<c.x<<' '<<c.y<<'\n';send(c);};
  auto shot=[&](const app::LevelFrameView& f,std::string name,char kind,int l=0,int t=0,int r=640,int b=480){
    check(f.selectedSkillIndex==0&&app::runtimeSkillPanelIndex(f.cursorX,f.cursorY)<0,
          "historical comparison has only selected Climber animation");
    shotlog<<f.frame<<' '<<name<<' '<<kind<<' '<<l<<' '<<t<<' '<<r<<' '<<b<<'\n';image(out,name,f);
  };
  int x=0,y=0,stock=0,n=0;std::size_t expected=0;int captures=0,offFrame=-1,firstRemaining=-1;
  int lastRemaining=-1; bool clicked=false; int observed=0;
  Image unmarkedActorFrame;
  auto frame=[&](const app::LevelFrameView& f){
    ++observed;
    int remaining=-1;
    for(const auto& h:f.hatches)if(f.hoveredHatch==h.index)remaining=h.remaining;
    trace<<f.frame<<','<<f.world.spawned()<<','<<(f.hoveredActor?int(*f.hoveredActor):-1)<<','
      <<(f.markedActor?int(*f.markedActor):-1)<<','<<(f.hoveredHatch?int(*f.hoveredHatch):-1)<<','<<remaining<<'\n';
    if(before) {
      if(auto it=shots.find(f.frame);it!=shots.end())image(out,it->second.name,f);
      if(auto it=tape.find(f.frame);it!=tape.end())for(auto c:it->second)send(c);
      return;
    }
    check(f.markedActor==f.hoveredActor,"exactly the picked actor is marked, or neither");
    if(mode=="actor") {
      if(f.frame==0) {command(f.frame,{'k',SDL_SCANCODE_P});command(f.frame,{'m',630,470});}
      else if(f.frame==1) {
        check(f.paused,"paused public actor route");shot(f,"empty",'n');
        unmarkedActorFrame = f.image;
        check(overlap(f,x,y,expected,n),"installed MM_CLIMB_002 has at least two overlapping eligible walkers");
        stock=f.world.setup().skills[0];command(f.frame,{'m',x,y});
      } else if(f.frame==2 || f.frame==5) {
        check(f.hoveredActor==expected && f.markedActor==expected,"overlap marking agrees with independent nearest-walker oracle");
        for (const auto& a : f.actors) if (a.index == expected) {
          check(bodyBrightened(f, unmarkedActorFrame, *bracket, a.placement),
                "selected body brightens outside the opaque cursor pixels");
          shot(f,f.frame==2?"overlap":"reentered",'a',int(std::floor(a.placement.left)),int(std::floor(a.placement.top)),int(std::ceil(a.placement.right)),int(std::ceil(a.placement.bottom)));
        }
        if(f.frame==2)command(f.frame,{'m',630,470});
        else {command(f.frame,{'c',x,y});clicked=true;}
      } else if(f.frame==3) {
        check(!f.hoveredActor&&!f.markedActor&&!f.hoveredHatch,"moving off clears all feedback");shot(f,"off",'n');
      } else if(f.frame==4)command(f.frame,{'m',x,y});
      else if(f.frame==6) {
        check(clicked && f.world.lemmings()[expected].canClimb && f.world.setup().skills[0]==stock-1,"click assigns marked actor and spends exactly one skill");
        check(std::count_if(f.world.lemmings().begin(),f.world.lemmings().end(),[](const auto& l){return l.canClimb;})==1,"only one overlapping actor receives the click");
        shot(f,"assigned",'s');command(f.frame,{'q'});
      }
    } else {
      if(f.frame==0)command(f.frame,{'m',630,470});
      if(f.frame==115)shot(f,"empty",'n');
      if(f.frame==120) {
        check(!f.hatches.empty(),"ordinary frontend shows projected hatch geometry");
        if(f.hatches.empty()){command(f.frame,{'q'});return;}
        const auto& h=f.hatches.front();x=h.left+2;y=h.top+2;
        command(f.frame,{'m',x,y});
      }
      if(f.frame>120 && offFrame<0 && f.hoveredHatch) {
        check(!f.hoveredActor&&!f.markedActor,"hatch feedback has no marked lemming");
        check(f.world.setup().entrances.size()==1,"single-hatch route makes independent remaining-count oracle exact");
        check(remaining==f.world.setup().totalLemmings-f.world.spawned(),"count is unreleased population, not live/saved count");
        if(remaining!=lastRemaining) {
          if(firstRemaining<0)firstRemaining=remaining;
          else check(remaining<lastRemaining,"remaining count decreases on release");
          lastRemaining=remaining;++captures;
          for(const auto& h:f.hatches)if(f.hoveredHatch==h.index)
            shot(f,"hatch-"+std::to_string(remaining),'h',h.left,h.top,h.right,h.bottom);
          if(captures==3){offFrame=f.frame+1;command(f.frame,{'m',630,470});}
        }
      }
      if(f.frame==offFrame) {check(!f.hoveredHatch,"moving off clears hatch label");shot(f,"off",'n');command(f.frame,{'q'});}
      if(f.frame==400){check(false,"hatch hover/count route watchdog");command(f.frame,{'q'});}
    }
  };
  std::array<int32_t,102> flags{};flags.fill(1);auto map=app::MapScene::build(*lib,flags);if(!map)return 2;
  int mx=-1,my=-1;
  for(int yy=0;yy<480&&mx<0;++yy)for(int xx=0;xx<640;++xx)if(map->pickNode(xx,yy)==0){mx=xx;my=yy;break;}
  auto front=[&](const app::FrontendSession& s,int){
    if(s.screen()==shell::Screen::Menu){send({'k',SDL_SCANCODE_DOWN});send({'k',SDL_SCANCODE_RETURN});}
    else if(s.screen()==shell::Screen::Map)send({'c',mx,my});
    else {check(false,"expected frontend entry route");send({'q'});}
  };
  std::vector<std::string> args{"hover-feedback","--native","--frames","450","--game-dir",game.string()};
  if(mode=="actor")args.insert(args.end(),{"--level","MM_CLIMB_002","--sim-ticks","100"});
  else args.push_back("--frontend");
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  check(app::runApplication(int(raw.size()),raw.data(),front,frame,options)==0,"application route exits successfully");
  check(mode=="actor"?observed==7:observed>120,"public route reached evidence frames");
  if(!before&&mode=="hatch")check(captures==3&&lastRemaining<firstRemaining,"three decreasing hatch counts retained");
  check(installed.unchanged()&&normal.unchanged()&&privateSave.unchanged(),"installed, normal and private saves untouched");
  std::ofstream(out/"summary.txt")<<"mode="<<mode<<"\nbefore="<<before<<"\nframes="<<observed<<"\noverlap_candidates="<<n
    <<"\nselected="<<expected<<"\ncount_images="<<captures<<"\nall_saves_unchanged="<<(installed.unchanged()&&normal.unchanged()&&privateSave.unchanged())<<"\nfailures="<<failures<<'\n';
  return failures?1:0;
}
int compare(const fs::path& root) {
  std::ofstream summary(root/"comparison.txt");
  const char* game=std::getenv("WEBOLUTION_GAME_DIR");if(!game)return 2;
  auto library=AssetLibrary::open(fs::path(game)/"lemmings.box",fs::path(game)/"patch.box");if(!library)return 2;
  auto font=loadTexture(*library,"TEXTURES\\^FONT.BMP");if(!font)return 2;
  const auto iconPixels=test::climberIconComparisonPixels(*library);
  summary<<"R7 animated Climber footprint="<<iconPixels.size()<<" pixels; exact comparison elsewhere\n";
  for(const auto& mode:{"actor","hatch"}) {
    const auto after=root/"after"/mode,old=root/"before"/mode;
    std::ifstream shots(after/"shots.txt");int frame,l,t,r,b;std::string name;char kind;int count=0;
    while(shots>>frame>>name>>kind>>l>>t>>r>>b) {
      ++count;auto a=bytes(after/(name+".rgba")),o=bytes(old/(name+".rgba"));
      check(a.size()==640*480*4&&a.size()==o.size(),"paired raw framebuffer sizes");
      // Both routes select Climber and point outside the panel. Static vs
      // animated Climber pixels encode R7, not an actor/hatch feedback failure.
      for(auto at:iconPixels)for(int c=0;c<4;++c) {a.at(at+c)=0;o.at(at+c)=0;}
      check(bytes(after/(name+".state"))==bytes(old/(name+".state")),"paired simulation and stock identical: "+name);
      int changed=0,outside=0;
      for(std::size_t at=0;at+3<std::min(a.size(),o.size());at+=4) {
        if(a.compare(at,4,o,at,4)==0)continue;
        ++changed;
        const int x=int((at/4)%640),y=int((at/4)/640);
        if(x<l||x>=r||y<t||y>=b)++outside;
      }
      if(kind=='n')check(changed==0,"nothing hovered is pixel-identical: "+name);
      if(kind=='a')check(changed>0&&outside==0,"only selected actor bounds change: "+name);
      if(kind=='h') {
        check(changed>10,"visible hatch marking/count added: "+name);
        // Independently compose the expected numeric glyphs on the authentic
        // baseline. Brackets alone cannot pass the count-display regression.
        const auto cells=shell::fontCells();
        const auto text=name.substr(6); // observed numeric count, asserted against World above
        const int width=shell::textWidth(text,cells);
        const shell::HudLine line{text,std::clamp((l+r-width)/2,0,640-width),
          std::clamp((t+b)/2-44,0,480-shell::kLineHeight),{255,255,255}};
        Image label;label.width=640;label.height=480;label.rgba.assign(o.begin(),o.end());
        shell::drawHud(label,*font,std::span<const shell::HudLine>(&line,1),cells);
        int px=0,py=0;std::ifstream(after/(name+".pointer"))>>px>>py;
        int glyphPixels=0;
        for(int y=line.y;y<line.y+shell::kLineHeight;++y)for(int x=line.x;x<line.x+width;++x) {
          if(x>=px-16&&x<px+16&&y>=py-16&&y<py+16)continue; // later software cursor owns these pixels
          const auto at=(std::size_t(y)*640+x)*4;
          if(at+3>=a.size()||at+3>=o.size())continue;
          bool ink=false;for(int c=0;c<3;++c)ink|=label.rgba[at+c]!=static_cast<unsigned char>(o[at+c]);
          if(!ink)continue;
          ++glyphPixels;
          for(int c=0;c<3;++c)check(label.rgba[at+c]==static_cast<unsigned char>(a[at+c]),"actual recovered-font count glyph: "+name);
        }
        check(glyphPixels>10,"remaining count has independently verified visible glyph pixels: "+name);
        summary<<"  verified_numeric_glyph_pixels="<<glyphPixels<<'\n';
      }
      summary<<mode<<'/'<<name<<": changed="<<changed<<", outside="<<outside<<'\n';
    }
    check(count>=4,"paired capture inventory exists");
  }
  return failures?1:0;
}
int units() {
  // A repeat Floater is still a pick, even though the next click would be refused.
  constexpr auto floater = sim::Skill::Floater;
  constexpr auto slot = static_cast<std::size_t>(floater);
  sim::LevelSetup setup;
  setup.skills[slot] = 2;
  sim::World world(std::move(setup));
  world.place(sim::Lemming{});
  check(world.canAssign(0, floater), "initial Floater is eligible for highlighting");
  check(world.setup().skills[slot] == 2 && !world.lemmings()[0].canFloat && world.takeCues().empty(),
        "checking highlight eligibility leaves the world and audio unchanged");
  check(world.assign(0, floater), "first Floater click succeeds");
  check(world.setup().skills[slot] == 1 && app::passesKnownPickExclusions(world.lemmings()[0], floater),
        "the repeat click can resolve to the same actor with stock remaining");
  check(!world.canAssign(0, floater) && !world.assign(0, floater) && world.setup().skills[slot] == 1,
        "a repeat Floater is ineligible for highlighting and refused without spending stock");

  // Uneven global totals, all release prefixes, every hatch: future allocation
  // independently enumerated from the unchanged round-robin World contract.
  for(int count=1;count<=8;++count)for(int total=0;total<25;++total)for(int spawned=0;spawned<=total;++spawned)
    for(int index=0;index<count;++index) {
      int expected=0;for(int future=spawned;future<total;++future)expected+=(future%count==index);
      check(app::hatchRemaining(total,spawned,count,index)==expected,"per-hatch future allocation");
    }
  check(app::hatchRemaining(10,0,0,0)==0&&app::hatchRemaining(10,0,1,1)==0,"invalid hatch is empty");
  return failures?1:0;
}
}
int main(int argc,char** argv) {
  if(argc<2)return 2;
  const std::string mode=argv[1];
  if(mode=="units")return units();
  if(argc!=3)return 2;
  if(mode=="compare")return compare(argv[2]);
  if(!std::getenv("WEBOLUTION_GAME_DIR")||!std::getenv("SDL_VIDEODRIVER")||std::string(std::getenv("SDL_VIDEODRIVER"))!="dummy")return 2;
  return route(mode,argv[2]);
}
