// Check scenery integration with installed inputs and a static camera.
#include "app/BackdropScene.h"
#include "app/SceneFrame.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Bitmap.h"
#include "assets/Png.h"
#include "lemmings/compiled_level.hpp"
#include "app/AppEntry.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace fs=std::filesystem;
using namespace webo;
using namespace webo::app;
using Bytes=std::vector<std::byte>;
namespace {
void require(bool pass,const std::string& message) {
  if(!pass)throw std::runtime_error(message);
}
[[maybe_unused]] std::span<const std::uint8_t> raw(std::span<const std::byte> b) {
  return {reinterpret_cast<const std::uint8_t*>(b.data()),b.size()};
}
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
void artifact(const fs::path& dir,const std::string& name,Raster& raster) {
  require(bool(writePng(dir/(name+".png"),image(raster))),"PNG "+name);
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
Rendered render(const BackdropScene& backdrop,const TubeScene& tube,const AssetLibrary& lib,
                const CameraBasis& cam,bool scenery,bool compose,std::ostream& log) {
  Rendered out;out.raster.clear(0,0,0);
  std::vector<std::uint8_t> mask(640*480,1);
  SceneOptions options;options.drawCellRoute=true;options.drawWall=false;
  options.drawOuter=false;options.drawInner=false;
  const auto stage=[&](std::size_t i){
    if(!scenery)return;
    SceneRasterObserver observer;observer.mask=&mask;
    observer.candidate=[&](auto,auto,auto,auto){++out.candidates[i];};
    observer.accepted=[&](const SceneRasterWriteEvent& e){
      // Sample individual pixel changes as well as aggregate counts across 640x480.
      if(out.writes[i]<3){
        log<<"accepted,"<<stageNames[i]<<",source="<<e.source<<",triangle="<<e.triangle
           <<",pixel="<<e.pixel<<",before=";
        for(auto c:e.colorBefore)log<<unsigned(c)<<'/';
        log<<",after=";for(auto c:e.colorAfter)log<<unsigned(c)<<'/';
        log<<",zBefore="<<e.depthBefore<<",zAfter="<<e.depthAfter<<",depthAssigned="<<e.depthAssigned<<'\n';
      }
      ++out.writes[i];out.depthWrites[i]+=e.depthAssigned;
      out.changedColors[i]+=e.colorBefore!=e.colorAfter;
    };
    backdrop.renderStage(out.raster,cam,lib,out.stats[i],stages[i],&observer);
  };
  scheduleSceneFrame([&]{stage(0);},[&]{stage(1);},[&]{if(compose)tube.render(out.raster,cam,options,&lib);},[&]{stage(2);});
  for(std::size_t i=0;i<3;++i){
    const auto& s=out.stats[i];
    log<<stageNames[i]<<",candidates="<<s.stageMeshRecords<<'/'<<s.stageBillboards
       <<",successfulRasterReturns="<<s.meshTriangles<<",acceptedColorStores="<<out.writes[i]
       <<",acceptedDepthStores="<<out.depthWrites[i]<<",changedColorStores="<<out.changedColors[i]
       <<",predicateCulls="<<s.predicateRejected<<",missing="<<s.missingTextures<<",rejected="<<s.rejected;
    log<<",rasterCalls="<<s.rasterCalls<<",projectionRejected="<<s.projectionRejected
       <<",rasterRejected="<<s.rasterRejected;
    require(s.rejected==s.projectionRejected+s.rasterRejected,"rejection attribution");
    require(s.acceptedColorWrites==out.writes[i]&&s.acceptedDepthWrites==out.depthWrites[i],"accepted-event counters");
    for(const auto& d:s.diagnostics)log<<"\ndiagnostic,"<<d;
    log<<'\n';
  }
  log<<"tubeCalls="<<out.raster.drawnTriangles()<<",tubeAcceptedPixels="<<out.raster.drawnPixels()<<'\n';
  return out;
}
std::uint32_t word(const Bytes& b,std::size_t p) {
  std::uint32_t v=0;for(int i=0;i<4;++i)v|=std::uint32_t(std::to_integer<unsigned char>(b.at(p+i)))<<(i*8);return v;
}
void setWord(Bytes& b,std::size_t p,std::uint32_t v) {
  for(int i=0;i<4;++i)b.at(p+i)=std::byte((v>>(8*i))&255);
}
void boundaries(const BackdropScene& scene,const Bytes& bytes,bool bash,std::ostream& log) {
  using E=lemmings::ObjectSourceExtent;
  const std::array<E,4> expected=bash?std::array<E,4>{{
      {0x5be41,0x5be4e,0x93f78,0x93f7e},{0x93f7f,0x93f8c,0x9b54b,0x9f453},
      {0x9f4ce,0x9f4db,0xa0335,0},{0xa34da,0xa34e7,0xad24d,0}}}:
      std::array<E,4>{{{0x59637,0x59644,0x9176e,0x91774},{0x91775,0x91782,0x98d41,0x9cc49},
      {0x9ccc4,0x9ccd1,0x9db2b,0},{0xa0cd0,0xa0cdd,0xaaa43,0}}};
  const std::array<std::size_t,4> wantNodes={4,30,4,2},wantMesh={923,82,16,156},wantBillboards={0,17,0,0};
  std::size_t i=0;
  scene.visitRoots([&](const BackdropScene::RootSource& source,const SceneNode& root){
    require(i<4,"bounded owned root count");const auto& e=source.extent;const auto& want=expected[i];
    log<<"root,"<<source.owner<<'['<<source.entityIndex<<"],"<<std::hex<<e.marker_begin<<','<<e.typed_root_begin<<','<<e.tree_end<<','<<e.holder_end<<std::dec<<'\n';
    require(e.marker_begin==want.marker_begin&&e.typed_root_begin==want.typed_root_begin&&e.tree_end==want.tree_end,"independent accepted research root boundaries");
    require(e.holder_end>e.tree_end&&e.holder_end<=bytes.size(),"holder includes trailer");
    require(source.stage==(i==0?stages[0]:i==1?stages[2]:stages[1]),"typed ownership role");
    require(source.entityIndex==0,"entity/model index provenance");
    std::size_t nodes=0,records=0,billboards=0;
    const auto visit=[&](const auto& self,const SceneNode& n)->void{
      ++nodes;billboards+=n.type==2;for(const auto& b:n.meshes)records+=b.records.size();
      for(const auto& c:n.children)self(self,c);
    };visit(visit,root);
    require(nodes==wantNodes[i]&&records==wantMesh[i]&&billboards==wantBillboards[i],"independent role inventory");
    auto parsed=readSceneNode(bytes,e.typed_root_begin);
    require(parsed&&parsed->endOffset==e.tree_end,"exact SceneNode recursive end");
    lemmings::LevelByteStream holder{raw(bytes),e.marker_begin};
    const auto independentHolder=lemmings::read_serialized_object(holder);
    require(holder.cursor==e.holder_end&&independentHolder.source_extent.typed_root_begin==e.typed_root_begin&&independentHolder.source_extent.tree_end==e.tree_end,"paired/inline extent equals independently consumed holder");
    for(auto cut:{e.tree_end,e.holder_end-1}){
      lemmings::LevelByteStream truncated{raw(bytes).first(cut),e.marker_begin};bool rejected=false;
      try{(void)lemmings::read_serialized_object(truncated);}catch(const std::invalid_argument&){rejected=true;}
      require(rejected,"standalone/inline truncated holder or auxiliary rejected");
    }
    ++i;
  });require(i==4,"two scenery roots and two inline models");
  const auto level=lemmings::read_compiled_level(raw(bytes));
  const auto& p=level.objects;
  require(p.source_extent.holder_end==expected[0].holder_end,"primary end before second-presence byte");
  require(p.second_object_source_extent&&p.second_object_source_extent->holder_end==expected[1].holder_end,"secondary includes nonempty auxiliary payload");
  require(p.second_object_auxiliary_records.size()==1&&!p.second_object_auxiliary_records[0].serialized_bytes.empty(),"retained raw auxiliary data");
  lemmings::LevelByteStream stream{raw(bytes),expected[1].marker_begin};
  const auto standalone=lemmings::read_serialized_object(stream);
  require(stream.cursor==expected[1].holder_end&&standalone.source_extent.tree_end==expected[1].tree_end,"shared SerializedObject consumes identical auxiliary extent");
  require(standalone.auxiliary_records[0].serialized_bytes==p.second_object_auxiliary_records[0].serialized_bytes,"auxiliary bytes preserved between readers");
}
void negatives(const AssetLibrary& lib,const Bytes& bytes,std::ostream& log) {
  const auto level=lemmings::read_compiled_level(raw(bytes));
  const auto primary=level.objects.source_extent,secondary=*level.objects.second_object_source_extent;
  // Delete the optional holder using the fixture's known serialized offset.
  auto absent=bytes;absent[primary.holder_end]=std::byte{0};
  absent.erase(absent.begin()+secondary.marker_begin,absent.begin()+secondary.holder_end);
  auto noSecond=BackdropScene::build(lib,absent,"MM_BASH_002");require(bool(noSecond),"absent secondary valid");
  std::size_t roots=0;noSecond->visitRoots([&](const auto& s,const auto&){++roots;require(s.stage!=stages[2],"absent secondary not synthesized");});
  require(roots==3,"absent secondary retains inline models");
  for(auto end:{primary.tree_end,secondary.holder_end-1}){
    lemmings::LevelByteStream input{raw(bytes).first(end),primary.marker_begin};bool rejected=false;
    try{(void)lemmings::read_object_base_prefix(input);}catch(const std::invalid_argument& e){rejected=true;log<<"truncated,"<<end<<','<<e.what()<<'\n';}
    require(rejected,"truncated flags/auxiliary rejected");
  }
  const auto& model=*level.gadgets.entities_425e50.at(0).model;
  const auto modelBegin=std::size_t{0x9f4a2}; // installed fixture offset
  lemmings::LevelByteStream modelStream{raw(bytes),modelBegin};(void)lemmings::read_level_model(modelStream);
  Bytes modelOne(24,std::byte{0});setWord(modelOne,0,1); // type1: four words and empty resource string
  auto unsupported=bytes;unsupported.erase(unsupported.begin()+modelBegin,unsupported.begin()+modelStream.cursor);
  unsupported.insert(unsupported.begin()+modelBegin,modelOne.begin(),modelOne.end());
  auto result=BackdropScene::build(lib,unsupported,"MM_BASH_002");
  // An unsupported present model produces an empty partial scene with a diagnostic.
  require(result&&result->empty()&&result->diagnostics().size()==1&&result->diagnostics()[0].find("unsupported present model type 1")!=std::string::npos&&result->diagnostics()[0].find("LegacyUnattributed")!=std::string::npos,"explicit typed model diagnostic (empty partial scene)");log<<"unsupported-model,"<<result->diagnostics()[0]<<'\n';
  auto noModel=bytes;noModel[modelBegin-2]=std::byte{0};
  noModel.erase(noModel.begin()+modelBegin,noModel.begin()+modelStream.cursor);
  result=BackdropScene::build(lib,noModel,"MM_BASH_002");require(bool(result),"absent optional model valid");
  roots=0;result->visitRoots([&](const auto&,const auto&){++roots;});require(roots==3,"absent model contributes no root");
  auto badNode=bytes;const auto& e=model.object->source_extent;
  // Replace the inline type0 root by a valid serialized type3 (10 f64 then
  // the same common base). The composed reader supports it; SceneNode rejects.
  badNode.insert(badNode.begin()+e.typed_root_begin+4,80,std::byte{0});setWord(badNode,e.typed_root_begin,3);
  result=BackdropScene::build(lib,badNode,"MM_BASH_002");
  // The empty scene diagnostic must name the role and unsupported node type.
  require(result&&result->empty()&&result->diagnostics().size()==1&&result->diagnostics()[0].find("LegacyUnattributed")!=std::string::npos&&result->diagnostics()[0].find("unsupported scene node type 3")!=std::string::npos,"explicit unsupported node diagnostic (empty partial scene)");log<<"unsupported-node,"<<result->diagnostics()[0]<<'\n';
  auto dig=lib.read("COMPILED\\MM_DIG_000.LVL");require(bool(dig),"read real branch1 level");
  // In MM_DIG_000 replace the first branch-1 'fire' name (four bytes at 0xa7587)
  // with missing sprite 'nofi' to exercise the unsupported form.
  { auto admitted=BackdropScene::build(lib,*dig,"MM_BASH_002");
    require(admitted&&!admitted->empty()&&admitted->diagnostics().empty()&&admitted->spriteCount()==19,"real branch1 bytes are admitted under any ID"); }
  auto digMissing=*dig;require(digMissing.size()>0xa758b&&char(digMissing[0xa7587])=='f'&&char(digMissing[0xa758a])=='e',"first branch-1 name is 'fire'");
  for(std::size_t i=0;i<4;++i)digMissing[0xa7587+i]=std::byte{"nofi"[i]};
  result=BackdropScene::build(lib,digMissing,"MM_BASH_002");
  require(result&&result->empty()&&result->diagnostics().size()==1&&result->diagnostics()[0].find("type-2 branch 1 sprite 'nofi'")!=std::string::npos&&result->diagnostics()[0].find("Secondary")!=std::string::npos,"unsupported branch1 form diagnosed (empty partial scene)");log<<"unsupported-branch,"<<result->diagnostics()[0]<<'\n';
  // Malformed data stays a hard error independent of the level ID: truncated container,
  // damaged Object 09 marker.
  auto cut=bytes;cut.resize(primary.tree_end);
  for(const char* id:{"MM_BASH_002","MM_DIG_000"}){auto truncatedBuild=BackdropScene::build(lib,cut,id);require(!truncatedBuild,std::string("truncated level stays a hard error for ")+id);log<<"malformed-truncated,"<<id<<','<<truncatedBuild.error()<<'\n';}
  auto badMarker=bytes;badMarker[primary.marker_begin+4]=std::byte{'X'};
  auto markerResult=BackdropScene::build(lib,badMarker,"MM_BASH_002");require(!markerResult,"damaged Object 09 marker stays a hard error");log<<"malformed-marker,"<<markerResult.error()<<'\n';
  // Shift a serialized prefix name by one byte: every downstream holder moves.
  auto shifted=bytes;const auto n=word(shifted,0);setWord(shifted,0,n+1);shifted.insert(shifted.begin()+4+n,std::byte{'x'});
  result=BackdropScene::build(lib,shifted,"MM_BASH_002");require(bool(result),"shifted layout parses structurally");
  std::array<std::size_t,4> oldRoots={primary.typed_root_begin,secondary.typed_root_begin,
      model.object->source_extent.typed_root_begin,level.gadgets.entities_426c6b.at(0).model->object->source_extent.typed_root_begin};
  std::size_t shiftedIndex=0;
  result->visitRoots([&](const auto& s,const auto&){require(shiftedIndex<oldRoots.size()&&s.extent.typed_root_begin==oldRoots[shiftedIndex++]+1,"each structural root follows shifted layout");});
  require(shiftedIndex==4,"shifted layout retains every owned root");
  auto unsupportedId=BackdropScene::build(lib,digMissing,"MM_DIG_000");require(unsupportedId&&unsupportedId->empty()&&!unsupportedId->diagnostics().empty(),"undemonstrated level explicitly partial");
  // The empty scene diagnostic must name the level, role and unsupported form.
  { const auto& d=unsupportedId->diagnostics().front();
    require(d.find("MM_DIG_000")!=std::string::npos&&d.find("Secondary")!=std::string::npos&&d.find("unsupported type-2 branch 1 sprite 'nofi'")!=std::string::npos&&d.find("scenery adapter not demonstrated")==std::string::npos,"real-ID MM_DIG_000 (missing-sprite variant) diagnosed empty partial scene names role and form");
    log<<"unsupported-real-id,"<<d<<'\n'; }
  // Check billboard adapter failure on an installed tree.
  auto invalidBillboard=bytes;auto tree=readSceneNode(bytes,secondary.typed_root_begin);
  require(bool(tree),"secondary tree for bounded adapter rejection");
  const auto leaf=std::find_if(tree->root.children.begin(),tree->root.children.end(),[](const auto& n){return n.type==2;});
  require(leaf!=tree->root.children.end(),"actual billboard leaf");
  const auto nan=std::bit_cast<std::uint64_t>(std::numeric_limits<double>::quiet_NaN());
  for(int i=0;i<8;++i)invalidBillboard[leaf->commonHeaderOffset+i]=std::byte((nan>>(8*i))&255);
  result=BackdropScene::build(lib,invalidBillboard,"MM_BASH_002");require(bool(result),"serialized NaN retained until adapter validation");
  Raster target(640,480);target.clear(0,0,0);BackdropStats stats;
  result->renderStage(target,buildCamera(CameraParams{}),lib,stats,stages[2]);
  require(std::any_of(stats.diagnostics.begin(),stats.diagnostics.end(),[](const auto& d){return d.find("Secondary")!=std::string::npos&&d.find("source=0x")!=std::string::npos&&d.find("node matrix must be finite")!=std::string::npos;}),"billboard-adapter failure has level/role/source diagnostic");
  for(const auto& d:stats.diagnostics)log<<"adapter-rejection,"<<d<<'\n';
}
void application(const std::string& gameDir,const fs::path& dir,std::ostream& log) {
  require(std::string(std::getenv("SDL_VIDEODRIVER")?std::getenv("SDL_VIDEODRIVER"):"")=="dummy","dummy video required");
  require(std::string(std::getenv("SDL_AUDIODRIVER")?std::getenv("SDL_AUDIODRIVER"):"")=="dummy","dummy audio required");
  const auto cameraFile=dir/"fixed-camera.json";std::ofstream(cameraFile)<<R"({"viewport":[640,480],"center":[319,239],"aspect":1.3333333333333333,"angle_degrees":19.03,"yaw_units":32,"yaw_offset":128,"yaw_units_per_turn":256,"zoom_shift":0,"level_eye_height":96,"height_bias":17.392,"eye_distance":377.516,"fixed_pitch_turns":-0.03135770272046,"pose_pi":3.1415927410125732,"lens_pi":3.1415927410125732,"angle_units_per_turn":360,"elevation":0})"<<'\n';
  std::array<Image,2> frames;
  for(int off=0;off<2;++off){
    std::vector<std::string> args={"scene-integration","--game-dir",gameDir,"--level","MM_BASH_002","--sim","--native","--geom","route","--camera",cameraFile.string(),"--no-audio"};
    if(off)args.emplace_back("--scene-off");
    std::vector<char*> argv;for(auto& a:args)argv.push_back(a.data());
    int callbacks=0;bool queued=false;
    const auto observer=[&](const LevelFrameView& view){
      ++callbacks;require(view.levelName=="MM_BASH_002"&&view.frame==0,"public level route/frame");frames[off]=view.image;
      SDL_Event quit{};quit.type=SDL_QUIT;queued=SDL_PushEvent(&quit)==1;
    };
    const auto result=runApplication(static_cast<int>(argv.size()),argv.data(),{},observer,{});
    require(result==0&&queued&&callbacks==1,"public --sim route exits normally through queued SDL_QUIT");
    require(bool(writePng(dir/(off?"application-scene-off.png":"application-scene-on.png"),frames[off])),"public frame artifact");
    log<<"application,sceneOff="<<off<<",return="<<result<<",frames="<<callbacks<<",queuedQuit="<<queued<<'\n';
  }
  const auto pixels=changed(frames[0].rgba,frames[1].rgba,4);require(pixels>0,"public route scenery changes rendered pixels");
  log<<"applicationChangedPixels="<<pixels<<'\n';
}
}

int main()try {
  const char* gameDir=std::getenv("WEBOLUTION_GAME_DIR");require(gameDir&&*gameDir,"installed assets required");
  auto lib=AssetLibrary::open(fs::path(gameDir)/"lemmings.box",fs::path(gameDir)/"patch.box");require(bool(lib),"open installed archives");
  const std::string side="after";
  const fs::path root=SCENE_DEMO_DIR,dir=root/side;fs::create_directories(dir);
  std::ofstream log(dir/"measurements.txt");log<<std::setprecision(17);
  CameraParams cp;const auto cam=buildCamera(cp); // explicit same source camera for both levels/sides
  for(const std::string id:{"MM_CLIMB_002","MM_BASH_002"}){
    auto bytes=lib->read("COMPILED\\"+id+".LVL");require(bool(bytes),"read actual "+id);
    auto backdrop=BackdropScene::build(*lib,*bytes,id);require(bool(backdrop),"build "+id+(backdrop?"":backdrop.error()));
    auto mesh=findCylinderMesh(*bytes);auto index=indexLevel(*bytes);require(bool(mesh)&&bool(index),"tube input");
    auto tube=TubeScene::build(*lib,*mesh,*bytes,index->grid0,true);require(bool(tube),"production tube build");
    for(const auto& missing:tube->missingTiles())log<<"tubeMissing,"<<missing<<'\n';
    log<<"level="<<id<<",side="<<side<<",bytes="<<bytes->size()<<'\n';
    boundaries(*backdrop,*bytes,id=="MM_BASH_002",log);
    if(id=="MM_BASH_002")negatives(*lib,*bytes,log);
    log<<"scene-only\n";auto scene=render(*backdrop,*tube,*lib,cam,true,false,log);artifact(dir,id+"-scene-only",scene.raster);
    log<<"composed-scene-on\n";auto on=render(*backdrop,*tube,*lib,cam,true,true,log);artifact(dir,id+"-scene-on",on.raster);
    log<<"composed-scene-off\n";auto off=render(*backdrop,*tube,*lib,cam,false,true,log);artifact(dir,id+"-scene-off",off.raster);
    const auto changedPixels=changed(on.raster.framebuffer().rgb,off.raster.framebuffer().rgb);
    log<<"composedChangedPixels="<<changedPixels<<'\n';
    if(id=="MM_CLIMB_002"){
      Raster primary(640,480);primary.clear(0,0,0);BackdropStats st;backdrop->renderStage(primary,cam,*lib,st,stages[0]);
      require(hash(primary)==0x1fd0bfb82ab39e95ull,"preserve primary RGB/depth hash");
      require(scene.stats[0].stageMeshRecords==923&&scene.stats[1].stageMeshRecords==172&&scene.stats[2].stageMeshRecords==82&&scene.stats[2].stageBillboards==17,"retain stage candidate partitions");
    }else{
      require(!backdrop->empty()&&scene.stats[0].meshRecords==1177,"current populated candidates");
      require(scene.stats[0].rasterCalls+scene.stats[2].rasterCalls>0&&scene.writes[0]+scene.writes[2]>0&&scene.depthWrites[0]+scene.depthWrites[2]>0,"actual scenery raster calls and accepted color/depth stores");
      require(changedPixels>0,"composed additional-level scenery changes pixels");
    }
  }
  application(gameDir,dir,log);
  log.close();const auto report=read(dir/"measurements.txt");std::cout.write(report.data(),static_cast<std::streamsize>(report.size()));
  std::cout<<"\nPASS scene-integration "<<side<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<"FAIL scene-integration: "<<e.what()<<'\n';return 1;}
