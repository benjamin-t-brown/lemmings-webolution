// scene-admission-001 step 1: structural census of every effective installed level.
// Source-built, read-only archive access. It mirrors the production structural
// pipeline (read_compiled_level, readSceneNode with exact tree_end check, Secondary
// leaf-domain check, model type0 requirement, buildSceneMeshSource and the billboard
// adapter) WITHOUT any level-ID allowlist and classifies each level by outcome. It
// does not render, run original code or claim native admission.
#include "app/Camera.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneMeshSource.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/SceneNode.h"
#include "assets/Sprite.h"
#include "lemmings/compiled_level.hpp"
#include "sceneadmission_census.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs=std::filesystem;
using namespace webo;
using namespace webo::app;
namespace {
using Matrix=std::array<double,12>;
Matrix identity(){return {1,0,0,0,1,0,0,0,1,0,0,0};}
// Same row-vector helpers as BackdropScene.cpp (copied so the census depends on no production seam).
Matrix compose(const Matrix&a,const Matrix&b){Matrix o{};for(int i=0;i<3;++i){for(int j=0;j<3;++j)for(int k=0;k<3;++k)o[3*i+j]+=a[3*i+k]*b[3*k+j];o[9+i]=b[9+i];for(int k=0;k<3;++k)o[9+i]+=a[9+k]*b[3*k+i];}return o;}
BillboardAffine3x4 affine(const Matrix& m){return {{{m[0],m[3],m[6],m[9],m[1],m[4],m[7],m[10],m[2],m[5],m[8],m[11]}}};}
BillboardAffine3x4 multiply(const BillboardAffine3x4&a,const BillboardAffine3x4&b){BillboardAffine3x4 o;for(int r=0;r<3;++r){for(int c=0;c<3;++c)for(int k=0;k<3;++k)o.values[4*r+c]+=a.values[4*r+k]*b.values[4*k+c];o.values[4*r+3]=a.values[4*r+3];for(int k=0;k<3;++k)o.values[4*r+3]+=a.values[4*r+k]*b.values[4*k+3];}return o;}

enum class Cls{Ok,Unsupported,Malformed};
const char* clsName(Cls c){return c==Cls::Ok?"ok":c==Cls::Unsupported?"unsupported":"malformed";}
// Explicit classification rule under test: a reader/adapter message beginning a
// form diagnosis with "unsupported" is an unsupported form; anything else
// (truncation, bounds, marker/end contradictions, bad references) is malformed.
Cls classify(const std::string& msg){return msg.find("unsupported")!=std::string::npos?Cls::Unsupported:Cls::Malformed;}

struct RootCensus{
  std::string role; // Primary / Secondary / LegacyUnattributed owner[index]
  std::size_t begin=0,end=0,nodes=0,meshes=0,billboards=0;
  std::uint64_t treeHash=0;
  Cls cls=Cls::Ok;std::string reason;
  std::set<std::string> textures;
};
struct LevelCensus{
  std::string id;std::size_t bytes=0;
  Cls cls=Cls::Ok;std::string firstReason,firstRole;
  std::vector<RootCensus> roots;
  std::size_t missingTextures=0;
};
std::uint64_t fnv(std::span<const std::byte> b){std::uint64_t h=1469598103934665603ull;for(auto v:b)h=(h^std::to_integer<std::uint8_t>(v))*1099511628211ull;return h;}
void walk(const SceneNode& n,RootCensus& r){
  ++r.nodes;r.billboards+=n.type==2;for(const auto& b:n.meshes)r.meshes+=b.records.size();
  for(const auto& m:n.materials)if(!m.name.empty())r.textures.insert(m.name);
  for(const auto& c:n.children)walk(c,r);
}
// scene-branch-impl-001: independent mirror of the branch-1 sprite admission rule (contract
// section 10): sprite file present and parsing, clip in range with >=1 temporal entry, every
// temporal entry exactly one directional image, texture flags row 1/2 with the texture present.
// Written from the contract, not from BackdropScene.cpp. Without a library it is unsupported.
const AssetLibrary* censusLibrary=nullptr;
std::string firstSpriteFailure(const SceneNode& root,bool secondary){
  std::string err;
  const auto go=[&](const auto& self,const SceneNode& n)->void{
    if(!err.empty())return;
    if(n.type==2&&n.type2.branch!=0){
      const auto no=[&](const std::string& why){err=std::format("unsupported type-2 branch 1 sprite '{}': {}",n.type2.branchName,why);};
      if(!secondary||!censusLibrary){no(secondary?"no asset library":"Secondary stage only");return;}
      const std::string asset="SPRITES\\"+n.type2.branchName+".SPR";
      if(!censusLibrary->contains(asset)){no("error: sprite file "+asset+" is missing");return;}
      auto sheet=loadSpriteSheet(*censusLibrary,asset);
      if(!sheet){no("error: does not parse");return;}
      const auto clipIndex=n.type2.branchWords[1];
      if(clipIndex>=sheet->sprites.size()){no("clip index outside the sheet");return;}
      const auto& clip=sheet->sprites[clipIndex];
      if(clip.animations.empty()){no("clip has no temporal entries");return;}
      for(const auto& a:clip.animations){
        if(a.frames.size()!=1){no(std::format("{} directional images; multi-image selection not implemented",a.frames.size()));return;}
        const auto leaf=a.frames[0].sheet.substr(a.frames[0].sheet.find_last_of('\\')==std::string::npos?0:a.frames[0].sheet.find_last_of('\\')+1);
        if(leaf.empty()||(leaf[0]!='_'&&leaf[0]!='$')){no("texture flags row not implemented");return;}
        if(!censusLibrary->contains(sheetAssetName(a.frames[0].sheet))){no("error: texture is missing");return;}
      }
    }
    for(const auto& c:n.children)self(self,c);
  };
  go(go,root);return err;
}
std::string firstBillboardFailure(const SceneNode& root,const BillboardSourceInputs& chain){
  std::string err;
  const auto go=[&](const auto& self,const SceneNode& n,const Matrix& ancestors)->void{
    if(!err.empty())return;
    if(n.type==2&&n.type2.branch==0){auto in=chain;in.parentCamera=multiply(chain.parentCamera,affine(ancestors));auto b=makeSceneBillboardSource(n,in);if(!b)err=std::format("billboard adapter at 0x{:x}: {}",n.sourceOffset,b.error());}
    const Matrix next=compose(n.matrix,ancestors);
    for(const auto& c:n.children)self(self,c,next);
  };
  go(go,root,identity());return err;
}
RootCensus evaluate(std::span<const std::byte> bytes,const std::string& role,bool secondary,const lemmings::ObjectSourceExtent& e,bool marker,const BillboardSourceInputs& chain){
  RootCensus r;r.role=role;r.begin=e.typed_root_begin;r.end=e.tree_end;
  const auto fail=[&](Cls c,std::string why){r.cls=c;r.reason=std::move(why);return r;};
  if(!marker)return fail(Cls::Malformed,"Object 09 marker mismatch");
  auto parsed=readSceneNode(bytes,e.typed_root_begin);
  if(!parsed)return fail(classify(parsed.error()),parsed.error());
  if(parsed->endOffset!=e.tree_end)return fail(Cls::Malformed,std::format("recursive tree end 0x{:x} differs from serialized reader 0x{:x}",parsed->endOffset,e.tree_end));
  if(e.tree_end<=bytes.size()&&e.typed_root_begin<=e.tree_end)r.treeHash=fnv(bytes.subspan(e.typed_root_begin,e.tree_end-e.typed_root_begin));
  walk(parsed->root,r);
  // scene-nested-impl-001: SUPERSEDED predicate (was: root type 0 and every direct child a childless type-1/2 leaf, i.e.
  // "unsupported nested/non-leaf secondary dispatch"). Independent mirror of the structural rule: type-0 root, node types
  // 0..2, and children only under type-0/type-1 nodes (a type-2 owner's double matrix push is not implemented).
  if(secondary){
    const auto badForm=[](const auto& self,const SceneNode& n)->bool{
      if(n.type>2||(n.type==2&&!n.children.empty()))return true;
      return std::any_of(n.children.begin(),n.children.end(),[&](const auto& c){return self(self,c);});
    };
    if(parsed->root.type!=0||badForm(badForm,parsed->root))return fail(Cls::Unsupported,"unsupported secondary node form");
  }
  if(auto bad=firstSpriteFailure(parsed->root,secondary);!bad.empty())return fail(classify(bad),bad);
  auto meshes=buildSceneMeshSource(parsed->root,identity(),identity());
  if(!meshes)return fail(classify(meshes.error()),"mesh adapter: "+meshes.error());
  if(auto bad=firstBillboardFailure(parsed->root,chain);!bad.empty())return fail(classify(bad),bad);
  return r;
}
LevelCensus census(std::span<const std::byte> bytes,const std::string& id,const BillboardSourceInputs& chain){
  LevelCensus L;L.id=id;L.bytes=bytes.size();
  const auto note=[&](const RootCensus& r){if(r.cls!=Cls::Ok&&L.cls==Cls::Ok){L.cls=r.cls;L.firstReason=r.reason;L.firstRole=r.role;}L.roots.push_back(r);};
  lemmings::CompiledLevel level;
  try{level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});}
  catch(const std::invalid_argument& e){L.cls=Cls::Malformed;L.firstRole="container";L.firstReason=std::string("serialized level: ")+e.what();return L;}
  note(evaluate(bytes,"Primary paired-primary[0]",false,level.objects.source_extent,level.objects.marker_matches,chain));
  if(level.objects.second_object_source_extent)note(evaluate(bytes,"Secondary paired-secondary[0]",true,*level.objects.second_object_source_extent,level.objects.second_object_marker_matches,chain));
  const auto models=[&](const auto& entities,const char* owner){
    for(std::size_t i=0;i<entities.size();++i){
      const auto& model=entities[i].model;if(!model)continue;
      const std::string role=std::format("LegacyUnattributed {}[{}]",owner,i);
      if(model->type!=0||!model->object){RootCensus r;r.role=role;r.cls=Cls::Unsupported;r.reason=std::format("unsupported present model type {} / missing inline object",model->type);note(r);continue;}
      note(evaluate(bytes,role,false,model->object->source_extent,model->object->marker_matches,chain));
    }
  };
  models(level.gadgets.entities_425e50,"entities_425e50");
  models(level.gadgets.entities_426c6b,"entities_426c6b");
  return L;
}
std::string idOf(const std::string& name){auto s=name.find_last_of("\\/");auto base=s==std::string::npos?name:name.substr(s+1);auto dot=base.rfind('.');return dot==std::string::npos?base:base.substr(0,dot);}
BillboardSourceInputs defaultChain(){
  const auto camera=buildCamera(CameraParams{});const auto built=buildBillboardCameraChain(camera);
  BillboardSourceInputs chain;chain.parentCamera.values=built.parentCamera;chain.sourceTransform.values=built.sourceTransform;chain.projectionTransform.values=built.projectionTransform;chain.center={camera.cx,camera.cy};
  return chain;
}
}

namespace sceneadmission {
Outcome structuralOutcome(std::span<const std::byte> levelBytes,const webo::AssetLibrary* library){
  censusLibrary=library;
  const auto l=census(levelBytes,"",defaultChain());
  return {l.cls==Cls::Ok,l.cls==Cls::Malformed,l.firstRole,l.firstReason};
}
}

#ifndef SCENE_ADMISSION_NO_MAIN
int main()try{
  const char* gameDir=std::getenv("WEBOLUTION_GAME_DIR");
  if(!gameDir||!*gameDir){std::cout<<"SKIP: WEBOLUTION_GAME_DIR unset\n";return 0;}
  auto lib=AssetLibrary::open(fs::path(gameDir)/"lemmings.box",fs::path(gameDir)/"patch.box");
  if(!lib){std::cerr<<"FAIL open archives: "<<lib.error()<<'\n';return 1;}
  const auto camera=buildCamera(CameraParams{});const auto built=buildBillboardCameraChain(camera);
  BillboardSourceInputs chain;chain.parentCamera.values=built.parentCamera;chain.sourceTransform.values=built.sourceTransform;chain.projectionTransform.values=built.projectionTransform;chain.center={camera.cx,camera.cy};
  std::vector<LevelCensus> levels;
  censusLibrary=&*lib;
  for(const auto& name:levelFileNames(*lib)){
    auto bytes=lib->read(name);if(!bytes){std::cerr<<"FAIL read "<<name<<": "<<bytes.error()<<'\n';return 1;}
    levels.push_back(census(*bytes,idOf(name),chain));
  }
  std::sort(levels.begin(),levels.end(),[](const auto&a,const auto&b){return a.id<b.id;});
  std::ostringstream out;
  out<<"scene-admission-001 step-1 structural census (source-built; not native admission)\n";
  out<<"effectiveLevels="<<levels.size()<<'\n';
  const LevelCensus* climb=nullptr;for(const auto& l:levels)if(l.id=="MM_CLIMB_002")climb=&l;
  if(!climb||climb->roots.size()<2){std::cerr<<"FAIL MM_CLIMB_002 reference census missing\n";return 1;}
  const auto climbPrimary=climb->roots[0].treeHash,climbSecondary=climb->roots[1].treeHash;
  std::map<Cls,std::size_t> counts;std::map<std::string,std::vector<std::string>> reasons;
  std::vector<std::string> okIds,primaryIdentical,secondaryIdentical,bothIdentical;
  std::map<std::pair<std::uint64_t,std::uint64_t>,std::vector<std::string>> groups;
  for(auto& l:levels){
    ++counts[l.cls];
    for(const auto& t:l.roots)for(const auto& n:t.textures)if(!lib->contains("TEXTURES\\"+n+".BMP"))++l.missingTextures;
    out<<"LEVEL "<<l.id<<" bytes="<<l.bytes<<" outcome="<<clsName(l.cls);
    if(l.cls!=Cls::Ok)out<<" firstRole=\""<<l.firstRole<<"\" reason=\""<<l.firstReason<<'"';
    out<<" roots="<<l.roots.size()<<" missingTextureNames="<<l.missingTextures<<'\n';
    for(const auto& r:l.roots)out<<"  ROOT "<<r.role<<" status="<<clsName(r.cls)<<" typedRoot=0x"<<std::hex<<r.begin<<" treeEnd=0x"<<r.end<<std::dec<<" nodes="<<r.nodes<<" meshRecords="<<r.meshes<<" billboards="<<r.billboards<<" treeFnv=0x"<<std::hex<<r.treeHash<<std::dec<<(r.cls!=Cls::Ok?" reason=\""+r.reason+"\"":"")<<'\n';
    if(l.cls!=Cls::Ok)reasons[l.firstRole.substr(0,l.firstRole.find(' '))+": "+l.firstReason.substr(0,l.firstReason.find(" at 0x"))].push_back(l.id);
    if(l.cls==Cls::Ok){
      okIds.push_back(l.id);
      const bool p=l.roots[0].treeHash==climbPrimary,s=l.roots.size()>1&&l.roots[1].role.starts_with("Secondary")&&l.roots[1].treeHash==climbSecondary;
      if(p)primaryIdentical.push_back(l.id);if(s)secondaryIdentical.push_back(l.id);if(p&&s)bothIdentical.push_back(l.id);
      groups[{l.roots[0].treeHash,l.roots.size()>1&&l.roots[1].role.starts_with("Secondary")?l.roots[1].treeHash:0}].push_back(l.id);
    }
  }
  const auto join=[](const std::vector<std::string>& v){std::string s;for(const auto& x:v)s+=(s.empty()?"":",")+x;return s;};
  out<<"COUNTS ok="<<counts[Cls::Ok]<<" unsupported="<<counts[Cls::Unsupported]<<" malformed="<<counts[Cls::Malformed]<<'\n';
  for(const auto& [k,v]:reasons)out<<"REASON \""<<k<<"\" levels="<<v.size()<<": "<<join(v)<<'\n';
  out<<"SUPPORTED "<<join(okIds)<<'\n';
  out<<"IDENTICAL_PRIMARY_TO_CLIMB "<<join(primaryIdentical)<<'\n';
  out<<"IDENTICAL_SECONDARY_TO_CLIMB "<<join(secondaryIdentical)<<'\n';
  out<<"IDENTICAL_PRIMARY_AND_SECONDARY_TO_CLIMB "<<join(bothIdentical)<<'\n';
  out<<"SUPPORTED_SCENERY_TREE_GROUPS(primaryFnv,secondaryFnv)="<<groups.size()<<'\n';
  for(const auto& [k,v]:groups)out<<"  GROUP 0x"<<std::hex<<k.first<<",0x"<<k.second<<std::dec<<" members="<<v.size()<<": "<<join(v)<<'\n';
  std::vector<std::string> extra;for(const auto& id:okIds)if(id!="MM_CLIMB_002"&&id!="MM_BASH_002")extra.push_back(id);
  out<<"SUPPORTED_BEYOND_ADMITTED_TWO="<<extra.size()<<": "<<join(extra)<<'\n';
  const bool bothAdmitted=std::find(okIds.begin(),okIds.end(),"MM_CLIMB_002")!=okIds.end()&&std::find(okIds.begin(),okIds.end(),"MM_BASH_002")!=okIds.end();
  out<<"ADMITTED_TWO_PASS_PIPELINE="<<bothAdmitted<<'\n';
  if(extra.empty())out<<"STOP_CONDITION_A: no installed level beyond the two admitted passes the structural pipeline\n";
  std::size_t endMismatch=0;for(const auto& l:levels)for(const auto& r:l.roots)if(r.reason.find("differs from serialized reader")!=std::string::npos)++endMismatch;
  out<<"EXACT_END_MISMATCHES="<<endMismatch<<'\n';
  if(endMismatch)out<<"STOP_CONDITION_B_CANDIDATE: exact-end mismatch present; inspect per-level lines\n";
  const fs::path dir=SCENE_ADMISSION_DIR;fs::create_directories(dir);
  std::ofstream(dir/"census.txt")<<out.str();
  std::cout<<out.str();
  // The census itself must have found the two accepted levels supported.
  if(!bothAdmitted){std::cerr<<"FAIL: an accepted level does not pass the structural pipeline\n";return 1;}
  std::cout<<"PASS sceneadmission_census\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL sceneadmission_census: "<<e.what()<<'\n';return 1;}
#endif
