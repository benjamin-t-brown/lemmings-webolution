#include "app/AppEntry.h"
#include "app/ActorVisibility.h"
#include "app/LemmingAnim.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "shell/HudText.h"
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
#include <string>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok) { ++failures; if (failures < 25) std::cerr << "FAIL: " << message << '\n'; }
}
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary); return {std::istreambuf_iterator<char>(in), {}};
}
struct SaveSnapshot {
  fs::path path; bool exists; std::string data;
  explicit SaveSnapshot(fs::path p) : path(std::move(p)), exists(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return exists == fs::exists(path) && data == bytes(path); }
};
Image image(const app::Framebuffer& fb) {
  Image im; im.width=fb.width; im.height=fb.height; im.rgba.resize(std::size_t(fb.width)*fb.height*4,255);
  for(std::size_t i=0;i<fb.rgb.size()/3;++i)
    std::copy_n(fb.rgb.begin()+i*3,3,im.rgba.begin()+i*4);
  return im;
}
std::size_t changed(const app::Framebuffer& a, const app::Framebuffer& b) {
  std::size_t n=0;
  for(std::size_t i=0;i<a.rgb.size();i+=3)
    n+=!std::equal(a.rgb.begin()+i,a.rgb.begin()+i+3,b.rgb.begin()+i);
  return n;
}
void png(const fs::path& path, const Image& im) { check(writePng(path,im).has_value(),"write "+path.string()); }
// Review aid only: nearest-neighbour enlargement of source-rendered pixels.
// Full-sized frames above remain the measurement artifacts.
Image enlargedCrop(const Image& im,int left,int top,int width,int height) {
  Image crop;crop.width=width*6;crop.height=height*6;
  crop.rgba.assign(std::size_t(crop.width)*crop.height*4,255);
  for(int y=0;y<crop.height;++y)for(int x=0;x<crop.width;++x) {
    const int sx=std::clamp(left+x/6,0,im.width-1),sy=std::clamp(top+y/6,0,im.height-1);
    std::copy_n(im.rgba.begin()+(std::size_t(sy)*im.width+sx)*4,4,
                crop.rgba.begin()+(std::size_t(y)*crop.width+x)*4);
  }
  return crop;
}
app::SceneOptions route() {
  app::SceneOptions o; o.drawCellRoute=true; o.drawWall=false; o.drawOuter=false; o.drawInner=false; return o;
}
app::Framebuffer render(const app::TubeScene& scene,const AssetLibrary& lib,
                        const app::CameraBasis& camera,const app::SceneOptions& options) {
  app::Raster r(camera.width,camera.height); r.clear(19,23,31); scene.render(r,camera,options,&lib); return r.framebuffer();
}

}
int main(int argc, char** argv) {
  if(argc<3) return 2;
  const bool before=std::string(argv[1])=="before";
  const fs::path out=argv[2], baseline=argc>3&&std::string(argv[3])!="-"?fs::path(argv[3]):fs::path{};
  const char* gameEnv=std::getenv("WEBOLUTION_GAME_DIR"); if(!gameEnv)return 2;
  const fs::path game=gameEnv;const auto local=save::localAppDataDirectory();if(!local)return 2;
  SaveSnapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  fs::create_directories(out);
  std::ofstream trace(out/"rotation.csv");
  trace<<std::setprecision(17)<<"route,view,yaw_units,x,y,state,depth,raw_pixels,expected_pixels,actual_pixels,leaked_pixels,missing_pixels,box,bias_extra1,bias_extra2,bias_extra4,bias_extra8\n";
  int leakViews=0,missingViews=0,partialViews=0,hiddenViews=0,visibleViews=0;
  std::size_t countdownPassed=0,countdownRejected=0;
  for(const std::string name:{"climber","ledge","pillar","tunnel"}) {
    const std::string level=(name=="climber"||name=="ledge")?"MM_CLIMB_002":"MM_BASH_002";
    auto setup=sim::loadRetailLevel(*lib,level);check(bool(setup),"load "+level);if(!setup)continue;
    sim::World world(*setup);std::string path;
    for(const auto& p:levelFileNames(*lib))if(p.find(level)!=std::string::npos){path=p;break;}
    auto blob=lib->read(path);auto mesh=findCylinderMesh(*blob);auto index=indexLevel(*blob);
    auto scene=app::TubeScene::build(*lib,*mesh,*blob,index->grid0);check(bool(scene),"build "+level);if(!scene)continue;
    app::LemmingAnimTracker anim;bool assigned=false,ready=false;int bashStart=0;
    for(int n=0;n<1800;++n) {
      world.advanceElapsedMs(100);anim.advance(world,*scene->lemmingSprites(),0.1);
      scene->syncTerrain(world.terrain());
      if(world.lemmings().empty())continue;
      const auto& l=world.lemmings()[0];
      if(name=="ledge"&&n==59){ready=true;break;}
      if(name=="climber") {
        if(!assigned&&l.state==sim::State::Walker)assigned=world.assign(0,sim::Skill::Climber);
        if(l.state==sim::State::Climber&&l.y<60){ready=true;break;}
      }
      if(name=="pillar"||name=="tunnel") {
        bool tall=true;
        for(int dy=0;dy<8;++dy){auto mat=world.terrain().at(world.terrain().wrapX(l.x+3*int(l.facing)),l.y-dy);
          tall &= sim::isSolid(mat)&&mat!=sim::Material::Steel;}
        if(!assigned&&l.state==sim::State::Walker&&tall) {
          if(name=="pillar"){ready=true;break;}
          bashStart=l.x;assigned=world.assign(0,sim::Skill::Basher);
        }
        if(assigned&&l.state==sim::State::Basher&&std::abs(l.x-bashStart)>=5){ready=true;break;}
      }
    }
    check(ready,"natural route snapshot "+name);if(!ready)continue;
    if(name=="tunnel")check(scene->terrainCellsChanged()>0,"tunnel mesh follows excavation");
    const auto actors=app::injectedWalkersFromWorld(world,world.terrain().height(),&anim);
    check(!actors.empty(),"released actor");if(actors.empty())continue;
    fs::create_directories(out/name);
    const auto& sample=actors[0];const auto& live=world.lemmings()[0];
    std::ostringstream state;
    state<<std::setprecision(17)<<"level="<<level<<"\nclock_ms="<<world.levelClockMs()
      <<"\nx="<<live.x<<"\ny="<<live.y<<"\nfacing="<<int(live.facing)<<"\nstate="<<sim::stateName(live.state)
      <<"\nclip="<<sample.clip<<"\ntemporal="<<sample.walkTemporalIndex
      <<"\nplus8="<<sample.plus8X<<','<<sample.plus8Y<<','<<sample.plus8Z
      <<"\nterrain_cells_changed="<<scene->terrainCellsChanged()<<'\n';
    std::ofstream(out/name/"state.txt",std::ios::binary)<<state.str();
    if(!baseline.empty())check(bytes(baseline/name/"state.txt")==state.str(),"paired simulation snapshot identical "+name);
    Image sheet;sheet.width=1200;sheet.height=460;
    sheet.rgba.assign(std::size_t(sheet.width)*sheet.height*4,35);
    for(std::size_t i=3;i<sheet.rgba.size();i+=4)sheet.rgba[i]=255;
    auto reviewFont=loadTexture(*lib,"TEXTURES\\^Font.bmp");
    for(int view=0;view<48;++view) {
      app::CameraParams cp;cp.yawUnits=actors[0].plus8X+view*256.0/48;
      const auto camera=app::buildCamera(cp);auto opt=route();
      app::Raster empty(640,480);empty.clear(19,23,31);scene->render(empty,camera,opt,&*lib);
      const auto sceneDepth=empty.mutableDepthBuffer();
      opt.injectedWalkers={actors[0]};auto rawOpt=opt;rawOpt.composeSkipActorVisibility=true;
      const auto raw=render(*scene,*lib,camera,rawOpt);
      const auto rawBoxes=scene->injectedWalkerScreenBoxes(camera,rawOpt,*lib);
      check(bool(rawBoxes[0]),"raw placement exists");if(!rawBoxes[0])continue;
      const auto placement=*rawBoxes[0];const double z=placement.depth*640/0.3-4.0;
      check(app::actorPhysicalDepth(placement,640)==z,"physical Z conversion and provisional contact bias");
      // Independent pixel oracle: unmasked production sprite + actual scene Z.
      const bool gate=app::actorSubmissionVisible(actors[0].plus8X,actors[0].plus8Z,true,
          app::tubePassBounds(camera.selectorTurns,scene->collisionGrid().cylinderExtent));
      auto expected=empty.framebuffer();int rawN=0,expectedN=0;std::array<int,4> biased{};
      for(std::size_t at=0;at<sceneDepth.size();++at) {
        const auto i=at*3;
        if(!std::equal(raw.rgb.begin()+i,raw.rgb.begin()+i+3,expected.rgb.begin()+i)) {
          ++rawN;
          for(int k=0;k<4;++k)biased[k]+=gate&&z-(1<<k)<sceneDepth[at];
          if(gate&&z<sceneDepth[at]){std::copy_n(raw.rgb.begin()+i,3,expected.rgb.begin()+i);++expectedN;}
        }
      }
      if(view==0)check(expectedN==rawN,"front contact pose stays intact "+name);
      if(name=="climber"&&(view==36||view==40||view==44))
        check(expectedN==rawN,"wall-contact climber intact at exposed angles");
      const auto actual=render(*scene,*lib,camera,opt);
      const auto boxes=scene->injectedWalkerScreenBoxes(camera,opt,*lib);
#ifndef ACTOR_DEPTH_BEFORE
      if(name=="climber"&&view==0) {
        // Supplemental final-scene-depth contract: this is the public pass
        // main.cpp uses after secondary scenery/fluids, on the real level.
        auto finalScene=empty;
        scene->drawInjectedWalkers(finalScene,camera,opt,*lib);
        check(finalScene.framebuffer().rgb==actual.rgb,"standalone and production final actor passes agree");
        check(finalScene.depthBuffer()==empty.depthBuffer(),"actor pass preserves scene depth");
        // A synthetic nearer vertical mask represents a later scenery pass;
        // it supplements, rather than replaces, natural wall/tunnel views.
        auto partialScene=empty;
        const int middle=int((placement.left+placement.right)/2);
        for(int y=0;y<camera.height;++y)for(int x=0;x<middle;++x)
          partialScene.mutableDepthBuffer()[std::size_t(y)*camera.width+x]=z-1;
        scene->drawInjectedWalkers(partialScene,camera,opt,*lib);
        const auto partialBox=scene->injectedWalkerScreenBoxes(camera,opt,*lib,partialScene);
        check(partialBox[0].has_value(),"partially visible actor pickable against final scenery depth");
        check(changed(empty.framebuffer(),partialScene.framebuffer())>0&&
              changed(empty.framebuffer(),partialScene.framebuffer())<rawN,"later scenery masks some actor pixels");
        png(out/name/"supplement-final-scenery-partial.png",image(partialScene.framebuffer()));
        auto hiddenScene=empty;
        std::fill(hiddenScene.mutableDepthBuffer().begin(),hiddenScene.mutableDepthBuffer().end(),z-1);
        scene->drawInjectedWalkers(hiddenScene,camera,opt,*lib);
        check(hiddenScene.framebuffer().rgb==empty.framebuffer().rgb,"final scenery fully hides actor");
        check(!scene->injectedWalkerScreenBoxes(camera,opt,*lib,hiddenScene)[0],"fully hidden actor not pickable against final scenery depth");
        png(out/name/"supplement-final-scenery-hidden.png",image(hiddenScene.framebuffer()));
      }
#endif
      int leaked=0,missing=0;
      for(std::size_t at=0;at<sceneDepth.size();++at) {
        const auto i=at*3;const bool actualPixel=!std::equal(actual.rgb.begin()+i,actual.rgb.begin()+i+3,empty.framebuffer().rgb.begin()+i);
        const bool expectedPixel=!std::equal(expected.rgb.begin()+i,expected.rgb.begin()+i+3,empty.framebuffer().rgb.begin()+i);
        leaked+=actualPixel&&!expectedPixel;missing+=!actualPixel&&expectedPixel;
      }
      leakViews+=leaked>0;missingViews+=missing>0;
      partialViews+=expectedN>0&&expectedN<rawN&&gate;hiddenViews+=expectedN==0;visibleViews+=expectedN==rawN&&rawN>0;
      if(!before) {
        check(actual.rgb==expected.rgb,"per-pixel scene occlusion "+name+" view "+std::to_string(view));
        check(bool(boxes[0])==(expectedN>0),"picking visibility "+name);
      }
      const auto stem="view-"+std::to_string(view);
      png(out/name/(stem+".png"),image(actual));
      png(out/name/(stem+"-crop.png"),enlargedCrop(image(actual),int(placement.left)-4,int(placement.top)-4,
          int(placement.right-placement.left)+9,int(placement.bottom-placement.top)+9));png(out/name/(stem+"-expected.png"),image(expected));
      if(view%4==0) {
        const auto crop=enlargedCrop(image(actual),int(placement.left)-4,int(placement.top)-4,
            int(placement.right-placement.left)+9,int(placement.bottom-placement.top)+9);
        const int slot=view/4,ox=(slot%6)*200,oy=(slot/6)*230;
        for(int y=0;y<std::min(crop.height,200);++y)for(int x=0;x<std::min(crop.width,200);++x)
          std::copy_n(crop.rgba.begin()+(std::size_t(y)*crop.width+x)*4,4,
              sheet.rgba.begin()+(std::size_t(oy+30+y)*sheet.width+ox+x)*4);
        if(reviewFont) {
          shell::HudLine label;label.text=std::to_string(view);label.x=ox+4;label.y=oy;
          label.colour={255,255,255};shell::drawHud(sheet,*reviewFont,std::span<const shell::HudLine>(&label,1),shell::fontCells());
        }
      }
      // Empty frames must match the authentic task-entry renderer byte-for-byte.
      if(before){auto encoded=encodePng(image(empty.framebuffer()));std::ofstream f(out/name/(stem+"-empty.png"),std::ios::binary);f.write(reinterpret_cast<const char*>(encoded.data()),encoded.size());}
      else if(!baseline.empty())check(bytes(baseline/name/(stem+"-empty.png"))==[&]{auto e=encodePng(image(empty.framebuffer()));return std::string(e.begin(),e.end());}(),"no-actor frame unchanged");
      const auto& l=world.lemmings()[0];
      trace<<name<<','<<view<<','<<cp.yawUnits<<','<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','<<z<<','<<rawN<<','<<expectedN<<','<<changed(empty.framebuffer(),actual)<<','<<leaked<<','<<missing<<','<<bool(boxes[0]);for(auto n:biased)trace<<','<<n;trace<<'\n';
      // Countdown fragments independently obey scene Z even for partial actors.
      if(view%4==0&&boxes[0]) {
        auto font=loadTexture(*lib,"TEXTURES\\^Font.bmp");
        if(font) {
          auto fuseSetup=sim::loadRetailLevel(*lib,"MM_BOMB");sim::World fuse(*fuseSetup);for(int n=0;n<60;++n)fuse.advanceElapsedMs(100);
          check(fuse.assign(0,sim::Skill::Bomber),"light countdown fixture fuse");
          std::vector<app::LevelActorView> views{{0,placement}};auto glyph=image(actual);const auto original=glyph;auto unmasked=glyph;
          app::drawBomberCountdowns(unmasked,*font,fuse,views);
          app::drawBomberCountdowns(glyph,*font,fuse,views,sceneDepth);
          png(out/name/(stem+"-countdown.png"),glyph);
          for(std::size_t at=0;at<sceneDepth.size();++at) {
            if(!std::equal(unmasked.rgba.begin()+at*4,unmasked.rgba.begin()+at*4+4,original.rgba.begin()+at*4)) {
              if(z<sceneDepth[at])++countdownPassed;else ++countdownRejected;
            }
            for(int c=0;c<4;++c)
              check(glyph.rgba[at*4+c]==(z<sceneDepth[at]?unmasked.rgba[at*4+c]:original.rgba[at*4+c]),"countdown depth");
          }
        }
      }
    }
    png(out/name/"review-12-views.png",sheet);
  }
  check(partialViews>0&&hiddenViews>0&&visibleViews>0,"partial, fully hidden and fully visible real-level cases");
  if(before)check(leakViews>0&&missingViews>0,"reproduce both reported failures");
  check(countdownPassed>0&&countdownRejected>0,"real scene both admits and rejects countdown pixels");
  check(installed.unchanged()&&normal.unchanged(),"saves untouched");
  std::ofstream(out/"summary.txt")<<"before="<<before<<"\nleak_views="<<leakViews<<"\nmissing_views="<<missingViews<<"\npartial_views="<<partialViews<<"\nhidden_views="<<hiddenViews<<"\nfully_visible_views="<<visibleViews<<"\ncountdown_passed_pixels="<<countdownPassed<<"\ncountdown_rejected_pixels="<<countdownRejected<<"\nfailures="<<failures<<'\n';
  std::cout<<"actor-depth "<<argv[1]<<": "<<failures<<" failures\n";return failures?1:0;
}
