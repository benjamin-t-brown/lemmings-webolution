// Q2/Q3: real installed data and production renderer, source-built clone only.
#include "app/BackdropScene.h"
#include "app/AppEntry.h"
#include "app/TubeScene.h"
#include "app/ActorPose.h"
#include "app/ActorSprite.h"
#include "app/LemmingMotion.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "assets/LevelIndex.h"
#include "assets/Lrz.h"
#include "assets/Png.h"
#include "app/MapScene.h"
#include "save/SaveStore.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <set>
using namespace webo;
namespace fs=std::filesystem;
namespace {
int failures=0;
void check(bool ok,const std::string& why){if(!ok){++failures;std::cerr<<"FAIL: "<<why<<'\n';}}
[[maybe_unused]] bool near(double a,double b,double e=1e-7){return std::abs(a-b)<e;}
std::string file(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct SavedFile {
  fs::path p;bool existed;std::string data;
  explicit SavedFile(fs::path path):p(std::move(path)),existed(fs::exists(p)),data(file(p)){}
  bool unchanged()const{return existed==fs::exists(p)&&data==file(p);}
};
struct Shot {Image image;app::BackdropStats stats;std::vector<std::optional<app::ActorSpritePlacement>> actors;};
Shot shoot(app::BackdropScene& backdrop,const app::TubeScene& tube,const AssetLibrary& lib,
    const sim::World& world,const app::CameraBasis& camera,const app::LemmingMotionTracker* motion=nullptr){
  app::Raster raster(640,480);raster.clear(0,0,0);Shot result;
  std::vector<app::BackdropScene::FluidGateState> states;
  for(const auto& g:world.setup().gates){
#ifdef FLUID_KINDS_after
    states.push_back({g.lane,g.height,g.wavePhase,g.animated,(world.levelClockMs()%4000)/1000.0});
#else
    states.push_back({g.lane,g.height,g.wavePhase,g.animated});
#endif
  }
  backdrop.setFluidState(std::move(states));using S=app::BackdropScene::Stage;
  backdrop.renderStage(raster,camera,lib,result.stats,S::Primary);
  backdrop.renderStage(raster,camera,lib,result.stats,S::LegacyUnattributed);
  app::SceneOptions opts;opts.drawCellRoute=true;opts.drawWall=false;opts.drawOuter=false;opts.drawInner=false;
  if(motion){opts.injectedWalkers=app::injectedWalkersFromWorld(world,world.terrain().height(),nullptr,motion);
    if(opts.injectedWalkers.size()>1)opts.injectedWalkers.resize(1);
    result.actors=tube.injectedWalkerScreenBoxes(camera,opts,lib);}
  tube.render(raster,camera,opts,&lib);
  std::vector<std::uint8_t> mask(640*480,1);app::SceneRasterObserver observer;observer.mask=&mask;
  backdrop.renderStage(raster,camera,lib,result.stats,S::FluidGate,&observer);
  backdrop.renderStage(raster,camera,lib,result.stats,S::Secondary);
  result.image.width=640;result.image.height=480;const auto& rgb=raster.framebuffer().rgb;
  for(std::size_t i=0;i<rgb.size();i+=3)result.image.rgba.insert(result.image.rgba.end(),{rgb[i],rgb[i+1],rgb[i+2],255});
  return result;
}
void png(const fs::path& p,const Image& image){check(writePng(p,image).has_value(),"write "+p.string());}
// Independent original row/layer indexing over decoded world data (0x43D56C,
// 0x43E580 stride 0x600 and 0x43EA20 stride 0x300). Reader 0x43D09A
// deinterleaves FILE triples into those layers; this test reads file order.
std::array<double,3> originalPoint(const lemmings::WorldGeometryPrefix& w,ActorTerrainSample p){
  const int bx=int(p.x)/8,by=int(p.y)/16,last=int(w.leading_value)/16-1;
  const auto lerp=[](auto a,auto b,double t){for(int j=0;j<3;++j)a[j]+=(b[j]-a[j])*t;return a;};
  const auto radial=[&](int x,int y){const auto& row=w.first_blocks.at(std::size_t(std::clamp(y,0,last)));
    std::array<double,3>a{},b{};for(int j=0;j<3;++j){a[j]=std::bit_cast<double>(row[(unsigned(x)&31u)*2u][j]);b[j]=std::bit_cast<double>(row[(unsigned(x)&31u)*2u+1u][j]);}return lerp(a,b,p.z);};
  return lerp(lerp(radial(bx,by),radial(bx+1,by),(p.x-bx*8)/8),
    lerp(radial(bx,by+1),radial(bx+1,by+1),(p.x-bx*8)/8),(p.y-by*16)/16);
}
std::array<double,2> projected(const app::CameraBasis& c,const std::array<double,3>& p){
  const auto m=app::actorProjectionInput(c).worldToLensedView;
  const double z=m[2]*p[0]+m[5]*p[1]+m[8]*p[2]+m[11]-0.0003;
  return {c.cx+(m[0]*p[0]+m[3]*p[1]+m[6]*p[2]+m[9])/z,
    c.cy-(m[1]*p[0]+m[4]*p[1]+m[7]*p[2]+m[10])/z};
}
#ifdef FLUID_KINDS_after
void units(){
  SceneNode node;node.type=1;node.positions.resize(6);node.normals.assign(3,{0,1,0});
  SceneMeshBlock side,top;side.kind=4;top.kind=5;
  for(auto indices:{std::array<unsigned,3>{2,4,3},std::array<unsigned,3>{3,4,5}}){
    SceneRecord tri;for(auto v:indices){SceneCorner c;c.vertexIndex=v;c.a=91;tri.corners.push_back(c);}top.records.push_back(tri);}
  side.records=top.records;node.meshes={side,top};
  for(unsigned kind:{0u,1u,2u}){
    app::BackdropScene::FluidRecord rec;rec.kind=kind;rec.baseColour={0,0,192};for(auto& a:rec.amp)a={0,1,0};
    app::BackdropScene::FluidGateState st{1,10,0,true,2};auto g=node;app::BackdropScene::applyFluidState(g,rec,st);
    const double expected=kind==0?10.0:kind==1?10.92387952832938:9.75;
    check(near(g.positions[2][1],expected,1e-6),"literal wave amplitude/lane for kind "+std::to_string(kind));
    for(const auto& block:g.meshes)for(const auto& tri:block.records)for(const auto& c:tri.corners)
      check(near(c.a,kind==2?127.0:91.0,1e-5),"kind-2 alpha midpoint; other kinds retain stored alpha");
    st.height=0;g=node;app::BackdropScene::applyFluidState(g,rec,st);
    check(near(g.positions[2][1],0)&&g.normals[0][1]==-1,"drained collapse and normal sign");
    sim::FluidGate gate;gate.kind=int(kind);gate.advanceWave(kind==1?8.2:kind==2?4.2:2.2);
    check(near(gate.wavePhase,0.2),"per-kind phase modulus");
  }
}

int publicApplication(const fs::path& out,const fs::path& game,const AssetLibrary& lib){
  std::ofstream audit(out/"frames.csv");audit<<"level,frame,ms,x,y,actual_origin_y,expected_origin_y\n";
  for(const auto& id:{"MM_W_012","MM_ALL_8_001","M17_MT_1"}){
    auto resolved=sim::resolveRetailLevel(lib,id);if(!resolved)return 2;
    auto blob=lib.read(resolved->file);if(!blob)return 2;
    const auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(blob->data()),blob->size()});
    const auto camera=out/(std::string(id)+"-camera.json");
    std::ofstream(camera)<<"{\"viewport\":[640,480],\"yaw_units\":128,\"level_eye_height\":96}\n";
    std::vector<std::string>args{"fluid-kinds","--game-dir",game.string(),"--level",id,"--sim","--native","--camera",camera.string(),"--frames","110","--no-audio"};
    std::vector<char*>raw;for(auto& a:args)raw.push_back(a.data());
    app::ApplicationRunOptions options;options.frameSeconds=0.1;
    options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
    int frames=0,landFrames=0;
    const auto observe=[&](const app::LevelFrameView& f){
      ++frames;
      if(f.frame==60||f.frame==100)png(out/(std::string(id)+"-frame-"+std::to_string(f.frame)+".png"),f.image);
      if(std::string(id)!="MM_W_012")return;
      for(const auto& a:f.actors){
        if(a.index!=0)continue;
        const auto& l=f.world.lemmings()[a.index];
        if(l.state!=sim::State::Walker||l.x<145||l.x>153)continue;
        const auto expected=originalPoint(level.world,{a.drawnSample[0],a.drawnSample[1],a.drawnSample[2]});
        const auto origin=projected(app::buildCamera(f.displayedPose),expected);
        audit<<id<<','<<f.frame<<','<<f.world.levelClockMs()<<','<<l.x<<','<<l.y<<','<<a.placement.originY<<','<<origin[1]<<'\n';
        check(near(a.placement.originY,origin[1],1e-6)&&near(expected[1],82),"ordinary runApplication actor stands on land after water");++landFrames;
      }
    };
    check(app::runApplication(int(raw.size()),raw.data(),{},observe,options)==0,"ordinary application route exits normally");
    check(frames==110,"bounded app frames observed");
    if(std::string(id)=="MM_W_012")check(landFrames>5,"ordinary app observes natural water-to-land crossing");
  }
  return failures?1:0;
}
#endif
}
int main(int argc,char** argv){
  if(argc<3||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  const fs::path out=argv[2];fs::create_directories(out);const fs::path game=std::getenv("WEBOLUTION_GAME_DIR");
  const auto local=save::localAppDataDirectory();if(!local)return 2;
  const SavedFile installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  // Component demonstrations never create a save sink or write progress.
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
#ifdef FLUID_KINDS_after
  if(std::string(argv[1])=="public"){
    const int result=publicApplication(out,game,*lib);
    check(installed.unchanged()&&normal.unchanged(),"ordinary application leaves installed and normal saves untouched");
    std::ofstream(out/"summary.txt")<<"installed_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
    return result||failures?1:0;
  }
#endif
  auto map=app::MapScene::build(*lib,{});if(!map)return 2;
  auto packed=lib->read("TEXTURES\\LEVEL MASK.BMP");if(!packed)return 2;auto raw=lrzDecode(*packed);if(!raw)return 2;
  auto u32=[&](int p){unsigned n=0;for(int i=0;i<4;++i)n|=std::to_integer<unsigned>((*raw)[p+i])<<(8*i);return n;};
  const auto offset=u32(10);if(offset+640u*480u>raw->size())return 2;
  std::array<int,102>sx{},sy{},count{};
  for(int y=0;y<480;++y)for(int x=0;x<640;++x){auto n=std::to_integer<unsigned>((*raw)[offset+(479-y)*640+x]);if(n<102){sx[n]+=x;sy[n]+=y;++count[n];}}
#ifdef FLUID_KINDS_after
  units();
#endif
  std::ofstream census(out/"census.txt"),trace(out/"walk.csv"),summary(out/"summary.txt");
  census<<std::setprecision(17);trace<<std::setprecision(17);
  trace<<"ms,x,y,state,draw_x,draw_y,expected_model_y,actual_origin_y,expected_origin_y,zone_water\n";
  int rejectedLevels=0,rendered=0;bool reproducedHeight=false,observedLand=false,observedWater=false;
  for(int n=0;n<102;++n){
    auto id=map->levelForNode(n);if(!id||!count[n])return 2;
    census<<"node="<<n<<" id="<<*id<<" title="<<*map->titleForNode(n)<<" map="<<sx[n]/count[n]<<','<<sy[n]/count[n];
    auto resolved=sim::resolveRetailLevel(*lib,*id);if(!resolved)return 2;auto bytes=lib->read(resolved->file);if(!bytes)return 2;
    const auto compiled=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes->data()),bytes->size()});
    std::set<unsigned>kinds;std::size_t modelCount=0;
    for(const auto& g:compiled.gadgets.entities_42acac){kinds.insert(g.values_18c_128_168[1]);modelCount+=g.model.has_value();}
    census<<" coordinate_height="<<compiled.world.leading_value<<" rings="<<compiled.world.first_blocks.size()<<" kinds=";for(auto k:kinds)census<<k<<',';
    auto setup=sim::loadRetailLevel(*lib,*id);if(!setup)return 2;
    census<<" hatches=";for(const auto& h:setup->entrances)census<<h.x<<','<<h.y<<",sheet"<<h.sheetType<<';';
    census<<" hazards="<<setup->terrain.waterHazardCells()<<" water="<<setup->terrain.zoneWaterCells()<<" acid="<<setup->terrain.zoneAcidCells();
    auto backdrop=app::BackdropScene::build(*lib,*bytes,*id);if(!backdrop)return 2;
    census<<" admitted="<<backdrop->fluidEntityIndices().size()<<'/'<<modelCount<<'\n';
    bool rejected=false;for(const auto& d:backdrop->diagnostics()){census<<d<<'\n';rejected|=d.find("receiver fluid not admitted")!=std::string::npos;}rejectedLevels+=rejected;
#ifdef FLUID_KINDS_after
    check(!rejected&&backdrop->fluidEntityIndices().size()==modelCount,"all installed receiver models admitted: "+*id);
#endif
    const bool selected=argc>3?*id==argv[3]:(n==8||n==22||n==35||n==43||n==44);if(!selected)continue;
    const fs::path dir=out/resolved->id;fs::create_directories(dir);++rendered;
    auto mesh=findCylinderMesh(*bytes);auto index=indexLevel(*bytes);if(!mesh||!index)return 2;
    if(n==8)for(auto vi:{0,1,2,15,16,31,32,33,63}){const auto& v=mesh->vertices[std::size_t(vi)];census<<"raw vertex["<<vi<<"]="<<v.x<<','<<v.y<<','<<v.z<<'\n';}
    auto tube=app::TubeScene::build(*lib,*mesh,*bytes,index->grid0,true);if(!tube)return 2;
    sim::World world(std::move(*setup));world.advanceElapsedMs(20);
    for(std::size_t i=0;i<world.setup().gates.size();++i){const auto& g=world.setup().gates[i];
      census<<"gate "<<*id<<'['<<i<<"] id="<<g.id<<" kind="<<g.kind<<" request="<<g.request<<" min="<<g.minX<<','<<g.minY<<" max="<<g.maxX<<','<<g.maxY<<" h="<<g.height<<" lane="<<g.lane<<'\n';}
    std::size_t best=0;
    for(int yaw=0;yaw<256;yaw+=32){
      app::CameraParams p;p.yawUnits=yaw;p.levelEyeHeight=world.terrain().height();
      const auto shot=shoot(*backdrop,*tube,*lib,world,app::buildCamera(p));png(dir/("initial-yaw-"+std::to_string(yaw)+".png"),shot.image);
      best=std::max(best,shot.stats.fluidPixelsChanged);census<<*id<<" yaw="<<yaw<<" fluid_pixels="<<shot.stats.fluidPixelsChanged<<'\n';
#if defined(FLUID_KINDS_after) && defined(FLUID_BASELINE_AVAILABLE)
      if(n==8)check(file(dir/("initial-yaw-"+std::to_string(yaw)+".png"))==
        file(out.parent_path()/"before"/resolved->id/("initial-yaw-"+std::to_string(yaw)+".png")),"MM_W_012 matched-phase fluid frames byte-identical");
#endif
    }
#ifdef FLUID_KINDS_after
    check(best>0,"installed fluid writes visible pixels in "+*id);
#else
    if(n!=8)check(best==0,"task-entry kinds 1/2 write no fluid in "+*id);
#endif
    if(n!=8)continue;
    check(world.setup().entrances.size()==1&&world.setup().entrances[0].sheetType==1&&world.terrain().waterHazardCells()==0,
      "described level has water lemmings and no stored 0x40 hazard");
    app::LemmingMotionTracker motion;motion.observe(world);app::CameraParams crossing;crossing.levelEyeHeight=96;crossing.yawUnits=128;
    bool cameraChosen=false;
    // Natural hatch -> upper water -> land crossing, no actor or gate injection.
    for(int tick=1;tick<320;++tick){
      world.advanceElapsedMs(20,false,[&](double){motion.observe(world);return true;});if(world.lemmings().empty())continue;
      const auto& l=world.lemmings()[0];const auto drawn=motion.positionOf(0,world);
      if(l.state!=sim::State::Walker||world.levelClockMs()<4200)continue;
      if(!cameraChosen){
        for(int yaw=0;yaw<256;yaw+=8){
          crossing.yawUnits=yaw;auto camera=app::buildCamera(crossing);
          const auto point=originalPoint(compiled.world,{drawn.x,world.terrain().height()-1.0-drawn.y,drawn.z});
          const auto origin=projected(camera,point);const auto shot=shoot(*backdrop,*tube,*lib,world,camera,&motion);
          if(!shot.actors.empty()&&shot.actors[0]&&origin[0]>200&&origin[0]<440&&origin[1]>40&&origin[1]<280){cameraChosen=true;break;}
        }check(cameraChosen,"visible camera for natural crossing");census<<"crossing_yaw="<<crossing.yawUnits<<'\n';
      }
      const auto camera=app::buildCamera(crossing);
      const auto expected=originalPoint(compiled.world,{drawn.x,world.terrain().height()-1.0-drawn.y,drawn.z});
      const auto origin=projected(camera,expected);const auto shot=shoot(*backdrop,*tube,*lib,world,camera,&motion);
      if(shot.actors.empty()||!shot.actors[0]){check(false,"natural crossing actor visible");continue;}
      const double actual=shot.actors[0]->originY;const bool water=world.terrain().zoneWater(l.x,l.y+1);
      observedWater|=water;observedLand|=!water;reproducedHeight|=std::abs(actual-origin[1])>1;
      trace<<world.levelClockMs()<<','<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','<<drawn.x<<','<<drawn.y<<','<<expected[1]<<','<<actual<<','<<origin[1]<<','<<water<<'\n';
#ifdef FLUID_KINDS_after
      check(near(actual,origin[1],1e-6),"production feet follow evidenced coordinate mesh at water/land boundary");
      if(drawn.x>=144)check(near(expected[1],82),"land feet rise to installed floor Y=82");
#endif
      if(world.levelClockMs()%200==0)png(dir/("crossing-"+std::to_string(world.levelClockMs())+".png"),shot.image);
    }
    // Labelled placement fixture for actual lever contact, separate from natural crossing.
    const auto lev=world.setup().levers.at(2);sim::Lemming contact;contact.x=lev.x+1;contact.y=lev.y+lev.height;contact.facing=sim::Facing::Left;contact.state=sim::State::Walker;
    for(int y=lev.y;y<=lev.y+lev.height;++y)if(world.terrain().solid(contact.x,y+1)){contact.y=y;break;}
    world.place(contact);for(int i=0;i<150;++i)world.advanceElapsedMs(100);
    app::CameraParams p;p.yawUnits=160;p.levelEyeHeight=96;
    const auto raised=shoot(*backdrop,*tube,*lib,world,app::buildCamera(p));png(dir/"bottom-tubs-after-lever-fixture.png",raised.image);
    int targets=0;for(const auto& g:world.setup().gates)if((g.id==5||g.id==6)&&g.height==24.5)++targets;
    check(targets==2&&raised.stats.fluidPixelsChanged>0,"real bottom receivers raise and draw after lever contact fixture");
  }
  check(observedWater&&observedLand,"natural water-to-land transition observed");
#ifdef FLUID_KINDS_before
  check(rejectedLevels==19,"reproduce sweep's 19 partially admitted levels");check(reproducedHeight,"reproduce visible standing-height defect");
#else
  check(rejectedLevels==0,"no installed receiver admission diagnostics remain");
#endif
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves untouched");
  summary<<"rejected_levels="<<rejectedLevels<<"\nrendered_levels="<<rendered<<"\nwater_observed="<<observedWater<<"\nland_observed="<<observedLand
    <<"\nvisible_height_defect="<<reproducedHeight<<"\ninstalled_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  std::cout<<"fluid-kinds evidence: "<<out<<" failures="<<failures<<'\n';return failures?1:0;
}
