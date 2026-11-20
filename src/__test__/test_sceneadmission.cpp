// Check installed scenery admission and render a subset with a static camera.
#include "app/BackdropScene.h"
#include "app/SceneFrame.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Bitmap.h"
#include "assets/Png.h"
#include "sceneadmission_census.hpp"
#include "app/AppEntry.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

namespace fs=std::filesystem;
using namespace webo;
using namespace webo::app;
using Bytes=std::vector<std::byte>;
using Clock=std::chrono::steady_clock;
namespace {
void require(bool pass,const std::string& message) {
  if(!pass)throw std::runtime_error(message);
}
double seconds(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double>(b-a).count();}
void binary(const fs::path& path,const void* data,std::size_t size) {
  std::ofstream out(path,std::ios::binary);
  out.write(static_cast<const char*>(data),static_cast<std::streamsize>(size));
  require(bool(out),"write "+path.string());
}
std::vector<char> read(const fs::path& path) {
  std::ifstream in(path,std::ios::binary);
  require(bool(in),"read "+path.string());
  return {std::istreambuf_iterator<char>(in),{}};
}
Image image(const Raster& raster) {
  const auto& f=raster.framebuffer();Image im;im.width=f.width;im.height=f.height;
  for(std::size_t i=0;i<f.rgb.size();i+=3)im.rgba.insert(im.rgba.end(),{f.rgb[i],f.rgb[i+1],f.rgb[i+2],255});
  return im;
}
std::size_t changed(const std::vector<std::uint8_t>& a,const std::vector<std::uint8_t>& b,std::size_t stride=3) {
  require(a.size()==b.size(),"equal pixel storage");std::size_t n=0;
  for(std::size_t i=0;i<a.size();i+=stride)
    if(!std::equal(a.begin()+i,a.begin()+i+3,b.begin()+i))++n;
  return n;
}
void artifact(const fs::path& dir,const std::string& name,Raster& raster,bool raw) {
  require(bool(writePng(dir/(name+".png"),image(raster))),"PNG "+name);
  if(!raw)return;
  const auto& rgb=raster.framebuffer().rgb;
  binary(dir/(name+".rgb"),rgb.data(),rgb.size());
  const auto& depth=raster.mutableDepthBuffer();
  binary(dir/(name+".depth"),depth.data(),depth.size()*sizeof(double));
}
std::uint64_t hash(Raster& raster) {
  std::uint64_t h=1469598103934665603ull;
  const auto add=[&](std::uint8_t v){h=(h^v)*1099511628211ull;};
  for(auto v:raster.framebuffer().rgb)add(v);
  for(double d:raster.mutableDepthBuffer()){
    auto bits=std::bit_cast<std::uint64_t>(d);for(int i=0;i<8;++i)add(bits>>(8*i));
  }
  return h;
}
constexpr std::array<BackdropScene::Stage,3> stages={BackdropScene::Stage::Primary,BackdropScene::Stage::LegacyUnattributed,BackdropScene::Stage::Secondary};
constexpr std::array<const char*,3> stageNames={"Primary","LegacyUnattributed","Secondary"};
struct Rendered {
  Raster raster{640,480};
  std::array<BackdropStats,3> stats{};
  std::array<std::size_t,3> writes{},depthWrites{},changedColors{},candidates{};
};
Rendered render(const BackdropScene& backdrop,const TubeScene* tube,const AssetLibrary& lib,
                const CameraBasis& cam,bool scenery,bool compose,std::ostream* log) {
  Rendered out;out.raster.clear(0,0,0);
  std::vector<std::uint8_t> mask(640*480,1);
  SceneOptions options;options.drawCellRoute=true;options.drawWall=false;
  options.drawOuter=false;options.drawInner=false;
  const auto stage=[&](std::size_t i){
    if(!scenery)return;
    SceneRasterObserver observer;observer.mask=&mask;
    observer.candidate=[&](auto,auto,auto,auto){++out.candidates[i];};
    observer.accepted=[&](const SceneRasterWriteEvent& e){
      ++out.writes[i];out.depthWrites[i]+=e.depthAssigned;
      out.changedColors[i]+=e.colorBefore!=e.colorAfter;
    };
    backdrop.renderStage(out.raster,cam,lib,out.stats[i],stages[i],&observer);
  };
  scheduleSceneFrame([&]{stage(0);},[&]{stage(1);},[&]{if(compose&&tube)tube->render(out.raster,cam,options,&lib);},[&]{stage(2);});
  for(std::size_t i=0;i<3;++i){
    const auto& s=out.stats[i];
    if(log){
      *log<<"  "<<stageNames[i]<<",candidates="<<s.stageMeshRecords<<'/'<<s.stageBillboards
          <<",rasterCalls="<<s.rasterCalls<<",successfulRasterReturns="<<s.meshTriangles
          <<",acceptedColorStores="<<out.writes[i]<<",acceptedDepthStores="<<out.depthWrites[i]
          <<",changedColorStores="<<out.changedColors[i]<<",predicateCulls="<<s.predicateRejected
          <<",missingTextures="<<s.missingTextures<<",rejected="<<s.rejected
          <<",projectionRejected="<<s.projectionRejected<<",rasterRejected="<<s.rasterRejected<<'\n';
      for(const auto& d:s.diagnostics)*log<<"  diagnostic,"<<d<<'\n';
    }
    require(s.rejected==s.projectionRejected+s.rasterRejected,"rejection attribution");
    require(s.acceptedColorWrites==out.writes[i]&&s.acceptedDepthWrites==out.depthWrites[i],"accepted-event counters");
  }
  if(log)*log<<"  tubeCalls="<<out.raster.drawnTriangles()<<",tubeAcceptedPixels="<<out.raster.drawnPixels()<<'\n';
  return out;
}
std::string idOf(const std::string& name){auto s=name.find_last_of("\\/");auto base=s==std::string::npos?name:name.substr(s+1);auto dot=base.rfind('.');return dot==std::string::npos?base:base.substr(0,dot);}
bool isAcceptedPair(const std::string& id){return id=="MM_CLIMB_002"||id=="MM_BASH_002";}
void application(const std::string& gameDir,const std::string& level,const fs::path& dir,std::ostream& log) {
  require(std::string(std::getenv("SDL_VIDEODRIVER")?std::getenv("SDL_VIDEODRIVER"):"")=="dummy","dummy video required");
  require(std::string(std::getenv("SDL_AUDIODRIVER")?std::getenv("SDL_AUDIODRIVER"):"")=="dummy","dummy audio required");
  const auto cameraFile=dir/"fixed-camera.json";std::ofstream(cameraFile)<<R"({"viewport":[640,480],"center":[319,239],"aspect":1.3333333333333333,"angle_degrees":19.03,"yaw_units":32,"yaw_offset":128,"yaw_units_per_turn":256,"zoom_shift":0,"level_eye_height":96,"height_bias":17.392,"eye_distance":377.516,"fixed_pitch_turns":-0.03135770272046,"pose_pi":3.1415927410125732,"lens_pi":3.1415927410125732,"angle_units_per_turn":360,"elevation":0})"<<'\n';
  std::array<Image,2> frames;
  for(int off=0;off<2;++off){
    std::vector<std::string> args={"scene-admission","--game-dir",gameDir,"--level",level,"--sim","--native","--geom","route","--camera",cameraFile.string(),"--no-audio"};
    if(off)args.emplace_back("--scene-off");
    std::vector<char*> argv;for(auto& a:args)argv.push_back(a.data());
    int callbacks=0;bool queued=false;
    const auto observer=[&](const LevelFrameView& view){
      ++callbacks;require(view.levelName==level&&view.frame==0,"public level route/frame");frames[off]=view.image;
      SDL_Event quit{};quit.type=SDL_QUIT;queued=SDL_PushEvent(&quit)==1;
    };
    const auto result=runApplication(static_cast<int>(argv.size()),argv.data(),{},observer,{});
    require(result==0&&queued&&callbacks==1,"public --sim route exits normally through queued SDL_QUIT");
    require(bool(writePng(dir/(std::string("application-")+level+(off?"-scene-off.png":"-scene-on.png")),frames[off])),"public frame artifact");
    log<<"application,level="<<level<<",sceneOff="<<off<<",return="<<result<<",frames="<<callbacks<<",queuedQuit="<<queued<<'\n';
  }
  const auto pixels=changed(frames[0].rgba,frames[1].rgba,4);require(pixels>0,"public route scenery changes rendered pixels");
  log<<"applicationChangedPixels,"<<level<<'='<<pixels<<'\n';
}
}

int main()try {
  const char* gameDir=std::getenv("WEBOLUTION_GAME_DIR");require(gameDir&&*gameDir,"installed assets required");
  auto lib=AssetLibrary::open(fs::path(gameDir)/"lemmings.box",fs::path(gameDir)/"patch.box");require(bool(lib),"open installed archives");
  const std::string side="after";
  const fs::path root=SCENE_ADMISSION_DIR,dir=root/side;fs::create_directories(dir);
  std::ofstream log(dir/"measurements.txt");log<<std::setprecision(17);
  const auto cam=buildCamera(CameraParams{}); // same camera for all levels

  const auto names=levelFileNames(*lib);
  log<<"side="<<side<<" effectiveLevels="<<names.size()<<'\n';
  std::map<std::string,BackdropScene> admitted;std::map<std::string,Bytes> admittedBytes;
  std::map<std::string,std::size_t> emptyReasons;std::size_t emptyCount=0,hardErrors=0;
  const auto buildStart=Clock::now();
  for(const auto& name:names){
    const auto id=idOf(name);
    auto bytes=lib->read(name);require(bool(bytes),"read actual "+id);
    const auto outcome=sceneadmission::structuralOutcome(*bytes,&*lib);  // branch-1 sprite checks need the archive
    const auto t0=Clock::now();
    auto backdrop=BackdropScene::build(*lib,*bytes,id);
    const auto ms=seconds(t0,Clock::now())*1000;
    if(!backdrop){
      ++hardErrors;log<<"LEVEL "<<id<<" build=HARD_ERROR "<<backdrop.error()<<'\n';
      require(false,"no installed level may hard-error: "+id+": "+backdrop.error());
    }
    // Compare admission with the independent structural census.
    require(!outcome.malformed,"installed level is not malformed: "+id);
    require(backdrop->empty()==!outcome.supported,"production admission equals independent structural outcome: "+id);
    if(isAcceptedPair(id))require(!backdrop->empty()&&backdrop->diagnostics().empty(),"accepted level stays admitted without diagnostics");
    if(backdrop->empty()){
      ++emptyCount;
      require(!backdrop->diagnostics().empty(),"empty level carries a diagnostic: "+id);
      const auto& d=backdrop->diagnostics().front();
      require(d.find(id)!=std::string::npos&&d.find(outcome.role.substr(0,outcome.role.find(' ')))!=std::string::npos&&d.find("unsupported")!=std::string::npos&&d.find("empty partial scene")!=std::string::npos&&d.find("scenery adapter not demonstrated")==std::string::npos,"level-qualified role/form diagnostic: "+id+": "+d);
      ++emptyReasons[d.find("nested/non-leaf")!=std::string::npos?"Secondary: unsupported nested/non-leaf secondary dispatch":d.find("type-2 branch 1 sprite")!=std::string::npos?"unsupported type-2 branch 1 sprite (structural admission)":"other: "+d];
      log<<"LEVEL "<<id<<" admitted=0 buildMs="<<ms<<" diagnostic="<<d<<'\n';
    }else{
      log<<"LEVEL "<<id<<" admitted=1 buildMs="<<ms<<" roots="<<[&]{std::size_t n=0;backdrop->visitRoots([&](auto&,auto&){++n;});return n;}()<<'\n';
      admittedBytes[id]=std::move(*bytes);admitted.emplace(id,std::move(*backdrop));
    }
  }
  const auto buildSeconds=seconds(buildStart,Clock::now());
  log<<"admittedCount="<<admitted.size()<<" emptyCount="<<emptyCount<<" hardErrors="<<hardErrors<<" buildAllSeconds="<<buildSeconds<<'\n';
  for(const auto& [k,v]:emptyReasons)log<<"emptyReason \""<<k.substr(0,160)<<"\" levels="<<v<<'\n';
  log<<"admittedIds=";for(const auto& [id,_]:admitted)log<<id<<',';log<<'\n';
  require(admitted.size()>2,"current admits additional levels");
  // Check that the 18 nested cases admit and any omitted scenery has a diagnostic.
  require(admitted.size()+emptyCount==names.size()&&admitted.size()==102&&emptyCount==0,"scene-nested-impl-001: 102 admitted, no level remains an empty partial scene");

  double renderSeconds=0;std::size_t newlyRendered=0;
  for(const auto& [id,backdrop]:admitted){
    // Render the ten levels that share the 923 primary partition. The branch-1
    // render check covers the additional sprite cases.
    static const std::set<std::string> acceptedTen={"E06_KF_1","H17_MG_1","M06_KF_1","M08_KF_1","MM_BASH_002","MM_CLIMB_002","MM_E1_3_SKILLS","MM_E1_4SKILLS","MM_W_012","VH22_JC"};
    if(!acceptedTen.count(id))continue;
    const auto& bytes=admittedBytes.at(id);
    const bool pair=isAcceptedPair(id);
    const auto r0=Clock::now();
    auto mesh=findCylinderMesh(bytes);auto index=indexLevel(bytes);
    std::expected<TubeScene,std::string> tube=std::unexpected(std::string("no tube input"));
    if(mesh&&index)tube=TubeScene::build(*lib,*mesh,bytes,index->grid0,true);
    const bool haveTube=bool(tube);
    log<<"RENDER level="<<id<<" role="<<(pair?"accepted":"newly-admitted")<<" tube="<<(haveTube?"built":"unavailable: "+tube.error())<<'\n';
    require(haveTube,"production tube builds for admitted level "+id);
    if(haveTube)for(const auto& missing:tube->missingTiles())log<<"  tubeMissing,"<<missing<<'\n';
    log<<" scene-only\n";auto scene=render(backdrop,&*tube,*lib,cam,true,false,&log);artifact(dir,id+"-scene-only",scene.raster,pair);
    log<<" composed-scene-on\n";auto on=render(backdrop,&*tube,*lib,cam,true,true,&log);artifact(dir,id+"-scene-on",on.raster,pair);
    log<<" composed-scene-off\n";auto off=render(backdrop,&*tube,*lib,cam,false,true,&log);artifact(dir,id+"-scene-off",off.raster,pair);
    const auto changedPixels=changed(on.raster.framebuffer().rgb,off.raster.framebuffer().rgb);
    const auto sceneOnlyNonblack=[&]{std::size_t n=0;const auto& v=scene.raster.framebuffer().rgb;for(std::size_t i=0;i<v.size();i+=3)n+=(v[i]|v[i+1]|v[i+2])!=0;return n;}();
    // Determinism: independent second renders must hash identically.
    auto scene2=render(backdrop,&*tube,*lib,cam,true,false,nullptr);auto on2=render(backdrop,&*tube,*lib,cam,true,true,nullptr);
    const auto hScene=hash(scene.raster),hOn=hash(on.raster),hOff=hash(off.raster);
    require(hScene==hash(scene2.raster)&&hOn==hash(on2.raster),"deterministic scene-only and composed output for "+id);
    renderSeconds+=seconds(r0,Clock::now());
    const auto sceneryCalls=scene.stats[0].rasterCalls+scene.stats[2].rasterCalls,modelCalls=scene.stats[1].rasterCalls;
    const auto sceneryWrites=scene.writes[0]+scene.writes[2],sceneryDepth=scene.depthWrites[0]+scene.depthWrites[2];
    log<<" summary level="<<id<<" sceneOnlyHash=0x"<<std::hex<<hScene<<" composedOnHash=0x"<<hOn<<" composedOffHash=0x"<<hOff<<std::dec
       <<" sceneryRasterCalls="<<sceneryCalls<<" modelRasterCalls="<<modelCalls<<" sceneryColorStores="<<sceneryWrites<<" sceneryDepthStores="<<sceneryDepth
       <<" modelColorStores="<<scene.writes[1]<<" sceneOnlyNonblackPixels="<<sceneOnlyNonblack<<" composedChangedPixels="<<changedPixels
       <<" deterministic=1\n";
    // Count scenery separately from models.
    require(sceneryCalls>0&&sceneryWrites>0&&sceneryDepth>0,"nonzero scenery raster calls and accepted color/depth stores: "+id);
    require(changedPixels>0,"composed scenery changes pixels: "+id);
    const bool first=id=="MM_CLIMB_002",bash=id=="MM_BASH_002";
    if(first){
      Raster primary(640,480);primary.clear(0,0,0);BackdropStats st;backdrop.renderStage(primary,cam,*lib,st,stages[0]);
      require(hash(primary)==0x1fd0bfb82ab39e95ull,"preserve primary RGB/depth hash");
      require(scene.stats[0].stageMeshRecords==923&&scene.stats[0].stageBillboards==0&&scene.stats[1].stageMeshRecords==172&&scene.stats[1].stageBillboards==0&&scene.stats[2].stageMeshRecords==82&&scene.stats[2].stageBillboards==17,"retain stage candidate partitions 923/0 172/0 82/17");
    }
    if(first||bash){
      require(scene.stats[0].stageMeshRecords+scene.stats[1].stageMeshRecords+scene.stats[2].stageMeshRecords==1177&&scene.stats[2].stageBillboards==17,"1177 records and 17 billboards");
      require(scene.stats[0].stageMeshRecords==923&&scene.stats[1].stageMeshRecords==172&&scene.stats[2].stageMeshRecords==82,"923/172/82 partition for "+id);
    }
    if(!pair){
      ++newlyRendered;
      // The scenery contribution is distinguished from the model contribution by stage.
      require(scene.stats[0].stageMeshRecords==923,"newly admitted primary candidate partition 923: "+id);
    }
  }
  log<<"renderAllSeconds="<<renderSeconds<<" newlyRendered="<<newlyRendered<<'\n';
  require(newlyRendered>=3,"at least three newly admitted levels rendered");
  application(gameDir,"MM_W_012",dir,log);
  require(admitted.count("MM_W_012")==1,"public route level is newly admitted");
  log.close();const auto report=read(dir/"measurements.txt");std::cout.write(report.data(),static_cast<std::streamsize>(report.size()));
  std::cout<<"\nPASS scene-admission "<<side<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<"FAIL scene-admission: "<<e.what()<<'\n';return 1;}
