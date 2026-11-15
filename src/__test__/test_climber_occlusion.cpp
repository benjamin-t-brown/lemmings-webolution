#include "app/AppEntry.h"
#include "app/BackdropScene.h"
#include "app/SceneFrame.h"
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
app::Raster renderScene(const app::TubeScene& scene, const app::BackdropScene* backdrop,
    const AssetLibrary& lib,const app::CameraBasis& camera,const app::SceneOptions& options) {
  app::Raster r(camera.width,camera.height);r.clear(19,23,31);
  auto geometry=options;geometry.injectedWalkers.clear();app::BackdropStats stats;
  const auto stage=[&](app::BackdropScene::Stage s){if(backdrop)backdrop->renderStage(r,camera,lib,stats,s);};
  app::scheduleSceneFrame([&]{stage(app::BackdropScene::Stage::Primary);},
    [&]{stage(app::BackdropScene::Stage::LegacyUnattributed);},
    [&]{scene.render(r,camera,geometry,&lib);},
    [&]{stage(app::BackdropScene::Stage::FluidGate);},
    [&]{stage(app::BackdropScene::Stage::Secondary);});
  scene.drawInjectedWalkers(r,camera,options,lib);return r;
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
  trace<<std::setprecision(17)<<"route,view,yaw_units,x,y,state,actor_world_z,normalized_bias,contact_allowance,comparison_z,toward_wall,raw_pixels,expected_pixels,actual_pixels,leaked_pixels,missing_pixels,box\n";
  int leakViews=0,missingViews=0,partialViews=0,hiddenViews=0,visibleViews=0;
  int changedClimberViews=0, unchangedControlViews=0;
  std::ofstream pixels(out/"pixels.csv");pixels<<std::setprecision(17)<<"route,view,screen_x,screen_y,actor_world_z,normalized_bias,contact_allowance,comparison_z,scene_z,drawn\n";
  for(const std::string name:{"climber","climber2","ledge","pillar","tunnel"}) {
    const std::string level=name=="climber2"?"MM_FLOAT_CLIMB2":(name=="climber"||name=="ledge")?"MM_CLIMB_002":"MM_BASH_002";
    auto setup=sim::loadRetailLevel(*lib,level);check(bool(setup),"load "+level);if(!setup)continue;
    sim::World world(*setup);std::string path;
    for(const auto& p:levelFileNames(*lib))if(p.find(level)!=std::string::npos){path=p;break;}
    auto blob=lib->read(path);auto mesh=findCylinderMesh(*blob);auto index=indexLevel(*blob);
    auto scene=app::TubeScene::build(*lib,*mesh,*blob,index->grid0);check(bool(scene),"build "+level);if(!scene)continue;
    app::LemmingAnimTracker anim;bool assigned=false,ready=false;int bashStart=0, climbStart=-1;
    for(int n=0;n<1800;++n) {
      world.advanceElapsedMs(100);anim.advance(world,*scene->lemmingSprites(),0.1);
      scene->syncTerrain(world.terrain());
      if(world.lemmings().empty())continue;
      const auto& l=world.lemmings()[0];
      if(name=="ledge"&&n==59){ready=true;break;}
      if(name=="climber"||name=="climber2") {
        if(!assigned&&l.state==sim::State::Walker)assigned=world.assign(0,sim::Skill::Climber);
        if(l.state==sim::State::Climber) { if(climbStart<0)climbStart=l.y; if(l.y<climbStart-4){ready=true;break;} }
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
    auto backdrop=app::BackdropScene::build(*lib,*blob,level);check(bool(backdrop),"full production scenery "+level);
    if(backdrop) {
      if(auto effects=backdrop->entranceExitPresentation(*blob))backdrop->setEntranceExitPresentation(*effects,1.0,0.0);
      std::vector<app::BackdropScene::FluidGateState> gates;
      for(const auto& g:world.setup().gates)gates.push_back({g.lane,g.height,g.wavePhase,g.animated,(world.levelClockMs()%4000)/1000.0});
      backdrop->setFluidState(std::move(gates));
    }
    const auto actors=app::injectedWalkersFromWorld(world,world.terrain().height(),&anim);
    check(!actors.empty(),"released actor");if(actors.empty())continue;
    for(bool fullScene:{false,true}) {
    const std::string evidenceName=name+(fullScene?"-full-scene":"");
    const auto* background=fullScene&&backdrop?&*backdrop:nullptr;
    fs::create_directories(out/evidenceName);
    const auto& sample=actors[0];const auto& live=world.lemmings()[0];
    std::ostringstream state;
    state<<std::setprecision(17)<<"level="<<level<<"\nclock_ms="<<world.levelClockMs()
      <<"\nx="<<live.x<<"\ny="<<live.y<<"\nfacing="<<int(live.facing)<<"\nstate="<<sim::stateName(live.state)
      <<"\nclip="<<sample.clip<<"\ntemporal="<<sample.walkTemporalIndex
      <<"\nplus8="<<sample.plus8X<<','<<sample.plus8Y<<','<<sample.plus8Z
      <<"\nterrain_cells_changed="<<scene->terrainCellsChanged()<<'\n';
    std::ofstream(out/evidenceName/"state.txt",std::ios::binary)<<state.str();
    if(!baseline.empty())check(bytes(baseline/evidenceName/"state.txt")==state.str(),"paired simulation snapshot identical "+name);
    Image sheet;sheet.width=1200;sheet.height=460;
    sheet.rgba.assign(std::size_t(sheet.width)*sheet.height*4,35);
    for(std::size_t i=3;i<sheet.rgba.size();i+=4)sheet.rgba[i]=255;
    std::array<Image,4> reviewPages;
    for(auto& page:reviewPages)page=sheet;
    auto reviewFont=loadTexture(*lib,"TEXTURES\\^Font.bmp");
    for(int view=0;view<48;++view) {
      app::CameraParams cp;cp.yawUnits=actors[0].plus8X+view*256.0/48;
      const auto camera=app::buildCamera(cp);auto opt=route();
      auto empty=renderScene(*scene,background,*lib,camera,opt);
      const auto sceneDepth=empty.mutableDepthBuffer();
      opt.injectedWalkers={actors[0]};auto rawOpt=opt;rawOpt.composeSkipActorVisibility=true;
      const auto raw=renderScene(*scene,background,*lib,camera,rawOpt).framebuffer();
      const auto rawBoxes=scene->injectedWalkerScreenBoxes(camera,rawOpt,*lib);
      check(bool(rawBoxes[0]),"raw placement exists");if(!rawBoxes[0])continue;
      const auto placement=*rawBoxes[0];
      ActorPoseLevel pl;pl.gridHeight=mesh->faceCount;pl.grid0=index->grid0;pl.surface=mesh->surface;pl.cylinderTemplate=&*mesh;
      const auto pose=actorPoseFromCylinderSample(pl,{sample.plus8X,sample.plus8Y,sample.plus8Z},sample.facing);
      double towardWall=0;for(int k=0;k<3;++k)towardWall+=pose.basis[k]*(camera.eyeWorld[k]-pose.modelPosition[k]);
      const bool back=(name=="climber"||name=="climber2")&&towardWall>0;
      const double baseZ=placement.depth*640/0.3, allowance=back?0.0:4.0, z=baseZ-allowance;
      const auto& m=camera.worldToView;
      const double worldZ=m[2]*pose.modelPosition[0]+m[5]*pose.modelPosition[1]+m[8]*pose.modelPosition[2]+m[11];
      check(std::abs(worldZ-baseZ-0.64)<1e-10,"traced normalized bias is 0.64 world units at width 640");
      // Independent pixel oracle: unmasked production sprite + actual scene Z.
      const bool gate=app::actorSubmissionVisible(actors[0].plus8X,actors[0].plus8Z,true,
          app::tubePassBounds(camera.selectorTurns,scene->collisionGrid().cylinderExtent));
      auto expected=empty.framebuffer();int rawN=0,expectedN=0;
      for(std::size_t at=0;at<sceneDepth.size();++at) {
        const auto i=at*3;
        if(!std::equal(raw.rgb.begin()+i,raw.rgb.begin()+i+3,expected.rgb.begin()+i)) {
          ++rawN;
          if(gate&&z<sceneDepth[at]){std::copy_n(raw.rgb.begin()+i,3,expected.rgb.begin()+i);++expectedN;}
        }
      }

      const auto actualRaster=renderScene(*scene,background,*lib,camera,opt);const auto& actual=actualRaster.framebuffer();
      const auto boxes=scene->injectedWalkerScreenBoxes(camera,opt,*lib,empty);
      check(actualRaster.depthBuffer()==empty.depthBuffer(),"actor composition preserves scene depth");

      if(name=="climber"||name=="climber2") {
        // At 30 degrees into the wall's back and at the exposed 300-degree
        // view these installed faces fill the actor rectangle. These explicit
        // requirements don't derive their expected answer from the bias rule.
        if(view==4) {
          if(before)check(changed(empty.framebuffer(),actual)>0&&bool(boxes[0]),"reproduce back-of-wall leak and pickability "+evidenceName);
          else check(changed(empty.framebuffer(),actual)==0&&!boxes[0],"back-of-wall climber has no pixels or pick box "+evidenceName);
        }
        if(view==40)check(actual.rgb==raw.rgb&&bool(boxes[0]),"front-of-wall climber fully drawn and pickable "+evidenceName);
      }
      const auto actualPng=encodePng(image(actual));
      if(!baseline.empty()) {
        const auto prior=bytes(baseline/evidenceName/("view-"+std::to_string(view)+".png"));
        check(!prior.empty(),"authentic paired full frame exists");
        const bool same=prior==std::string(actualPng.begin(),actualPng.end());
        if(name=="ledge"||name=="pillar"||name=="tunnel") {check(same,"no-climber frame byte-identical "+evidenceName);++unchangedControlViews;}
        else changedClimberViews+=!same;
      }
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
      png(out/evidenceName/(stem+".png"),image(actual));
      png(out/evidenceName/(stem+"-crop.png"),enlargedCrop(image(actual),int(placement.left)-4,int(placement.top)-4,
          int(placement.right-placement.left)+9,int(placement.bottom-placement.top)+9));png(out/evidenceName/(stem+"-expected.png"),image(expected));
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
      {
        auto& page=reviewPages[view/12];
        const auto crop=enlargedCrop(image(actual),int(placement.left)-4,int(placement.top)-4,
            int(placement.right-placement.left)+9,int(placement.bottom-placement.top)+9);
        const int slot=view%12,ox=(slot%6)*200,oy=(slot/6)*230;
        for(int y=0;y<std::min(crop.height,200);++y)for(int x=0;x<std::min(crop.width,200);++x)
          std::copy_n(crop.rgba.begin()+(std::size_t(y)*crop.width+x)*4,4,
              page.rgba.begin()+(std::size_t(oy+30+y)*page.width+ox+x)*4);
        if(reviewFont){shell::HudLine label;label.text=std::to_string(view);label.x=ox+4;label.y=oy;label.colour={255,255,255};
          shell::drawHud(page,*reviewFont,std::span<const shell::HudLine>(&label,1),shell::fontCells());}
      }
      // Empty frames must match the authentic task-entry renderer byte-for-byte.
      if(before){auto encoded=encodePng(image(empty.framebuffer()));std::ofstream f(out/evidenceName/(stem+"-empty.png"),std::ios::binary);f.write(reinterpret_cast<const char*>(encoded.data()),encoded.size());}
      else if(!baseline.empty())check(bytes(baseline/evidenceName/(stem+"-empty.png"))==[&]{auto e=encodePng(image(empty.framebuffer()));return std::string(e.begin(),e.end());}(),"no-actor frame unchanged");
      for(std::size_t at=0;at<sceneDepth.size();++at)if(!std::equal(raw.rgb.begin()+at*3,raw.rgb.begin()+at*3+3,empty.framebuffer().rgb.begin()+at*3))
        pixels<<evidenceName<<','<<view<<','<<at%640<<','<<at/640<<','<<worldZ<<",0.64,"<<(before?4.0:allowance)<<','<<(before?baseZ-4:z)<<','<<sceneDepth[at]<<','<<!std::equal(actual.rgb.begin()+at*3,actual.rgb.begin()+at*3+3,empty.framebuffer().rgb.begin()+at*3)<<'\n';
      const auto& l=world.lemmings()[0];
      trace<<evidenceName<<','<<view<<','<<cp.yawUnits<<','<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','<<worldZ<<",0.64,"<<(before?4.0:allowance)<<','<<(before?baseZ-4:z)<<','<<towardWall<<','<<rawN<<','<<expectedN<<','<<changed(empty.framebuffer(),actual)<<','<<leaked<<','<<missing<<','<<bool(boxes[0])<<'\n';

    }
    png(out/evidenceName/"review-12-views.png",sheet);
    for(int page=0;page<4;++page)png(out/evidenceName/("review-page-"+std::to_string(page)+".png"),reviewPages[page]);
    }
  }
  if(before)check(leakViews>0,"back-of-wall pixels violate corrected contract before fix");
  else if(!baseline.empty())check(changedClimberViews>0&&unchangedControlViews==288,"changed climbers and all 288 unchanged non-climber frames");
  check(partialViews>0&&hiddenViews>0&&visibleViews>0,"partial, fully hidden and fully visible real-level cases");

  check(installed.unchanged()&&normal.unchanged(),"saves untouched");
  std::ofstream(out/"summary.txt")<<"before="<<before<<"\nleak_views="<<leakViews<<"\nmissing_views="<<missingViews<<"\npartial_views="<<partialViews<<"\nhidden_views="<<hiddenViews<<"\nfully_visible_views="<<visibleViews<<"\nchanged_climber_views="<<changedClimberViews<<"\nunchanged_control_views="<<unchangedControlViews<<"\nfailures="<<failures<<'\n';
  std::cout<<"climber-occlusion "<<argv[1]<<": "<<failures<<" failures\n";return failures?1:0;
}
