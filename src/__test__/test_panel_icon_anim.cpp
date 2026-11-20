// Check installed panel icon animation through the application's SDL input.
#include "app/AppEntry.h"
#include "PanelIconComparison.h"
#include "app/SkillPanel.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "sim/World.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>

using namespace webo;
namespace fs = std::filesystem;
namespace {
#ifdef PANEL_ICON_BEFORE
constexpr bool before = true;
#else
constexpr bool before = false;
#endif
int failures = 0;
void check(bool ok, const std::string& why) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
std::string bytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary); return {std::istreambuf_iterator<char>(in), {}};
}
struct Saved {
  fs::path path; bool existed; std::string data;
  explicit Saved(fs::path p): path(p), existed(fs::exists(p)), data(bytes(p)) {}
  bool unchanged() const { return existed == fs::exists(path) && data == bytes(path); }
};
Image canvas() {
  Image im; im.width = 640; im.height = 480; im.rgba.resize(640*480*4);
  for (int y=0;y<480;++y) for (int x=0;x<640;++x) {
    auto* p=im.rgba.data()+(y*640+x)*4;
    p[0]=x%256; p[1]=y%256; p[2]=(x+y)%256; p[3]=255;
  }
  return im;
}
std::string region(const Image& im,int plate) {
  std::string result;
  for (int y=12+40*plate;y<44+40*plate;++y)
    for (int x=18;x<60;++x) for(int c=0;c<4;++c) result+=char(im.rgba[(y*640+x)*4+c]);
  return result;
}
void retain(const fs::path& path,const Image& im) {
  check(writePng(path.string()+".png",im).has_value(),"retain PNG");
  std::ofstream raw(path.string()+".rgba",std::ios::binary);
  raw.write(reinterpret_cast<const char*>(im.rgba.data()),im.rgba.size());
}
void measurement(const AssetLibrary& lib,const fs::path& out) {
  std::ofstream log(out/"assets.txt");
  const char* names[]={"climb","float","bomb","block","build","bash","mine","dig","questionmark","armageddon"};
  const int facing[]={2,4,0,4,2,2,2,4,0,0};
  for(int i=0;i<10;++i) {
    const char* sheet=i==2?"GREYICONS":i==8?"BLACKICONS":i==9?"ADDITIVEICONS":"LEMMINGSHI";
    auto spr=loadSpriteSheet(lib,std::string("SPRITES\\")+sheet+".SPR"); check(spr.has_value(),"read installed SPR"); if(!spr)continue;
    auto* clip=spr->find(names[i]); check(clip!=nullptr,"installed clip exists");if(!clip)continue;
    std::set<std::string> textures;
    log<<i<<' '<<names[i]<<" sheet="<<sheet<<" temporal="<<clip->animations.size()<<" rate="<<clip->rate<<" facing="<<facing[i]<<" directions=";
    for(const auto& entry:clip->animations) {
      log<<entry.frames.size()<<',';
      check(std::size_t(facing[i])<entry.frames.size(),"fixed facing exists in every temporal entry");
      if(std::size_t(facing[i])<entry.frames.size())textures.insert(entry.frames[facing[i]].sheet);
    }
    log<<" textures=";for(const auto& t:textures)log<<t<<';';log<<'\n';
  }
}
void fixtures(const AssetLibrary& lib,const fs::path& out) {
  auto panel=app::RetailSkillPanel::create(lib,"MM_CLIMB_002");
  check(panel.has_value(),"create production panel");if(!panel)return;
  auto idle=canvas();panel->draw(idle,panel->initialCounts(),std::nullopt);
  retain(out/"idle",idle);
#ifndef PANEL_ICON_BEFORE
  auto anim=app::SkillPanelIconAnimation::create(lib);check(anim.has_value(),"create animation cache");if(!anim)return;
  anim->advanceIcons(7.0,std::nullopt,-1);
  auto later=canvas();panel->draw(later,panel->initialCounts(),std::nullopt,false,&*anim);
  check(later.rgba==idle.rgba,"all idle plates remain pixel-identical after elapsed time");
#ifdef PANEL_ICON_BASELINE_AVAILABLE
  check(fs::exists(out.parent_path()/"before/idle.rgba"),"authentic idle baseline exists");
    check(bytes(out.parent_path()/"before/idle.rgba")==std::string(reinterpret_cast<const char*>(idle.rgba.data()),idle.rgba.size()),"idle image equals authentic pre-task renderer");
#endif
  // Compare equal UI time split into different frame rates and simultaneous
  // selected+hovered activation; non-boundary time avoids float tie ambiguity.
  auto atTime = [&](double seconds,int frames,int hover) {
    auto a=app::SkillPanelIconAnimation::create(lib);
    if(!a){check(false,"clock fixture loads");return canvas();}
    for(int n=0;n<frames;++n)a->advanceIcons(seconds/frames,0,hover);
    auto im=canvas();panel->draw(im,panel->initialCounts(),0,false,&*a);return im;
  };
  const auto once=atTime(0.317,1,-1);
  check(once.rgba==atTime(0.317,30,-1).rgba&&once.rgba==atTime(0.317,144,-1).rgba,
        "equal UI time gives same pose across frame rates");
  check(once.rgba==atTime(0.317,1,0).rgba,"hovering selection never doubles its speed");
  check(once.rgba==atTime(0.317+18*0.06*1000,1,-1).rgba,"selected animation wraps repeatedly at strip end");
#endif
  for(int i=0;i<8;++i) for(bool hover:{false,true}) {
    auto fresh=app::RetailSkillPanel::create(lib,"MM_CLIMB_002");if(!fresh){check(false,"reload panel");return;}
#ifndef PANEL_ICON_BEFORE
    auto animation=app::SkillPanelIconAnimation::create(lib);if(!animation){check(false,"load animation");return;}
#endif
    std::set<std::string> poses;
    const std::string tag=std::string(hover?"hover-":"selected-")+std::to_string(i);
    for(int n=0;n<5;++n) {
#ifndef PANEL_ICON_BEFORE
      animation->advanceIcons(n==0?0.0:0.19,hover?std::nullopt:std::optional<int>(i),hover?i:-1);
#endif
      auto image=canvas();fresh->draw(image,fresh->initialCounts(),hover?std::nullopt:std::optional<int>(i)
#ifndef PANEL_ICON_BEFORE
          ,false,&*animation
#endif
      );
      poses.insert(region(image,i));retain(out/(tag+"-"+std::to_string(n)),image);
#if defined(PANEL_ICON_BASELINE_AVAILABLE) && !defined(PANEL_ICON_BEFORE)
      if(i==0&&!hover&&n==1) {
        const auto mask=test::climberIconComparisonPixels(lib);
        std::ofstream(out/"comparison-footprint.txt")<<"Climber sprite union pixels="<<mask.size()<<'\n';
        const auto baseline=bytes(out.parent_path()/"before"/(tag+"-"+std::to_string(n)+".rgba"));
        check(baseline.size()==image.rgba.size(),"frozen selected frame exists");
        if(baseline.size()==image.rgba.size())for(std::size_t at=0;at<image.rgba.size();at+=4)
          if(!std::binary_search(mask.begin(),mask.end(),at)) for(int c=0;c<4;++c)
            check(image.rgba[at+c]==static_cast<unsigned char>(baseline[at+c]),"animation changes only traced sprite pixels");
      }
#endif
      for(int j=0;j<10;++j)if(j!=i)check(region(image,j)==region(idle,j),"inactive plate icon remains unchanged");
      auto repeat=canvas();fresh->draw(repeat,fresh->initialCounts(),hover?std::nullopt:std::optional<int>(i)
#ifndef PANEL_ICON_BEFORE
          ,false,&*animation
#endif
      );
      check(repeat.rgba==image.rgba,"draw alone does not advance frames");
    }
    check((before||i==2)?poses.size()==1:poses.size()>1,"hover/selected frame advance, except single-frame bomb: "+tag);
#ifndef PANEL_ICON_BEFORE
    animation->advanceIcons(0,std::nullopt,-1);
    auto reset=canvas();fresh->draw(reset,fresh->initialCounts(),std::nullopt,false,&*animation);
    check(reset.rgba==idle.rgba,"leaving active states restores retained idle pose");
#endif
  }
}
void push(SDL_Event event){check(SDL_PushEvent(&event)==1,"queue public SDL input");}
void motion(int x,int y){SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=x;e.motion.y=y;push(e);}
void key(SDL_Scancode code){SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.scancode=code;e.key.keysym.sym=SDL_GetKeyFromScancode(code);push(e);}
void route(const fs::path& game,const fs::path& out) {
  std::vector<std::string> args{"panel-icon-demo","--game-dir",game.string(),"--level","MM_CLIMB_002","--sim","--native","--frames","28","--no-audio"};
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.03;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"private-SavedGame.dat",game));
  std::set<std::string> selected,hovered;std::string idle;int seen=0;
  std::ofstream log(out/"public.csv");log<<"frame,paused,selected,pointerX,pointerY,worldMs\n";
  const auto observer=[&](const app::LevelFrameView& f) {
    ++seen;log<<f.frame<<','<<f.paused<<','<<f.selectedSkillIndex<<','<<f.cursorX<<','<<f.cursorY<<','<<f.world.levelClockMs()<<'\n';
    if(f.frame==0){key(SDL_SCANCODE_P);key(SDL_SCANCODE_1);motion(50,68);}
    if(f.frame>=1&&f.frame<=14) {
      check(f.paused&&f.selectedSkillIndex==0&&f.cursorX==50&&f.cursorY==68,"public hover and active selection while paused");
      selected.insert(region(f.image,0));hovered.insert(region(f.image,1));
      if(f.frame==1)idle=region(f.image,3);
      check(idle==region(f.image,3),"idle plate is stable through production loop");
      if(f.frame==1||f.frame==4||f.frame==8||f.frame==12)retain(out/("public-"+std::to_string(f.frame)),f.image);
    }
    if(f.frame==14)motion(320,240);
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observer,options)==0,"public application exits normally");
  check(seen==28,"bounded application demonstration completed");
  check(before?selected.size()==1:selected.size()>1,"production selected icon advances with simulation paused");
  check(before?hovered.size()==1:hovered.size()>1,"production hovered icon advances with simulation paused");
}
}
int main(int argc,char** argv) {
  if(argc!=2||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  const fs::path out=argv[1],game=std::getenv("WEBOLUTION_GAME_DIR");fs::create_directories(out);
  auto local=save::localAppDataDirectory();if(!local)return 2;
  Saved installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  measurement(*lib,out);fixtures(*lib,out);route(game,out);
  check(installed.unchanged()&&normal.unchanged(),"installed and normal user saves untouched");
  std::ofstream(out/"checks.txt")<<"failures="<<failures<<" installed_save_unchanged="<<installed.unchanged()<<" normal_save_unchanged="<<normal.unchanged()<<'\n';
  return failures?1:0;
}
