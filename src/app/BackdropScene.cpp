#include "app/RasterRows.h"
#include "app/FrameProfile.h"
#include "app/BackdropScene.h"
#include "app/ExitBalloon.h"
#include "app/Raster.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneMeshSource.h"
#include "app/SceneMaterialFlags.h"
#include "app/SceneDepth.h"
#include "app/SceneDiffuse.h"
#include "app/SceneDispatch.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/LevelObjects.h"
#include "sim/World.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <map>
#include <cmath>
#include <cstdio>
#include <format>
#include <stdexcept>
#include "lemmings/compiled_level.hpp"

namespace webo::app {
namespace {
// Copy, draw and compare only bands with triangles. Each changed pixel counts once.
template<int Channels, class T>
std::vector<std::string> flushCountingChanges(detail::RasterCommandBatch& batch,
    const std::vector<T>& buffer, int width, std::size_t& total) {
  std::atomic<std::size_t> changed{};
  auto errors = batch.flush([&](int first, int last, auto draw) {
    const auto start = buffer.begin() + std::size_t(first) * width * Channels;
    const auto end = buffer.begin() + std::size_t(last + 1) * width * Channels;
    thread_local std::vector<T> before;
    before.assign(start, end);
    draw();
    std::size_t count = 0;
    for (std::size_t i = 0; i < before.size(); i += Channels) {
      if constexpr (Channels == 1) count += start[i] != before[i];
      else count += !std::equal(before.data()+i, before.data()+i+Channels, &start[i]);
    }
    changed.fetch_add(count, std::memory_order_relaxed);
  });
  total += changed.load(std::memory_order_relaxed);
  return errors;
}

std::string context(std::string_view level, const BackdropScene::RootSource& root,
                    std::size_t offset) {
 return std::format("{} {} {}[{}] source=0x{:x}: ", level,
     root.stage==BackdropScene::Stage::Primary?"Primary":
     root.stage==BackdropScene::Stage::Secondary?"Secondary":
     root.stage==BackdropScene::Stage::FluidPipe?"FluidPipe":
     root.stage==BackdropScene::Stage::FluidGate?"FluidGate":"LegacyUnattributed",
     root.owner,root.entityIndex,offset);
}
std::array<double,12> identity(){return {1,0,0,0,1,0,0,0,1,0,0,0};}
BillboardAffine3x4 affine(const std::array<double,12>& m){return {{{m[0],m[3],m[6],m[9],m[1],m[4],m[7],m[10],m[2],m[5],m[8],m[11]}}};}
using Matrix=std::array<double,12>;
Matrix compose(const Matrix&a,const Matrix&b){Matrix o{};for(int i=0;i<3;++i){for(int j=0;j<3;++j)for(int k=0;k<3;++k)o[3*i+j]+=a[3*i+k]*b[3*k+j];o[9+i]=b[9+i];for(int k=0;k<3;++k)o[9+i]+=a[9+k]*b[3*k+i];}return o;}
constexpr std::string_view mineBackdropMaterials = "Models\\bg_mine01\\";
constexpr std::string_view gravityShaftMaterial = R"(Models\anti_grav\~shaft_antigrav)";
bool isBackdropWaterMaterial(std::string_view name) {
  if (name == R"(Models\bg_sand_medium\~Water1)") return true;
  if (!name.starts_with(mineBackdropMaterials)) return false;
  name.remove_prefix(mineBackdropMaterials.size());
  return name == "~Water01" || name == "^WaterFall01" || name == "~Splash01" ||
         name == "WaterRef01" || name == "WaterRef02";
}

// 0x47645E -> 0x46BAFE builds the target table in preorder, including the root.
template<class Node>
void collectSceneNodes(Node& node, std::vector<Node*>& nodes) {
  nodes.push_back(&node);
  for (auto& child : node.children) collectSceneNodes<Node>(child, nodes);
}

std::array<double, 2> sampleUvOffset(std::span<const GravityPresentation::Key> keys, double time) {
  // 0x476FB0 / 0x476A80: select the key ending at or after time, then
  // evaluate its base UV plus the interval fraction times its stored delta.
  auto key = std::ranges::lower_bound(keys, time, {}, &GravityPresentation::Key::end);
  if (key == keys.end()) --key;
  const double start = key == keys.begin() ? 0.0 : (key - 1)->end;
  const double t = key->end > start ? std::clamp((time - start) / (key->end - start), 0.0, 1.0) : 0.0;
  return {key->u + t * key->du, key->v + t * key->dv};
}

void setSceneSpin(Matrix& matrix, std::uint32_t axis, double turns) {
  const double angle = turns * 2 * 3.1415927410125732; // 0x4B0F88
  const double c = std::cos(angle), s = std::sin(angle);
  // 0x476EE0 selects Y, X or Z. The setters overwrite the basis and retain the translation.
  const std::array<double, 9> rotation = axis == 0 ?
      std::array<double, 9>{c, 0, s, 0, 1, 0, -s, 0, c} : axis == 1 ?
      std::array<double, 9>{1, 0, 0, 0, c, -s, 0, s, c} :
      std::array<double, 9>{c, -s, 0, s, c, 0, 0, 0, 1};
  std::copy(rotation.begin(), rotation.end(), matrix.begin());
}

bool hasMineBackdropMaterials(const SceneNode& node) {
  return std::ranges::any_of(node.materials, [](const SceneMaterial& material) {
    return material.name.starts_with(mineBackdropMaterials);
  }) || std::ranges::any_of(node.children, hasMineBackdropMaterials);
}
BillboardAffine3x4 multiply(const BillboardAffine3x4&a,const BillboardAffine3x4&b){BillboardAffine3x4 o;for(int r=0;r<3;++r){for(int c=0;c<3;++c)for(int k=0;k<3;++k)o.values[4*r+c]+=a.values[4*r+k]*b.values[4*k+c];o.values[4*r+3]=a.values[4*r+3];for(int k=0;k<3;++k)o.values[4*r+3]+=a.values[4*r+k]*b.values[4*k+3];}return o;}
std::array<double,3> eyeLocal(const Matrix&m){double a=m[0],b=m[1],c=m[2],d=m[3],e=m[4],f=m[5],g=m[6],h=m[7],i=m[8];double det=a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g);if(std::abs(det)<1e-15)return {NAN,NAN,NAN};double q=1/det;double inv[9]={(e*i-f*h)*q,(c*h-b*i)*q,(b*f-c*e)*q,(f*g-d*i)*q,(a*i-c*g)*q,(c*d-a*f)*q,(d*h-e*g)*q,(b*g-a*h)*q,(a*e-b*d)*q};std::array<double,3> out{};for(int j=0;j<3;++j)for(int k=0;k<3;++k)out[j]-=m[9+k]*inv[3*k+j];return out;}
Matrix nativeAffine(const BillboardAffine3x4& a){return {a.values[0],a.values[4],a.values[8],a.values[1],a.values[5],a.values[9],a.values[2],a.values[6],a.values[10],a.values[3],a.values[7],a.values[11]};}
// Convert the camera parent transform and source S to SceneNode row-vector layout
// before composing the facing predicate.
Matrix predicateContext(const CameraBasis& camera){const auto chain=buildBillboardCameraChain(camera);return compose(nativeAffine(BillboardAffine3x4{chain.parentCamera}),nativeAffine(BillboardAffine3x4{chain.sourceTransform}));}
void collectEyes(const SceneNode& n,const Matrix& parent,const Matrix& context,std::map<std::size_t,std::array<double,3>>& out){auto m=compose(n.matrix,parent);out[n.sourceOffset]=eyeLocal(compose(m,context));for(auto&c:n.children)collectEyes(c,m,context,out);}
bool passesFacingPredicate(const SceneMeshRecordSource& rec,
                           const std::map<std::size_t, std::array<double, 3>>& eyes,
                           bool gravityEffect = false) {
  // Kinds 4/5 (0x46AE20/0x46AFA0) and kind-7 emitters (0x46B2A0/0x46B350)
  // use the authored normal and first indexed vertex.
  const bool singleSidedQuad = rec.kind == 7 &&
      (gravityEffect || rec.materialName == R"(Models\bg_mine01\^WaterFall01)" ||
       rec.materialName == R"(Models\bg_tree04\Background_01)");
  if ((rec.kind != 4 && rec.kind != 5 && !singleSidedQuad) || rec.corners.empty()) return true;
  const auto eye = eyes.find(rec.nodeSourceOffset);
  if (eye == eyes.end()) return false;
  const auto& e = eye->second;
  const auto& v = rec.corners.front().localPosition;
  const auto& n = rec.corners.front().localNormal;
  return (e[0] - v[0]) * n[0] + (e[1] - v[1]) * n[1] + (e[2] - v[2]) * n[2] > 0;
}
SceneRasterOptions options(bool alpha,bool blend,bool depthWrite=true){return {SceneFilter::Linear,SceneAddress::Wrap,alpha?SceneAlphaFunction::StrictGreater:SceneAlphaFunction::Disabled,0xef,blend,SceneBlendFactor::SrcAlpha,SceneBlendFactor::InvSrcAlpha,SceneDepthFunction::Less,depthWrite,SceneShade::Gouraud,SceneRgbOperation::Modulate,SceneAlphaOperation::Modulate,0.5,0.5};}
SceneRgbaImage rgba(Image&& im){return {im.width,im.height,std::move(im.rgba)};}

// 0x474485 selects texture flags from the first character of the basename.
std::uint32_t textureNameFlags(std::string_view name) {
  const auto slash = name.find_last_of('\\');
  if (slash != std::string_view::npos) name.remove_prefix(slash + 1);
  if (name.empty()) return 0;
  switch (name.front()) {
    case '_': return 1;
    case '$': return 2;
    case '~': return 4;
    case '^': return 8;
    case '+': return 16;
    default: return 0;
  }
}
// 0x414D80 uses the same blend selector for mesh materials and sprite nodes.
SceneRasterOptions blendOptions(bool alphaTest, std::uint32_t selector) {
  auto state = options(alphaTest, selector != 0);
  if (selector == 2) state.sourceBlend = SceneBlendFactor::One;
  if (selector == 2 || selector == 3) state.destinationBlend = SceneBlendFactor::One;
  return state;
}

SceneRasterOptions spriteOptions(std::uint32_t selector){
 auto o=blendOptions(true,selector);
 // For texture flags 1, the `flags & 3` row of 0x0047261F uses SELECTARG1 for alpha.
 // GREATER uses reference 0xEF and RGB modulates white diffuse.
 o.alphaOperation=SceneAlphaOperation::SelectTexture;
 return o;
}
}
std::expected<BackdropScene,std::string> BackdropScene::build(const AssetLibrary& lib,std::span<const std::byte> bytes,std::string_view levelId){
 BackdropScene s;
 s.levelId_=levelId;
 // The clone admits scenery by supported serialized structure. The caller's
 // byte storage must outlive both readers.
 lemmings::CompiledLevel level;
 try {
   level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
 } catch(const std::invalid_argument& e) {
   return std::unexpected(std::string(levelId)+" serialized level: "+e.what());
 }
 // Malformed data is an error. Unsupported forms produce an empty partial scene
 // with a diagnostic. The reader identifies these with "unsupported".
 struct Failure{bool unsupported;std::string message;};
 const auto formFailure=[](const std::string& label,const std::string& reason){return std::unexpected(Failure{reason.find("unsupported")!=std::string::npos,label+reason});};
 // Type-2 branch-1 binds a valid SPRITES\<name>.SPR clip and supported texture.
 // Each entry needs one directional image (0x0046840B early return).
 // Multi-image selection (0x00418350/0x00480AC8) is unsupported.
 // A missing sprite file produces a diagnostic and an empty partial scene.
 const auto admitSprites=[&](const RootSource& source,const SceneNode& root)->std::expected<void,Failure>{
   std::vector<const SceneNode*> found;
   const auto collect=[&](const auto& self,const SceneNode& n)->void{
     if(n.type==2&&n.type2.branch!=0)found.push_back(&n);
     for(const auto& c:n.children)self(self,c);
   };collect(collect,root);
   for(const SceneNode* n:found){
     const std::string& name=n->type2.branchName;
     const auto reject=[&](const std::string& why){return std::unexpected(Failure{true,context(levelId,source,n->sourceOffset)+"unsupported type-2 branch 1 sprite '"+name+"': "+why});};
     if(source.stage!=Stage::Secondary)return reject("branch-1 sprite scenery is implemented for the Secondary stage only");
     const std::string asset="SPRITES\\"+name+".SPR";
     if(!lib.contains(asset))return reject("error: sprite file "+asset+" is missing from the effective archive");
     auto it=s.sheets_.find(name);
     if(it==s.sheets_.end()){
       auto sheet=loadSpriteSheet(lib,asset);
       if(!sheet)return reject("error: sprite file does not parse: "+sheet.error());
       it=s.sheets_.emplace(name,std::move(*sheet)).first;
     }
     const auto& sheet=it->second;
     const std::uint32_t clipIndex=n->type2.branchWords[1];
     if(clipIndex>=sheet.sprites.size())return reject(std::format("clip index {} outside the {} clips",clipIndex,sheet.sprites.size()));
     const auto& clip=sheet.sprites[clipIndex];
     if(clip.animations.empty())return reject(std::format("clip {} has no temporal entries",clipIndex));
     if(!std::isfinite(clip.rate))return reject("clip rate is not finite");
     for(std::size_t i=0;i<clip.animations.size();++i){
       const auto& frames=clip.animations[i].frames;
       if(frames.size()!=1)return reject(std::format("clip {} temporal entry {} has {} directional images; multi-image selection (0x00418350/0x00480AC8) is not implemented",clipIndex,i,frames.size()));
       if((textureNameFlags(frames[0].sheet)&3)==0)return reject("texture '"+frames[0].sheet+"' flags row is not implemented");
       if(!lib.contains(sheetAssetName(frames[0].sheet)))return reject("error: texture "+sheetAssetName(frames[0].sheet)+" is missing");
     }
     SpriteNode node;node.sheetName=name;node.clip=clipIndex;node.mirror=n->type2.branchWords[0]!=0;
     s.sprites_.emplace(n->sourceOffset,std::move(node));
   }
   return {};
 };
 const auto add=[&](RootSource source,bool marker,double projectionBias=0.0,bool mirror=false)->std::expected<void,Failure>{
   const auto& e=source.extent;
   const auto label=context(levelId,source,e.typed_root_begin);
   if(!marker)return std::unexpected(Failure{false,label+"Object 09 marker mismatch"});
   auto parsed=readSceneNode(bytes,e.typed_root_begin);
   if(!parsed)return formFailure(label,parsed.error());
   if(parsed->endOffset!=e.tree_end)return std::unexpected(Failure{false,label+"recursive tree end differs from serialized reader"});
   // 0x437DBD / 0x437E56: the paired holder flag negates the root's Z basis.
   // Scope the presentation change to the shared mine backdrop.
   if(mirror && hasMineBackdropMaterials(parsed->root))
     for (std::size_t i : {6, 7, 8}) parsed->root.matrix[i] = -parsed->root.matrix[i];
   // Secondary slot8 requires a type-0 root and nodes of types 0..2.
   // Type-1 owners draw meshes before children in file order (0x0046C55D).
   // Type-2 owners apply their matrix twice to children (0x47D0F2 and 0x46B6EB).
   // The clone rejects that ownership and returns an empty partial scene.
   if(source.stage==Stage::Secondary){
     if(parsed->root.type!=0)return std::unexpected(Failure{true,label+"unsupported secondary root node type "+std::to_string(parsed->root.type)});
     std::string rejected;
     const auto check=[&](const auto& self,const SceneNode& n)->void{
       if(!rejected.empty())return;
       if(n.type>2){rejected=std::format("unsupported secondary node type {} at 0x{:x}",n.type,n.sourceOffset);return;}
       if(n.type==2&&!n.children.empty()){rejected=std::format("unsupported type-2 node with children at 0x{:x} (double matrix push 0x47D0F2/0x46B6EB is not implemented)",n.sourceOffset);return;}
       for(const auto& c:n.children)self(self,c);
     };check(check,parsed->root);
     if(!rejected.empty())return std::unexpected(Failure{true,label+rejected});
   }
   if(auto sprites=admitSprites(source,parsed->root);!sprites)return sprites;
   // Validate the adapters too: a successful SceneNode parse alone must not
   // conceal invalid material/vertex/normal references or unsupported meshes.
   auto meshes=buildSceneMeshSource(parsed->root,identity(),identity());
   if(!meshes)return formFailure(label,meshes.error());
   s.roots_.push_back({std::move(source),std::move(parsed->root),projectionBias});
   return {};
 };
 const auto models=[&](const auto& entities,std::string_view owner)->std::expected<void,Failure>{
   for(std::size_t i=0;i<entities.size();++i){
     const auto& model=entities[i].model;
     if(!model)continue;
     if(model->type!=0 || !model->object)
       return std::unexpected(Failure{true,std::format("{} LegacyUnattributed {}[{}]: unsupported present model type {} / missing inline object",levelId,owner,i,model->type)});
     auto result=add({Stage::LegacyUnattributed,std::string(owner),i,model->object->source_extent},model->object->marker_matches);
     if(!result)return result;
   }
   return {};
 };
 const auto extract=[&]()->std::expected<void,Failure>{
   auto ok=add({Stage::Primary,"paired-primary",0,level.objects.source_extent},level.objects.marker_matches,0.0,level.objects.first_flag);
   if(!ok)return ok;
   if(level.objects.second_object_source_extent){
     ok=add({Stage::Secondary,"paired-secondary",0,*level.objects.second_object_source_extent},level.objects.second_object_marker_matches,0.0,level.objects.second_object_first_flag);
     if(!ok)return ok;
   }
   ok=models(level.gadgets.entities_425e50,"entities_425e50");
   if(!ok)return ok;
   return models(level.gadgets.entities_426c6b,"entities_426c6b");
 };
 if(auto extracted=extract();!extracted){
   if(!extracted.error().unsupported)return std::unexpected(extracted.error().message);
   // Never retain a supported subset: a partially extracted level would draw
   // some roles and silently omit the rest.
   s.roots_.clear();s.sheets_.clear();s.sprites_.clear();
   s.diagnostics_.push_back(extracted.error().message+"; scenery not admitted: empty partial scene");
   std::fprintf(stderr,"backdrop: %s\n",s.diagnostics_.back().c_str());
 }
 s.bindSecondaryAnimation(level.objects);
 // Route tagged type-0 pads, type-1 levers and type-3 gravity models through
 // the same depth path as other placed scenery, retaining each root.
 for(std::size_t i=0;i<level.gadgets.tagged_entities.size();++i){
   const auto& gadget=level.gadgets.tagged_entities[i];
   if(gadget.type!=0&&gadget.type!=1&&gadget.type!=3&&gadget.type!=4)continue;
   const auto& model=gadget.model;
   if(!model||model->type!=0||!model->object){
     s.diagnostics_.push_back(std::format("{} tagged type {}[{}]: unsupported model",levelId,gadget.type,i));
     continue;
   }
   auto added=add({Stage::LegacyUnattributed,gadget.type==0?"tagged-pressure-pad":gadget.type==3?"tagged-gravity":gadget.type==4?"tagged-speed-pad":"tagged-lever",i,model->object->source_extent},model->object->marker_matches,
       gadget.type==4?std::bit_cast<double>(model->double_bits[2]):0.0);
   if(!added)return std::unexpected(added.error().message);
 }
 // 4347EC owns a base and two separately enabled FX models (434AAF).
 for(std::size_t i=0;i<level.gadgets.entities_4347ec.size();++i){
   const auto& entity=level.gadgets.entities_4347ec[i];
   const lemmings::LevelModel* models[]{entity.model?&*entity.model:nullptr,
       &entity.models_1c8_1cc[0],&entity.models_1c8_1cc[1]};
   for(int role=0;role<3;++role){
     const auto* model=models[role];
     if(!model||model->type!=0||!model->object)
       return std::unexpected(std::string(levelId)+": unsupported teleport model");
     auto added=add({Stage::LegacyUnattributed,role==0?"teleport-base":role==1?"teleport-fx-0":"teleport-fx-1",
         i,model->object->source_extent},model->object->marker_matches,std::bit_cast<double>(model->double_bits[2]));
     if(!added)return std::unexpected(added.error().message);
     if(role) s.roots_.back().visible=false;
   }
 }
 // Named STAT entities own the installed Crab.a3d models and their cylinder placement.
 for(std::size_t i=0;i<level.gadgets.named_entities.size();++i){
   const auto& entity=level.gadgets.named_entities[i];
   const auto& model=entity.model;
   if(!model||model->name!="Crab.a3d")continue;
   if(entity.type!="STAT"||model->type!=0||!model->object)
     return std::unexpected(std::string(levelId)+": unsupported crab model/placement");
   auto added=add({Stage::LegacyUnattributed,"named-crab",i,model->object->source_extent},model->object->marker_matches);
   if(!added)return std::unexpected(added.error().message);
 }
 auto crabs=s.crabPresentation(bytes);
 if(!crabs)return std::unexpected(crabs.error());
 s.setCrabStates(*crabs,{});
 // Named STAT and DYNA hazards use the placed-mesh depth path.
 for(std::size_t i=0;i<level.gadgets.named_entities.size();++i){
   const auto& model=level.gadgets.named_entities[i].model;
   if(!model||(model->name!="deathroller.a3d"&&model->name!="buzzsaw.a3d"&&
       model->name!="Scorp.a3d"&&model->name!="robin.a3d"))continue;
   if(model->type!=0||!model->object)
     return std::unexpected(std::string(levelId)+": unsupported named hazard model");
   auto added=add({Stage::LegacyUnattributed,"named-hazard",i,model->object->source_extent},
       model->object->marker_matches,std::bit_cast<double>(model->double_bits[2]));
   if(!added)return std::unexpected(added.error().message);
 }
 auto hazards=s.namedHazardPresentation(bytes);
 if(!hazards)return std::unexpected(hazards.error());
 s.setNamedHazardStates(*hazards,{});
 auto levers=s.leverPresentation(bytes);
 if(!levers)return std::unexpected(levers.error());
 s.setLeverFaces(*levers,{});
 // 0x42C556 submits optional +0x160 drain and +0x164 supply models in receiver
 // order, independent of height and request. The pipes retain their inline
 // placement and have no animation tracks. The Fluid root uses world coordinates.
 for(std::size_t i=0;i<level.gadgets.entities_42acac.size();++i){
   const auto& gate=level.gadgets.entities_42acac[i];
   for(const auto slot:{0x160,0x164}){
     const auto& model=slot==0x160?gate.model_160:gate.model_164;
     if(!model)continue;
     const auto owner=slot==0x160?"fluid-pipe-160":"fluid-pipe-164";
     if(model->type!=0||!model->object){
       s.diagnostics_.push_back(std::format("{} FluidPipe {}[{}]: unsupported model type {}; pipe not admitted",levelId,owner,i,model->type));
       continue;
     }
     const auto& object=*model->object;
     // Model+0xF8 is the third stored double (0x451865). Drains use 0.0003
     // and supplies use zero. This projection bias is independent of depth policy.
     auto added=add({Stage::FluidPipe,owner,i,object.source_extent},object.marker_matches,
                    std::bit_cast<double>(model->double_bits[2]));
     if(!added)return std::unexpected(added.error().message);
   }
 }
 // Admit animated base Fluid models for entities_42acac kinds 0..2
 // (0x428848) when an inline type-0 root begins with the expected type-1 mesh.
 // A binding failure omits only that receiver's surface.
 for(std::size_t i=0;i<level.gadgets.entities_42acac.size();++i){
   const auto& gate=level.gadgets.entities_42acac[i];
   if(!gate.model)continue;
   const auto reject=[&](const std::string& why){s.diagnostics_.push_back(std::format("{} FluidGate entities_42acac[{}]: {}; receiver fluid not admitted",levelId,i,why));};
   if(gate.values_18c_128_168[1]>2){reject(std::format("receiver kind {} has no traced wave row",gate.values_18c_128_168[1]));continue;}
   if(gate.model->type!=0||!gate.model->object){reject("base model is not an inline type-0 object");continue;}
   const auto& object=*gate.model->object;
   if(!object.marker_matches){reject("Object 09 marker mismatch");continue;}
   auto parsed=readSceneNode(bytes,object.source_extent.typed_root_begin);
   if(!parsed){reject("base model parse: "+parsed.error());continue;}
   if(parsed->endOffset!=object.source_extent.tree_end){reject("recursive tree end differs from serialized reader");continue;}
   // The reader forces the root head matrix to identity (vertices are world coordinates).
   parsed->root.matrix=identity();
   const SceneNode* geometry=nullptr;
   const auto first=[&](const auto& self,const SceneNode& n)->void{if(geometry)return;if(n.type==1){geometry=&n;return;}for(const auto& c:n.children)self(self,c);};
   first(first,parsed->root);
   // The mesh needs six vertices, three normals and a kind-5 top at block[1].
   // Its corners reference slots 2..5, which 0x42B24D updates.
   bool layout=geometry&&geometry->positions.size()>=6&&geometry->normals.size()>=3&&geometry->meshes.size()>=2&&geometry->meshes[1].kind==5&&geometry->meshes[1].records.size()>=2;
   if(layout)for(const auto& rec:geometry->meshes[1].records)for(const auto& c:rec.corners)if(c.vertexIndex<2||c.vertexIndex>5)layout=false;
   if(!layout){reject("base model lacks the traced Fluid layout (6 vertices, 3 normals, kind-5 top over vertices 2..5)");continue;}
   auto meshes=buildSceneMeshSource(parsed->root,identity(),identity());
   if(!meshes){reject("mesh adapter: "+meshes.error());continue;}
   FluidRecord record;record.entityIndex=i;
   record.kind=gate.values_18c_128_168[1];
   for(std::size_t k=0;k<4;++k)for(std::size_t j=0;j<3;++j){
     record.posOff[k][j]=std::bit_cast<double>(gate.triples[2*k][j]);
     record.amp[k][j]=std::bit_cast<double>(gate.triples[2*k+1][j]);
   }
   for(std::size_t j=0;j<3;++j)record.baseColour[j]=std::bit_cast<double>(gate.triple_170[j]);
   record.projectionBias=std::bit_cast<double>(gate.model->double_bits[2]);
   s.fluidRoots_.push_back({{Stage::FluidGate,"entities_42acac",i,object.source_extent},std::move(parsed->root),record});
 }
 return s;
}

void BackdropScene::bindSecondaryAnimation(const lemmings::ObjectBasePrefix& objects) {
  const auto root = std::ranges::find_if(roots_, [](const auto& owned) {
    return owned.source.stage == Stage::Secondary;
  });
  // 0x437BDE selects clip zero. The installed water clips have UV-only tracks.
  const auto& clips = objects.second_object_auxiliary_records;
  if (root == roots_.end() || clips.empty()) return;
  const auto& clip = clips.front();
  const auto waterTrackCount = clip.first_count == 0 ? clip.second_count : 0;
  if (waterTrackCount == 0 && clip.spins.empty()) return;

  try {
    const double duration = std::bit_cast<double>(clip.leading_double_bits);
    if (!std::isfinite(duration) || duration <= 0)
      throw std::invalid_argument("invalid duration");
    SecondaryAnimation animation{.root = std::size_t(root - roots_.begin()),
        .duration = duration, .waterTracks = {}, .spins = {}};
    std::vector<const SceneNode*> nodes;
    collectSceneNodes(std::as_const(root->node), nodes);

    for (const auto& spin : clip.spins) {
      const double rate = std::bit_cast<double>(spin.turns_per_second_bits);
      if (spin.node_index >= nodes.size() || !std::isfinite(rate))
        throw std::invalid_argument("invalid spin target or rate");
      animation.spins.push_back({spin.node_index, spin.axis_selector, rate});
    }

    const auto& bytes = clip.serialized_bytes;
    // Skip the duration and the counts of the empty transform and the UV arrays.
    std::size_t cursor = 16;
    const auto integer = [&](std::size_t size) {
      if (cursor > bytes.size() || size > bytes.size() - cursor)
        throw std::invalid_argument("truncated track");
      std::uint64_t value = 0;
      for (std::size_t i = 0; i < size; ++i)
        value |= std::uint64_t(bytes[cursor++]) << (8 * i);
      return value;
    };
    const auto number = [&]() {
      const double value = std::bit_cast<double>(integer(8));
      if (!std::isfinite(value)) throw std::invalid_argument("nonfinite UV key");
      return value;
    };

    // 0x47C93D / 0x47CF5C: keys, material slot, mesh node indices, sprite node indices.
    for (std::uint32_t i = 0; i < waterTrackCount; ++i) {
      if (integer(4) != 0) throw std::invalid_argument("unsupported UV track kind");
      const auto count = integer(4);
      if (!count || count > (bytes.size() - cursor) / 40)
        throw std::invalid_argument("invalid UV key count");
      WaterUvTrack track;
      track.keys.reserve(count);
      double previous = 0.0;
      for (std::size_t k = 0; k < count; ++k) {
        GravityPresentation::Key key{number(), number(), number(), number(), number()};
        if (key.end < previous) throw std::invalid_argument("unordered UV keys");
        previous = key.end;
        track.keys.push_back(key);
      }
      if (previous < duration) throw std::invalid_argument("UV keys end before the clip");
      track.materialSlot = integer(4);
      const auto targets = integer(4);
      if (targets > (bytes.size() - cursor) / 4)
        throw std::invalid_argument("invalid mesh target count");
      for (std::size_t k = 0; k < targets; ++k) {
        const auto node = integer(4);
        if (node >= nodes.size() || track.materialSlot >= nodes[node]->materials.size())
          throw std::invalid_argument("mesh material target outside the scene");
        if (isBackdropWaterMaterial(nodes[node]->materials[track.materialSlot].name))
          track.nodes.push_back(node);
      }
      const auto sprites = integer(4);
      if (sprites > (bytes.size() - cursor) / 4)
        throw std::invalid_argument("invalid sprite target count");
      cursor += sprites * 4;
      if (!track.nodes.empty()) animation.waterTracks.push_back(std::move(track));
    }
    if (!animation.waterTracks.empty() || !animation.spins.empty())
      secondaryAnimation_ = std::move(animation);
  } catch (const std::invalid_argument& e) {
    diagnostics_.push_back(std::format("{} secondary clip: {}", levelId_, e.what()));
  }
}

void BackdropScene::advanceSecondaryAnimation(double dt) {
  if (!secondaryAnimation_ || (dt == 0.0 && secondaryAnimation_->spins.empty())) return;
  auto& animation = *secondaryAnimation_;
  animation.time += dt;
  if (animation.time > animation.duration) {
    const double remainder = std::fmod(animation.time, animation.duration);
    animation.time = remainder == 0.0 ? animation.duration : remainder;
  }
  std::vector<SceneNode*> nodes;
  collectSceneNodes(roots_[animation.root].node, nodes);
  if (dt != 0.0) {
    for (const auto& track : animation.waterTracks) {
      const auto offset = sampleUvOffset(track.keys, animation.time);
      for (const auto node : track.nodes)
        nodes[node]->materials[track.materialSlot].uvOffset = offset;
    }
  }
  // 0x476A6D samples the absolute clip phase, including zero on entry.
  for (const auto& spin : animation.spins)
    setSceneSpin(nodes[spin.node]->matrix, spin.axis, animation.time * spin.turnsPerSecond);
}

std::expected<GravityPresentation,std::string> BackdropScene::gravityPresentation(std::span<const std::byte> bytes) const {
 GravityPresentation result;
 if(std::none_of(roots_.begin(),roots_.end(),[](const auto& r){return r.source.owner=="tagged-gravity";}))return result;
 try {
   const auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
   for(std::size_t root=0;root<roots_.size();++root){
     const auto& owned=roots_[root];if(owned.source.owner!="tagged-gravity")continue;
     const auto& model=*level.gadgets.tagged_entities.at(owned.source.entityIndex).model;
     for(const auto& aux:model.object->auxiliary_records){
       if(aux.name!="anti_grav")continue;
       // At 47C93D/47CF5C the second array holds two-component material tracks.
       // Installed anti_grav has no transform or event tracks.
       std::size_t at=0;const auto& raw=aux.serialized_bytes;
       const auto integer=[&](std::size_t size){
         if(at>raw.size()||size>raw.size()-at)throw std::invalid_argument("truncated gravity UV animation");
         std::uint64_t n=0;for(std::size_t k=0;k<size;++k)n|=std::uint64_t(raw[at++])<<(8*k);return n;
       };
       const auto number=[&](){double n=std::bit_cast<double>(integer(8));if(!std::isfinite(n))throw std::invalid_argument("nonfinite gravity UV key");return n;};
       GravityPresentation::Binding binding{root,number(),{}};
       if(binding.duration<=0||integer(4)!=0)throw std::invalid_argument("unsupported gravity transform tracks/duration");
       std::vector<const SceneNode*> nodes;
       collectSceneNodes(owned.node, nodes);
       const auto count=integer(4);if(count>raw.size()/16)throw std::invalid_argument("invalid gravity track count");
       for(std::size_t i=0;i<count;++i){
         if(integer(4)!=0)throw std::invalid_argument("unsupported gravity UV track id");
         GravityPresentation::Track track;
         const auto keys=integer(4);if(!keys||keys>(raw.size()-at)/40)throw std::invalid_argument("invalid gravity UV key count");
         double previous=0;
         for(std::size_t k=0;k<keys;++k){GravityPresentation::Key key{number(),number(),number(),number(),number()};if(key.end<previous)throw std::invalid_argument("unordered gravity UV keys");previous=key.end;track.keys.push_back(key);}
         if(previous<binding.duration)throw std::invalid_argument("gravity UV keys do not cover duration");
         const auto material=integer(4), targets=integer(4);if(targets>(raw.size()-at)/4)throw std::invalid_argument("invalid gravity UV target count");
         for(std::size_t k=0;k<targets;++k){
           // Provisional node indexing uses preorder, as in hatch tracks.
           // The four anti_grav coil panels are nodes 1, 2, 4 and 5.
           const auto node=integer(4);if(node>=nodes.size())throw std::invalid_argument("gravity UV node outside placed tree");
           for(std::size_t m=0;m<nodes[node]->meshes.size();++m){const auto& mesh=nodes[node]->meshes[m];if(mesh.materialSlot!=material)continue;
             for(std::size_t r=0;r<mesh.records.size();++r)for(std::size_t c=0;c<mesh.records[r].corners.size();++c){const auto& uv=mesh.records[r].corners[c];track.corners.push_back({std::size_t(node),m,r,c,uv.u,uv.v});}
           }
         }
         if(integer(4)!=0)throw std::invalid_argument("unsupported gravity sprite UV targets");
         if(track.corners.empty())throw std::invalid_argument("gravity UV track has no mesh targets");
         binding.tracks.push_back(std::move(track));
       }
       if(integer(4)!=0)throw std::invalid_argument("unsupported gravity event tracks");
       result.bindings.push_back(std::move(binding));
     }
   }
 }catch(const std::exception& e){return std::unexpected(std::string("gravity presentation: ")+e.what());}
 return result;
}
void BackdropScene::animateGravity(const GravityPresentation& presentation){
 for(const auto& binding:presentation.bindings){
   std::vector<SceneNode*> nodes;
   collectSceneNodes(roots_.at(binding.root).node, nodes);
   // Provisional clone UV playback. Loop from a shared zero phase on the scenery
   // clock, which pauses with play.
   const double time=std::fmod(sceneryClock_,binding.duration);
   for(const auto& track:binding.tracks){
     const auto [u, v] = sampleUvOffset(track.keys, time);
     for(const auto& c:track.corners){auto& uv=nodes[c.node]->meshes[c.mesh].records[c.record].corners[c.corner];uv.u=c.u+u;uv.v=c.v+v;}
   }
 }
}
std::expected<GadgetPresentation,std::string> BackdropScene::gadgetPresentation(std::span<const std::byte> bytes) const {
 GadgetPresentation result;
 if(std::none_of(roots_.begin(),roots_.end(),[](const auto& r){return r.source.owner.starts_with("teleport-")||r.source.owner=="tagged-speed-pad";}))return result;
 try {
   const auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
   auto links=readTeleportLinks(bytes);
   if(!links)throw std::invalid_argument(links.error());
   std::string diagnostic;
   const auto partners=resolveTeleportPartners(*links,diagnostic);
   std::vector<std::size_t> teleIndices(partners.size(),std::numeric_limits<std::size_t>::max());
   std::size_t next=0;
   for(std::size_t i=0;i<partners.size();++i)if(partners[i]>=0)teleIndices[i]=next++;
   for(std::size_t i=0;i<roots_.size();++i){
     const auto& owned=roots_[i];
     const bool teleport=owned.source.owner.starts_with("teleport-");
     if(!teleport&&owned.source.owner!="tagged-speed-pad")continue;
     GadgetPresentation::Binding b{};
     b.root=i;b.teleport=teleport;b.original=owned.node;
     const lemmings::LevelModel* model=nullptr;
     if(teleport){
       const auto& e=level.gadgets.entities_4347ec.at(owned.source.entityIndex);
       b.simIndex=teleIndices.at(owned.source.entityIndex);
       b.samples={e.names[2],e.names[3]};
       if(owned.source.owner=="teleport-base"){
         model=&*e.model;
         result.teleportHover.push_back({i,partners.at(owned.source.entityIndex)});
       }else {
         b.fxSide=owned.source.owner=="teleport-fx-0"?0:1;
         model=&e.models_1c8_1cc[b.fxSide];
         // 43540C/435467 move FX to the owning entity and set heading
         // (+30 + .5 - side*.5). PROVISIONAL matrix adapter: reuse the
         // placed base's scale/tilt; side zero reverses its tangent heading.
         for(const auto& base:roots_)if(base.source.owner=="teleport-base"&&base.source.entityIndex==owned.source.entityIndex){
           b.original.matrix=base.node.matrix;
           if(b.fxSide==0)for(int row:{0,2})for(int j=0;j<3;++j)b.original.matrix[3*row+j]*=-1;
           break;
         }
       }
     }else{
       const auto& e=level.gadgets.tagged_entities.at(owned.source.entityIndex);
       model=&*e.model;b.simIndex=0;
       for(std::size_t j=0;j<owned.source.entityIndex;++j)
         if(level.gadgets.tagged_entities[j].type==4)++b.simIndex;
     }
     const std::string name=teleport?(b.fxSide<0?"teleport":"teleport_fx"):"speedup_two_way";
     const auto& clips=model->object->auxiliary_records;
     const auto clip=std::find_if(clips.begin(),clips.end(),[&](const auto& c){return c.name==name;});
     if(clip==clips.end())throw std::invalid_argument("missing gadget clip "+name);
     const auto& raw=clip->serialized_bytes;std::size_t at=0;
     const auto integer=[&](std::size_t n){
       if(at>raw.size()||n>raw.size()-at)throw std::invalid_argument("truncated gadget clip");
       std::uint64_t v=0;for(std::size_t k=0;k<n;++k)v|=std::uint64_t(raw[at++])<<(8*k);return v;
     };
     const auto number=[&](){double v=std::bit_cast<double>(integer(8));if(!std::isfinite(v))throw std::invalid_argument("nonfinite gadget key");return v;};
     number();const auto transforms=integer(4);
     if(transforms>raw.size()/8)throw std::invalid_argument("invalid gadget transforms");
     for(std::size_t t=0;t<transforms;++t){integer(4);const auto keys=integer(4);if(keys>(raw.size()-at)/89)throw std::invalid_argument("invalid gadget keys");at+=keys*89;}
     std::vector<std::uint8_t> transformBytes(raw.begin(),raw.begin()+at);transformBytes.insert(transformBytes.end(),8,0);
     auto animation=readHatchAnimation(transformBytes);
     if(!animation)throw std::invalid_argument(animation.error());
     b.clip.transforms=std::move(*animation);
     std::vector<const SceneNode*> nodes;
     const auto collect=[&](auto& self,const SceneNode& n)->void{nodes.push_back(&n);for(const auto& c:n.children)self(self,c);};collect(collect,b.original);
     for(const auto& t:b.clip.transforms.tracks)if(t.node>=nodes.size())throw std::invalid_argument("gadget transform outside tree");
     const auto tracks=integer(4);if(tracks>raw.size()/16)throw std::invalid_argument("invalid gadget UV tracks");
     for(std::size_t t=0;t<tracks;++t){
       if(integer(4)!=0)throw std::invalid_argument("unsupported gadget UV kind");
       PressurePadPresentation::UvTrack track;
       const auto keys=integer(4);if(!keys||keys>(raw.size()-at)/40)throw std::invalid_argument("invalid gadget UV keys");
       double previous=0;
       for(std::size_t k=0;k<keys;++k){PressurePadPresentation::UvKey key{number(),number(),number(),number(),number()};if(key.end<previous)throw std::invalid_argument("unordered gadget UV keys");previous=key.end;track.keys.push_back(key);}
       if(previous<b.clip.transforms.duration)throw std::invalid_argument("gadget UV duration");
       const auto material=integer(4),targets=integer(4);if(targets>(raw.size()-at)/4)throw std::invalid_argument("invalid gadget UV targets");
       for(std::size_t k=0;k<targets;++k){
         const auto node=integer(4);if(node>=nodes.size())throw std::invalid_argument("gadget UV outside tree");
         for(std::size_t m=0;m<nodes[node]->meshes.size();++m){const auto& mesh=nodes[node]->meshes[m];if(mesh.materialSlot!=material)continue;
           for(std::size_t r=0;r<mesh.records.size();++r)for(std::size_t c=0;c<mesh.records[r].corners.size();++c){const auto& uv=mesh.records[r].corners[c];track.corners.push_back({std::size_t(node),m,r,c,uv.u,uv.v});}
         }
       }
       if(integer(4)!=0||track.corners.empty())throw std::invalid_argument("unsupported gadget UV targets");
       b.clip.uv.push_back(std::move(track));
     }
     const auto spins=integer(4);if(spins>(raw.size()-at)/16)throw std::invalid_argument("invalid gadget spins");
     for(std::size_t k=0;k<spins;++k){
       const auto node=integer(4),axis=integer(4);const double speed=number();
       if(node>=nodes.size()||axis>2)throw std::invalid_argument("gadget spin outside tree");
       b.clip.spins.push_back({std::size_t(node),unsigned(axis),speed});
     }
     result.bindings.push_back(std::move(b));
   }
 }catch(const std::exception& e){return std::unexpected(std::string("gadget presentation: ")+e.what());}
 return result;
}
std::string BackdropScene::triggerGadget(GadgetPresentation& presentation,bool teleport,
    std::size_t index,bool arrival,int facing,double seconds){
 if(!std::isfinite(seconds))return {};
 std::string sample;
 for(auto& b:presentation.bindings){
   if(b.teleport!=teleport||b.simIndex!=index)continue;
   if(b.fxSide<0){
     b.triggeredAt=seconds;
     if(teleport)sample=b.samples[arrival?1:0];
   }else{
     // 4355xx uses the entry side; 4350xx uses the preserved arrival face.
     const int side=arrival?(facing==1?1:0):(facing==1?0:1);
     if(b.fxSide!=side)continue;
     if(seconds>=b.activeUntil)b.triggeredAt=seconds;
     // PROVISIONAL finite tail: one authored FX cycle after the last use,
     // instead of reproducing native per-side occupied-actor counters.
     b.activeUntil=seconds+b.clip.transforms.duration;
   }
 }
 return sample;
}
void BackdropScene::animateGadgets(const GadgetPresentation& presentation,double seconds){
 if(!std::isfinite(seconds))return;
 for(const auto& b:presentation.bindings){
   auto& owned=roots_.at(b.root);owned.node=b.original;
   owned.visible=b.fxSide<0||seconds<b.activeUntil;
   if(!owned.visible)continue;
   const double duration=b.clip.transforms.duration;
   // Base rings loop while idle (4347EC selects clip 0 with loop=1).
   // Speed pads freeze at clip zero until contact, then play one full cycle;
   // restart on the next use. This contact-feedback policy is provisional.
   const double elapsed=b.fxSide<0&&b.teleport?seconds:std::max(0.0,seconds-b.triggeredAt);
   const double time=!b.teleport?(!std::isfinite(b.triggeredAt)?0.0:std::min(duration,elapsed)):
       std::fmod(elapsed,duration);
   applyHatchAnimation(owned.node,b.clip.transforms,time/duration);
   std::vector<SceneNode*> nodes;
   const auto collect=[&](auto& self,SceneNode& n)->void{nodes.push_back(&n);for(auto& c:n.children)self(self,c);};collect(collect,owned.node);
   for(const auto& track:b.clip.uv){
     std::size_t k=0;while(k+1<track.keys.size()&&time>track.keys[k].end)++k;
     const auto& key=track.keys[k];const double start=k?track.keys[k-1].end:0;
     const double fraction=key.end>start?std::clamp((time-start)/(key.end-start),0.0,1.0):0;
     for(const auto& c:track.corners){auto& uv=nodes[c.node]->meshes[c.mesh].records[c.record].corners[c.corner];uv.u=c.u+key.u+fraction*key.du;uv.v=c.v+key.v+fraction*key.dv;}
   }
   for(const auto& spin:b.clip.spins)
     setSceneSpin(nodes[spin.node]->matrix, spin.axis, time * spin.turnsPerSecond);
 }
}
std::expected<CrabPresentation,std::string> BackdropScene::crabPresentation(std::span<const std::byte> bytes) const {
 CrabPresentation presentation;
 if(std::none_of(roots_.begin(),roots_.end(),[](const auto& r){return r.source.owner=="named-crab";}))return presentation;
 try {
   const auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
   for(std::size_t i=0;i<roots_.size();++i){
     const auto& root=roots_[i];if(root.source.owner!="named-crab")continue;
     const auto& entity=level.gadgets.named_entities.at(root.source.entityIndex);
     const auto& tracks=entity.model->object->auxiliary_records;
     const auto animation=[&](const std::string& name)->std::expected<HatchAnimation,std::string>{
       for(const auto& track:tracks)if(track.name==name)return readHatchAnimation(track.serialized_bytes);
       return std::unexpected("crab lacks transform clip '"+name+"'");
     };
     auto rest=animation(entity.names[0]),attack=animation(entity.names[1]);
     if(!rest)return std::unexpected(rest.error());
     if(!attack)return std::unexpected(attack.error());
     presentation.bindings.push_back({i,root.source.entityIndex,std::move(*rest),std::move(*attack)});
   }
 }catch(const std::invalid_argument& e){return std::unexpected(e.what());}
 return presentation;
}
void BackdropScene::setCrabStates(const CrabPresentation& presentation,std::span<const sim::Crab> states){
 for(const auto& binding:presentation.bindings){
   const auto state=std::find_if(states.begin(),states.end(),[&](const auto& c){return c.entityIndex==binding.entityIndex;});
   const bool attacking=state!=states.end()&&state->attacking;
   const auto& clip=attacking?binding.attack:binding.rest;
   const double time=state==states.end()?0.0:state->animationSeconds;
   applyHatchAnimation(roots_.at(binding.root).node,clip,time/clip.duration);
 }
}
std::expected<NamedHazardPresentation,std::string> BackdropScene::namedHazardPresentation(
    std::span<const std::byte> bytes) const {
 NamedHazardPresentation result;
 if(std::none_of(roots_.begin(),roots_.end(),[](const auto& r){return r.source.owner=="named-hazard";}))return result;
 try {
   const auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
   result.coordinateHeight=std::int32_t(level.world.leading_value);
   result.coordinateRows.resize(level.world.first_blocks.size());
   // Convert the file lattice to the runtime layout used by balloon departure.
   for(std::size_t row=0;row<result.coordinateRows.size();++row)
     for(std::size_t x=0;x<32;++x)for(std::size_t layer=0;layer<2;++layer)
       result.coordinateRows[row][layer*32+x]=level.world.first_blocks[row][x*2+layer];
   for(std::size_t i=0;i<roots_.size();++i){
     const auto& root=roots_[i];if(root.source.owner!="named-hazard")continue;
     const auto& entity=level.gadgets.named_entities.at(root.source.entityIndex);
     NamedHazardPresentation::Binding binding{i,root.source.entityIndex,{},{},{}};
     const auto collect=[&](auto& self,const SceneNode& node)->void{
       binding.originalMatrices.push_back(node.matrix);for(const auto& c:node.children)self(self,c);
     };collect(collect,root.node);
     const auto animation=[&](const std::string& name)->NamedHazardPresentation::Clip {
       NamedHazardPresentation::Clip clip;
       if(name.empty())return clip;
       for(const auto& a:entity.model->object->auxiliary_records)if(a.name==name){
         if(a.third_count==0){
           auto transforms=readHatchAnimation(a.serialized_bytes);
           if(!transforms)throw std::invalid_argument(name+": "+transforms.error());
           clip.transforms=std::move(*transforms);
         }else{
           // 0x476277/0x47CDE7 read node, axis and turns/sec in the third array.
           // 0x476EE0 uses absolute clip time for rotation.
           if(a.first_count||a.second_count)throw std::invalid_argument("mixed hazard spin/transform tracks");
           const auto& raw=a.serialized_bytes;std::size_t at=20;
           const auto integer=[&](std::size_t n){
             if(at>raw.size()||n>raw.size()-at)throw std::invalid_argument("truncated hazard spin");
             std::uint64_t v=0;for(std::size_t k=0;k<n;++k)v|=std::uint64_t(raw[at++])<<(8*k);return v;
           };
           clip.transforms.duration=std::bit_cast<double>(a.leading_double_bits);
           for(unsigned j=0;j<a.third_count;++j){
             const auto node=integer(4),axis=integer(4);const double speed=std::bit_cast<double>(integer(8));
             if(node>=binding.originalMatrices.size()||axis>2||!std::isfinite(speed))
               throw std::invalid_argument("invalid hazard spin target");
             clip.spins.push_back({std::size_t(node),unsigned(axis),speed});
           }
         }
         return clip;
       }
       throw std::invalid_argument("missing hazard clip '"+name+"'");
     };
     binding.rest=animation(entity.names[0]);binding.attack=animation(entity.names[1]);
     result.bindings.push_back(std::move(binding));
   }
 }catch(const std::exception& e){return std::unexpected(std::string("named hazard presentation: ")+e.what());}
 return result;
}
void BackdropScene::setNamedHazardStates(const NamedHazardPresentation& presentation,
    std::span<const sim::NamedHazard> states){
 for(const auto& binding:presentation.bindings){
   auto& root=roots_.at(binding.root).node;
   std::vector<SceneNode*> nodes;
   const auto collect=[&](auto& self,SceneNode& node)->void{
     nodes.push_back(&node);for(auto& c:node.children)self(self,c);
   };collect(collect,root);
   for(std::size_t i=0;i<nodes.size();++i)nodes[i]->matrix=binding.originalMatrices[i];
   const auto state=std::find_if(states.begin(),states.end(),[&](const auto& h){return h.entityIndex==binding.entityIndex;});
   const auto& clip=state!=states.end()&&state->attacking?binding.attack:binding.rest;
   const double time=state==states.end()?0:state->animationSeconds;
   if(clip.transforms.duration>0)applyHatchAnimation(root,clip.transforms,time/clip.transforms.duration);
   for(const auto& spin:clip.spins){
     const double angle=time*spin.turnsPerSecond*2*3.1415927410125732; // 0x4B0F88
     const double c=std::cos(angle),s=std::sin(angle);
     auto& matrix=nodes[spin.node]->matrix;
     // 0x481D10/1E26/1F3C overwrite the rotation and retain translation.
     std::array<double,9> rotation{1,0,0,0,1,0,0,0,1};
     const unsigned a=(spin.axis+1)%3,b=(spin.axis+2)%3;
     rotation[3*a+a]=rotation[3*b+b]=c;
     rotation[3*a+b]=-s;rotation[3*b+a]=s;
     std::copy(rotation.begin(),rotation.end(),matrix.begin());
   }
   if(state==states.end()||!state->moved)continue;
   const lemmings::WorldCoordinateMesh mesh{presentation.coordinateHeight,presentation.coordinateRows};
   const auto start=lemmings::map_world_coordinates(mesh,state->initialPosition);
   const auto moved=lemmings::map_world_coordinates(mesh,state->position);
   for(unsigned j=0;j<3;++j)root.matrix[9+j]+=moved[j]-start[j];
   // Provisional orientation follows radial heading while preserving authored tilt and scale.
   const double angle=std::atan2(moved[2],moved[0])-std::atan2(start[2],start[0]);
   const double c=std::cos(angle),s=std::sin(angle);
   for(unsigned row=0;row<3;++row){
     const double x=root.matrix[3*row],z=root.matrix[3*row+2];
     root.matrix[3*row]=c*x-s*z;root.matrix[3*row+2]=s*x+c*z;
   }
 }
}
std::expected<PressurePadPresentation,std::string> BackdropScene::pressurePadPresentation(std::span<const std::byte> bytes) const {
 PressurePadPresentation presentation;
 if(std::none_of(roots_.begin(),roots_.end(),[](const auto& r){return r.source.owner=="tagged-pressure-pad";}))return presentation;
 try {
   const auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
   for(std::size_t i=0;i<roots_.size();++i){
     const auto& root=roots_[i];if(root.source.owner!="tagged-pressure-pad")continue;
     const auto& pad=level.gadgets.tagged_entities.at(root.source.entityIndex);
     PressurePadPresentation::Binding binding{};
     binding.root=i;binding.id=pad.id;binding.initialState=static_cast<std::int32_t>(pad.values[0]);binding.original=root.node;
     binding.displayedState=binding.initialState;
     const auto& clips=pad.model->object->auxiliary_records;
     if(clips.empty())return std::unexpected("pressure pad has no animation");
     // The pad selects clip index 0 (43191F/431950). Its transform and
     // material tracks share the Object 09 clock and preorder node indices.
     const auto& raw=clips.front().serialized_bytes;
     std::size_t at=0;
     const auto integer=[&](std::size_t size){
       if(at>raw.size()||size>raw.size()-at)throw std::invalid_argument("truncated pad animation");
       std::uint64_t value=0;for(std::size_t k=0;k<size;++k)value|=std::uint64_t(raw[at++])<<(8*k);return value;
     };
     const auto number=[&](){const double value=std::bit_cast<double>(integer(8));if(!std::isfinite(value))throw std::invalid_argument("nonfinite pad animation");return value;};
     number();const auto transforms=integer(4);
     if(transforms>raw.size()/8)throw std::invalid_argument("invalid pad transform count");
     for(std::size_t t=0;t<transforms;++t){integer(4);const auto keys=integer(4);if(keys>(raw.size()-at)/89)throw std::invalid_argument("invalid pad key count");at+=keys*89;}
     // Sample the transform keys before reading material and event tracks.
     std::vector<std::uint8_t> transformBytes(raw.begin(),raw.begin()+at);
     transformBytes.insert(transformBytes.end(),8,0);
     auto animation=readHatchAnimation(transformBytes);
     if(!animation)return std::unexpected("pressure pad: "+animation.error());
     binding.animation=std::move(*animation);
     std::vector<const SceneNode*> nodes;
     const auto collect=[&](auto& self,const SceneNode& n)->void{nodes.push_back(&n);for(const auto& c:n.children)self(self,c);};collect(collect,root.node);
     for(const auto& track:binding.animation.tracks)if(track.node>=nodes.size())throw std::invalid_argument("pad transform node outside tree");
     const auto tracks=integer(4);if(tracks>raw.size()/16)throw std::invalid_argument("invalid pad UV track count");
     for(std::size_t t=0;t<tracks;++t){
       if(integer(4)!=0)throw std::invalid_argument("unsupported pad material track");
       PressurePadPresentation::UvTrack track;
       const auto keys=integer(4);if(!keys||keys>(raw.size()-at)/40)throw std::invalid_argument("invalid pad UV keys");
       double previous=0;
       for(std::size_t k=0;k<keys;++k){PressurePadPresentation::UvKey key{number(),number(),number(),number(),number()};if(key.end<previous)throw std::invalid_argument("unordered pad UV keys");previous=key.end;track.keys.push_back(key);}
       if(previous<binding.animation.duration)throw std::invalid_argument("pad UV keys do not cover duration");
       const auto material=integer(4),targets=integer(4);if(targets>(raw.size()-at)/4)throw std::invalid_argument("invalid pad UV targets");
       for(std::size_t k=0;k<targets;++k){
         const auto node=integer(4);if(node>=nodes.size())throw std::invalid_argument("pad UV node outside tree");
         for(std::size_t m=0;m<nodes[node]->meshes.size();++m){const auto& mesh=nodes[node]->meshes[m];if(mesh.materialSlot!=material)continue;
           for(std::size_t r=0;r<mesh.records.size();++r)for(std::size_t c=0;c<mesh.records[r].corners.size();++c){const auto& uv=mesh.records[r].corners[c];track.corners.push_back({std::size_t(node),m,r,c,uv.u,uv.v});}
         }
       }
       if(integer(4)!=0)throw std::invalid_argument("unsupported pad sprite UV targets");
       if(track.corners.empty())throw std::invalid_argument("pad UV track has no mesh targets");
       binding.uvTracks.push_back(std::move(track));
     }
     if(integer(4)!=0)throw std::invalid_argument("unsupported pad event tracks");
     presentation.bindings.push_back(std::move(binding));
   }
 } catch(const std::invalid_argument& e){return std::unexpected(e.what());}
 return presentation;
}
void BackdropScene::setPressurePadStates(PressurePadPresentation& presentation,std::span<const std::pair<int,int>> idStates){
 for(auto& binding:presentation.bindings){
   int state=binding.initialState;
   for(const auto& [id,current]:idStates)if(id==binding.id){state=current;break;}
   if(state!=binding.displayedState){binding.triggeredAt=sceneryClock_;binding.displayedState=state;}
   auto& root=roots_.at(binding.root).node;
   root=binding.original;
   const double duration=binding.animation.duration;
   const double time=state==1?std::min(duration,sceneryClock_-binding.triggeredAt):std::fmod(sceneryClock_,duration);
   applyHatchAnimation(root,binding.animation,time/duration);
   std::vector<SceneNode*> nodes;
   const auto collect=[&](auto& self,SceneNode& n)->void{nodes.push_back(&n);for(auto& c:n.children)self(self,c);};collect(collect,root);
   for(const auto& track:binding.uvTracks){
     std::size_t k=0;while(k+1<track.keys.size()&&time>track.keys[k].end)++k;
     const auto& key=track.keys[k];const double start=k?track.keys[k-1].end:0.0;
     const double t=key.end>start?std::clamp((time-start)/(key.end-start),0.0,1.0):0.0;
     // 476FB0/476A80: base UV plus the key's offset + fraction*delta.
     for(const auto& c:track.corners){auto& uv=nodes[c.node]->meshes[c.mesh].records[c.record].corners[c.corner];uv.u=c.u+key.u+t*key.du;uv.v=c.v+key.v+t*key.dv;}
   }
   if(state==1&&time>=duration){
     // 0x431B93..0x431BF9 finds "cut" nodes. 0x4319E5 hides them on first activation.
     // Provisional clone feedback plays the clip before pruning each subtree,
     // delaying the immediate hide at native 4319E5. Simulation latches immediately.
     const auto hide=[&](auto& self,SceneNode& node)->void{
       if(node.name.find("cut")!=std::string::npos){node.meshes.clear();node.children.clear();return;}
       for(auto& child:node.children)self(self,child);
     };hide(hide,root);
   }
   binding.displayedState=state;
 }
}
std::expected<LeverPresentation,std::string> BackdropScene::leverPresentation(std::span<const std::byte> bytes) const {
 LeverPresentation presentation;
 // Avoid parsing the full level twice for levels without levers.
 if(std::none_of(roots_.begin(),roots_.end(),[](const auto& r){return r.source.owner=="tagged-lever";}))return presentation;
 lemmings::CompiledLevel level;
 try{level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});}
 catch(const std::invalid_argument& e){return std::unexpected(e.what());}
 for(std::size_t i=0;i<roots_.size();++i){
   const auto& root=roots_[i];if(root.source.owner!="tagged-lever")continue;
   const auto& lever=level.gadgets.tagged_entities.at(root.source.entityIndex);
   const auto& tracks=lever.model->object->auxiliary_records;
   const auto track=std::find_if(tracks.begin(),tracks.end(),[](const auto& a){return a.name=="lever";});
   if(track==tracks.end())return std::unexpected("lever model lacks its lever transform track");
   auto animation=readHatchAnimation(track->serialized_bytes);
   if(!animation)return std::unexpected("lever transform: "+animation.error());
   presentation.bindings.push_back({i,lever.id,static_cast<std::int32_t>(lever.values[0]),lever.flag_a0,std::move(*animation)});
 }
 return presentation;
}
void BackdropScene::setLeverFaces(const LeverPresentation& presentation,std::span<const std::pair<int,int>> idFaces){
 for(const auto& binding:presentation.bindings){
   int face=binding.initialFace;
   for(const auto& [id,current]:idFaces)if(id==binding.id){face=current;break;}
   if(face!=1&&face!=-1)continue; // 0x436E6A ignores unsupported faces.
   // 0x436E6A selects "lever" with direction (face == -1) XOR [+0xA0].
   // At load, 0x43703F calls model vt+0x24 (0x45130A), sampling duration
   // forward and zero in reverse. Provisional clone presentation jumps to the endpoint.
   const bool reverse=(face==-1)!=binding.reversed;
   applyHatchAnimation(roots_.at(binding.root).node,binding.animation,reverse?0.0:1.0);
 }
}
std::expected<EntranceExitPresentation,std::string> BackdropScene::entranceExitPresentation(std::span<const std::byte> bytes) const {
 EntranceExitPresentation presentation;
 lemmings::CompiledLevel level;
 try {
   level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
 } catch(const std::invalid_argument& e) { return std::unexpected(e.what()); }
 presentation.exitCoordinateHeight=static_cast<std::int32_t>(level.world.leading_value);
 presentation.exitCoordinateRows.resize(level.world.first_blocks.size());
 // 0x43D09A separates file inner/outer pairs into the runtime layers for 0x43D56C.
 for(std::size_t row=0;row<level.world.first_blocks.size();++row)
   for(std::size_t x=0;x<32;++x)for(std::size_t layer=0;layer<2;++layer)
     presentation.exitCoordinateRows[row][layer*32+x]=level.world.first_blocks[row][x*2+layer];
 for(std::size_t i=0;i<roots_.size();++i) {
   const auto& owned=roots_[i];
   if(owned.source.owner=="entities_425e50") {
     const auto& model=level.gadgets.entities_425e50.at(owned.source.entityIndex).model;
     if(!model||!model->object)continue;
     // Water hatches use "entrance_acid" for the same two-door track layout.
     for(const auto& aux:model->object->auxiliary_records)if(aux.name=="entrance"||aux.name=="entrance_acid") {
       auto animation=readHatchAnimation(aux.serialized_bytes);
       if(!animation)return std::unexpected(animation.error());
       presentation.entrances.push_back({i,std::move(*animation)});
       break;
     }
   } else if(owned.source.owner=="entities_426c6b") {
     const auto& entity=level.gadgets.entities_426c6b.at(owned.source.entityIndex);
     std::array<double,3> position{};
     for(std::size_t j=0;j<3;++j)position[j]=std::bit_cast<double>(entity.double_bits[j]);
     // Validate the view once at binding, before the frame loop uses it.
     try {
       (void)lemmings::map_world_coordinates(
           {presentation.exitCoordinateHeight,presentation.exitCoordinateRows},position);
     }catch(const std::exception& e){return std::unexpected(std::string("exit coordinates: ")+e.what());}
     presentation.exits.push_back({i,owned.node.matrix,position});
   }
 }
 return presentation;
}
void BackdropScene::setEntranceExitPresentation(const EntranceExitPresentation& presentation,
    double openingProgress, double departureSeconds, double levelSeconds, double swayScale) {
 if(!std::isfinite(openingProgress)||!std::isfinite(departureSeconds)||!std::isfinite(levelSeconds)||
    !std::isfinite(swayScale))return;
 const double time = std::max(0.0, departureSeconds);
 const double bob = exitBalloonBob(levelSeconds);
 // Provisional departure starts on final rescue. The caller damps sway during landings.
 const double departureRamp = (time - kExitJumpSeconds) / kExitBalloonSwayBlendSeconds;
 const double swayWeight = std::clamp(std::min(departureRamp, swayScale), 0.0, 1.0);
 for(const auto& entrance:presentation.entrances)
   applyHatchAnimation(roots_[entrance.root].node,entrance.animation,openingProgress);
 for(const auto& exit:presentation.exits) {
   auto& matrix=roots_[exit.root].node.matrix;
   matrix=exit.matrix;
   if(time==0.0&&bob==0.0)continue;
   // READ 0x426A0C..0x426A95: logical X += dt*0.5, Y += dt*5,
   // radial blend += dt*0.5; X wraps at 256 (0x4231DE). The installed
   // lattice converts the blend into the root's outward motion.
   // Model slot+8 (0x451668 -> 0x4658D6 -> 0x43D56C) maps this triple.
   auto position=exit.cylinderPosition;
   position[0]=std::fmod(position[0]+0.5*time,256.0);
   position[1]+=5.0*time+bob;
   position[2]+=0.5*time;
   const lemmings::WorldCoordinateMesh mesh{presentation.exitCoordinateHeight,presentation.exitCoordinateRows};
   const auto start=lemmings::map_world_coordinates(mesh,exit.cylinderPosition);
   const auto moved=lemmings::map_world_coordinates(mesh,position);
   for(std::size_t j=0;j<3;++j)matrix[9+j]+=moved[j]-start[j];
   // Provisional departure follows radial heading, preserves scale and tilt,
   // and continues the idle bob through Pending.
   const double angle=std::atan2(moved[2],moved[0])-std::atan2(start[2],start[0]);
   const double c=std::cos(angle),s=std::sin(angle);
   for(std::size_t row=0;row<3;++row){
     const double x=exit.matrix[3*row],z=exit.matrix[3*row+2];
     matrix[3*row]=c*x-s*z;matrix[3*row+2]=s*x+c*z;
   }

   if (swayWeight == 0.0) continue;

   // READ 0x426993..0x426A03 and 0x451668. The whole model tilts by
   // 0.02*cos(phase) turns about X, then 0.02*sin(phase) turns about Z,
   // before applying its heading. Translation is independent of the tilt.
   const double phase = exitBalloonPhaseRadians(levelSeconds);
   const double amplitude = (0.02 * 2.0 * kExitBalloonPi) * swayWeight;
   const double xTilt = amplitude * std::cos(phase);
   const double zTilt = amplitude * std::sin(phase);
   const double cx = std::cos(xTilt), sx = std::sin(xTilt);
   const double cz = std::cos(zTilt), sz = std::sin(zTilt);
   const Matrix rotateX{1, 0, 0, 0, cx, -sx, 0, sx, cx, 0, 0, 0};
   const Matrix rotateZ{cz, -sz, 0, sz, cz, 0, 0, 0, 1, 0, 0, 0};
   matrix = compose(compose(rotateX, rotateZ), matrix);
 }
}
namespace {
template<class Consumer>
void projectMeshForBounds(const SceneNode& node, const CameraBasis& camera,
                          const Matrix& context, Consumer consume) {
  auto records = buildSceneMeshSource(node, identity(), camera.worldToView);
  if (!records) return;
  std::map<std::size_t, std::array<double, 3>> eyes;
  collectEyes(node, identity(), context, eyes);
  for (const auto& record : *records) {
    if (!passesFacingPredicate(record, eyes)) continue;
    for (std::size_t k = 1; k + 1 < record.corners.size(); ++k) {
      const std::size_t ids[]{0, k, k + 1};
      ProjectedMeshTriangle t;
      bool valid = true;
      for (int j = 0; j < 3; ++j) {
        const auto& p = record.corners[ids[j]].viewPosition;
        if (!(p[2] > 0)) { valid = false; break; }
        const double x = camera.cx + camera.focalX * p[0] / p[2];
        const double y = camera.cy - camera.focalY * p[1] / p[2];
        if (!std::isfinite(x) || !std::isfinite(y)) { valid = false; break; }
        t[j] = {x, y, p[2]};
      }
      if (valid) consume(t);
    }
  }
}

// Provisional clone bounds follow visible depth pixels, including the current
// hatch pose. The original uses a box of +/-20 around the model origin.
void accumulateVisibleBounds(PickBox& h, const ProjectedMeshTriangle& t,
                             const CameraBasis& camera, const std::vector<double>& depth) {
  const auto& [ax, ay, az] = t[0];
  const auto& [bx, by, bz] = t[1];
  const auto& [cx, cy, cz] = t[2];
  const double area = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
  if (std::abs(area) < 1e-9) return;
  const int x0 = int(std::clamp(std::floor(std::min({ax, bx, cx})), 0.0, double(camera.width)));
  const int x1 = int(std::clamp(std::ceil(std::max({ax, bx, cx})), 0.0, double(camera.width)));
  const int y0 = int(std::clamp(std::floor(std::min({ay, by, cy})), 0.0, double(camera.height)));
  const int y1 = int(std::clamp(std::ceil(std::max({ay, by, cy})), 0.0, double(camera.height)));
  for (int y = y0; y < y1; ++y) for (int x = x0; x < x1; ++x) {
    const double px = x + 0.5, py = y + 0.5;
    const double b = ((px - ax) * (cy - ay) - (py - ay) * (cx - ax)) / area;
    const double c = ((bx - ax) * (py - ay) - (by - ay) * (px - ax)) / area;
    const double a = 1 - b - c;
    if (a < 0 || b < 0 || c < 0) continue;
    const double plane = a * az + b * bz + c * cz;
    if (std::abs(plane - depth[std::size_t(y) * camera.width + x]) > 0.5) continue;
    h.left = std::min(h.left, x);
    h.top = std::min(h.top, y);
    h.right = std::max(h.right, x + 1);
    h.bottom = std::max(h.bottom, y + 1);
  }
}

std::optional<PickBox> visibleMeshBounds(const SceneNode& node, const CameraBasis& camera,
                                        const std::vector<double>& depth, const Matrix& context) {
  PickBox bounds{camera.width, camera.height, 0, 0};
  projectMeshForBounds(node, camera, context, [&](const auto& t) {
    accumulateVisibleBounds(bounds, t, camera, depth);
  });
  if (bounds.left < bounds.right && bounds.top < bounds.bottom) return bounds;
  return std::nullopt;
}

}  // namespace

std::vector<PreparedHatchQuery> BackdropScene::prepareHatchQueries(const CameraBasis& camera) const {
  std::vector<PreparedHatchQuery> queries;
  const auto context = predicateContext(camera);
  for (const auto& owned : roots_) {
    if (owned.source.owner != "entities_425e50") continue;
    auto& query = queries.emplace_back(PreparedHatchQuery{owned.source.entityIndex, {}});
    projectMeshForBounds(owned.node, camera, context, [&](const auto& t) { query.triangles.push_back(t); });
  }
  return queries;
}

std::vector<HatchHoverTarget> BackdropScene::hatchHoverTargets(
    const CameraBasis& camera, const Raster& raster, std::span<const PreparedHatchQuery> queries) {
  std::vector<HatchHoverTarget> targets;
  if (raster.framebuffer().width != camera.width || raster.framebuffer().height != camera.height) return targets;
  for (const auto& query : queries) {
    PickBox bounds{camera.width, camera.height, 0, 0};
    for (const auto& t : query.triangles) accumulateVisibleBounds(bounds, t, camera, raster.depthBuffer());
    if (bounds.left < bounds.right && bounds.top < bounds.bottom)
      targets.push_back({query.index, bounds.left, bounds.top, bounds.right, bounds.bottom});
  }
  return targets;
}

std::vector<HatchHoverTarget> BackdropScene::hatchHoverTargets(
    const CameraBasis& camera, const Raster& raster) const {
  std::vector<HatchHoverTarget> targets;
  if (raster.framebuffer().width != camera.width || raster.framebuffer().height != camera.height) return targets;
  const auto context = predicateContext(camera);
  for (const auto& owned : roots_) {
    if (owned.source.owner != "entities_425e50") continue;
    if (auto bounds = visibleMeshBounds(owned.node, camera, raster.depthBuffer(), context))
      targets.push_back({owned.source.entityIndex, bounds->left, bounds->top, bounds->right, bounds->bottom});
  }
  return targets;
}

std::optional<PickBox> BackdropScene::exitHoverTarget(
    const CameraBasis& camera, const Raster& raster, int x, int y) const {
  if (raster.framebuffer().width != camera.width || raster.framebuffer().height != camera.height ||
      !pointerOverSceneBounds(x, y, 0, 0, camera.width, camera.height)) return std::nullopt;
  const auto context = predicateContext(camera);
  for (const auto& owned : roots_) {
    if (owned.source.owner != "entities_426c6b" || !owned.visible) continue;
    // Use the current root pose for the envelope, ropes and basket together.
    const auto box = visibleMeshBounds(owned.node, camera, raster.depthBuffer(), context);
    if (box && pointerOverSceneBounds(x, y, box->left, box->top, box->right, box->bottom))
      return box;
  }
  return std::nullopt;
}

std::optional<PreparedHatchQuery> BackdropScene::prepareHoverQuery(
    const CameraBasis& camera, std::size_t root, int x, int y) const {
  if (!pointerOverSceneBounds(x, y, 0, 0, camera.width, camera.height)) return std::nullopt;
  PreparedHatchQuery query{root, {}};
  const auto context = predicateContext(camera);
  double left = camera.width, top = camera.height, right = 0, bottom = 0;
  projectMeshForBounds(roots_[root].node, camera, context, [&](const ProjectedMeshTriangle& t) {
    query.triangles.push_back(t);
    for (const auto& p : t) {
      left = std::min(left, p[0]); top = std::min(top, p[1]);
      right = std::max(right, p[0]); bottom = std::max(bottom, p[1]);
    }
  });
  if (x < std::floor(left) || x >= std::ceil(right) || y < std::floor(top) || y >= std::ceil(bottom))
    return std::nullopt;
  return query;
}

std::vector<PreparedHatchQuery> BackdropScene::prepareExitQueries(const CameraBasis& camera, int x, int y) const {
  std::vector<PreparedHatchQuery> queries;
  for (std::size_t root = 0; root < roots_.size(); ++root) {
    const auto& owned = roots_[root];
    if (owned.source.owner == "entities_426c6b" && owned.visible)
      if (auto query = prepareHoverQuery(camera, root, x, y)) queries.push_back(std::move(*query));
  }
  return queries;
}

bool BackdropScene::prepareSwitchHover(std::vector<HoverFragment>& fragments,
    const CameraBasis& camera, const Raster& raster,
    const sim::LevelSetup& level, const LeverPresentation* levers,
    const PressurePadPresentation* pads,
    std::span<const GadgetPresentation::TeleportHoverBinding> teleports, int pointerX, int pointerY,
    const std::function<void(const sim::OccupancySensor&)>& appendSensor) const {
  const auto pixels = raster.highlightPixels();
  if (pixels.empty() || pixels.size() != std::size_t(camera.width) * camera.height) return false;
  if ((!levers || levers->bindings.empty()) && (!pads || pads->bindings.empty()) &&
      teleports.empty()) return false;
  if (!pointerOverSceneBounds(pointerX, pointerY, 0, 0, camera.width, camera.height))
    return false;
  const auto context = predicateContext(camera);
  const auto pick = [&](std::size_t root) {
    const auto bounds = visibleMeshBounds(roots_[root].node, camera, raster.depthBuffer(), context);
    return bounds && pointerOverSceneBounds(pointerX, pointerY, bounds->left, bounds->top,
                                            bounds->right, bounds->bottom);
  };

  const auto selected = selectSwitchHover(level, levers, pads, teleports,
      pixels[std::size_t(pointerY) * camera.width + pointerX].owner, pick);
  if (!selected) return false;
  for (std::size_t p = 0; p < pixels.size(); ++p) {
    const auto& pixel = pixels[p];
    if (pixel.owner.kind != SceneHighlightKind::None &&
        std::ranges::find(selected->targets, pixel.owner) != selected->targets.end())
      fragments.push_back({p, pixel.rgb});
  }
  for (const auto index : selected->sensors) appendSensor(level.occupancySensors[index]);
  return true;
}

std::optional<BackdropScene::SwitchHover> BackdropScene::selectSwitchHover(
    const sim::LevelSetup& level, const LeverPresentation* levers, const PressurePadPresentation* pads,
    std::span<const GadgetPresentation::TeleportHoverBinding> teleports, SceneHighlightId pointerOwner,
    const std::function<bool(std::size_t)>& pick) const {
  std::optional<std::size_t> hovered;
  auto portal = teleports.end();
  if (!teleports.empty()) {
    // A visible portal fragment takes priority over switch bounds, including
    // idle ring fragments that do not write scene depth.
    if (pointerOwner.kind == SceneHighlightKind::BackdropRoot)
      portal = std::ranges::find(teleports, pointerOwner.index, &GadgetPresentation::TeleportHoverBinding::root);
    if (portal != teleports.end()) hovered = portal->root;
  }

  // Clone addition. Read both lever lists so the control can be identified
  // in either position. A laser switch shares the one-shot pad binding.
  std::span<const sim::LeverMessage> first, second;
  if (!hovered && levers) for (const auto& binding : levers->bindings) {
    if (!pick(binding.root)) continue;
    hovered = binding.root;
    const auto lever = std::ranges::find_if(level.levers, [&](const auto& entry) {
      return entry.linked && entry.serializedId == binding.id;
    });
    if (lever != level.levers.end()) {
      first = lever->faceMinus;
      second = lever->facePlus;
    }
    break;
  }
  if (!hovered && pads) for (const auto& binding : pads->bindings) {
    if (!pick(binding.root)) continue;
    hovered = binding.root;
    const auto pad = std::ranges::find(level.pressurePads, binding.id, &sim::PressurePad::id);
    if (pad != level.pressurePads.end()) first = pad->messages;
    break;
  }
  if (!hovered) {
    portal = std::ranges::find_if(teleports, [&](const auto& binding) {
      return roots_[binding.root].visible && pick(binding.root);
    });
    if (portal != teleports.end()) hovered = portal->root;
  }
  if (!hovered) return std::nullopt;

  // Clone addition. Like drawHoveredWalker, add the source RGB once on the
  // completed frame, using ownership from the original geometry submission.
  std::vector<SceneHighlightId> targets{{SceneHighlightKind::BackdropRoot,
                                         static_cast<std::uint32_t>(*hovered)}};
  const auto model = [&](std::string_view collection, std::size_t index) {
    for (std::uint32_t i = 0; i < roots_.size(); ++i) {
      const auto& root = roots_[i];
      if (!root.visible || root.source.entityIndex != index) continue;
      const std::string_view owner = root.source.owner;
      if (owner == collection ||
          (collection == "tagged_entities" && owner.starts_with("tagged-")) ||
          (collection == "named_entities" && owner.starts_with("named-")) ||
          (collection == "entities_4347ec" && owner == "teleport-base"))
        targets.push_back({SceneHighlightKind::BackdropRoot, i});
    }
  };
  // Follow one outgoing link. Departure and arrival shafts are transient
  // effects with their own roots; the hover identifies the portal bases.
  if (portal != teleports.end() && portal->partnerEntityIndex >= 0)
    model("entities_4347ec", portal->partnerEntityIndex);
  std::vector<std::uint32_t> sensors;
  const auto addReceiver = [&](const sim::ObjectRef& ref) {
    const auto index = static_cast<std::uint32_t>(ref.index);
    switch (ref.cls) {
      case sim::ObjectClass::Gate:
        if (index < level.gates.size()) targets.push_back({SceneHighlightKind::Fluid, index});
        break;
      case sim::ObjectClass::CounterGate:
        if (index < level.counterGates.size()) targets.push_back({SceneHighlightKind::CounterGate, index});
        break;
      case sim::ObjectClass::OccupancySensor:
        if (index < level.occupancySensors.size()) sensors.push_back(index);
        break;
      case sim::ObjectClass::NamedHazard:
        if (index < level.namedHazards.size()) model(ref.collection, level.namedHazards[index].entityIndex);
        break;
      case sim::ObjectClass::Other:
        model(ref.collection, index);
        break;
    }
  };

  // Use the World loader's signed base-id table directly.
  std::vector<int> resolved;
  for (const auto messages : {first, second}) for (const auto& message : messages) {
    if (message.targetId < 0 || std::ranges::find(resolved, message.targetId) != resolved.end()) continue;
    const auto receiver = level.objectIds.find(message.targetId);
    if (receiver == level.objectIds.end()) continue;
    resolved.push_back(message.targetId);
    addReceiver(receiver->second);
  }
  return SwitchHover{std::move(targets), std::move(sensors)};
}

bool BackdropScene::drawSwitchHover(Image& image, const CameraBasis& camera, const Raster& raster,
    const sim::LevelSetup& level, const LeverPresentation* levers, const PressurePadPresentation* pads,
    std::span<const GadgetPresentation::TeleportHoverBinding> teleports, int x, int y,
    const std::function<void(const sim::OccupancySensor&)>& drawSensor) const {
  if (image.width != camera.width || image.height != camera.height ||
      image.rgba.size() / 4 != image.pixelCount()) return false;
  std::vector<HoverFragment> fragments;
  const bool hovered = prepareSwitchHover(fragments, camera, raster, level, levers, pads, teleports, x, y,
      [&](const sim::OccupancySensor& sensor) {
        drawHoverFragments(image, fragments);
        fragments.clear();
        drawSensor(sensor);
      });
  drawHoverFragments(image, fragments);
  return hovered;
}

std::vector<std::size_t> BackdropScene::fluidEntityIndices() const {
 std::vector<std::size_t> out;
 for(const auto& r:fluidRoots_)out.push_back(r.record.entityIndex);
 return out;
}
void BackdropScene::render(Raster& target,const CameraBasis& camera,const AssetLibrary& lib,BackdropStats& stats) const {
  renderStage(target,camera,lib,stats,Stage::Primary);
  renderStage(target,camera,lib,stats,Stage::LegacyUnattributed);
  renderStage(target,camera,lib,stats,Stage::GadgetEffect);
  renderStage(target,camera,lib,stats,Stage::FluidPipe);
  renderStage(target,camera,lib,stats,Stage::FluidGate);
  renderStage(target,camera,lib,stats,Stage::Secondary);
  renderStage(target,camera,lib,stats,Stage::TeleportEffect);
}
// Update the parsed base model copy with 0x0042B24D's animation and
// 0x0042C5C9's normal sign. The 0x428848 table supplies constants and
// pi is the promoted f32 value 3.1415927410125732.
void BackdropScene::applyFluidState(SceneNode& g,const FluidRecord& rec,const FluidGateState& st){
 constexpr double kPi=3.1415927410125732;
 // 0x428848 writes rows at 0x4BF6E8 + kind*0x48. 0x42B24D reads
 // period +0, amplitude +8 and lane count +0x38.
 const double period=rec.kind==1?8.0:rec.kind==2?0.8:2.0;
 const double amplitude=rec.kind==1?1.0:rec.kind==2?0.25:0.5;
 const std::uint32_t lanes=rec.kind==1?16u:rec.kind==2?2u:4u;
 const double h=st.height;
 // 0x42C5C9: normals 0..2 are negated whenever the height is zero (the facing predicate is then inverted).
 if(h==0.0)for(std::size_t k=0;k<3;++k)for(double& x:g.normals[k])x=-x;
 if(!st.animated)return;  // Deserialized mesh until the first update.
 const double phase=st.wavePhase/period;
 const double x1=static_cast<double>(st.lane&(lanes-1u))/lanes+phase;
 const double x2=static_cast<double>((st.lane+1u)&(lanes-1u))/lanes+phase;
 const auto wave=[&](double x){double w=std::cos(2.0*kPi*x)*amplitude;if(h==0.0)w=0.0;if(w+h<0.0)w=-h;return w;};
 const double w1=wave(x1),w2=wave(x2);
 const auto place=[&](std::size_t slot,std::size_t k,double w){for(std::size_t j=0;j<3;++j)g.positions[slot][j]=rec.posOff[k][j]+(h+w)*rec.amp[k][j];};
 place(2,0,w1);place(4,3,w1);place(3,1,w2);place(5,2,w2);
 const auto colour=[&](double x){
   double col=0.0;
   if(h!=0.0){const double t=(std::cos(2.0*kPi*(x-0.25))+1.0)/2.0;col=t*t;}
   std::array<double,3> c{};for(std::size_t j=0;j<3;++j)c[j]=rec.baseColour[j]+(255.0-rec.baseColour[j])*col;
   return c;
 };
 const auto c1=colour(x1),c2=colour(x2);
 // Lane 1 colors vertex slots 2 and 4. Lane 2 colors slots 3 and 5.
 // Kind 0 retains authored corner alpha.
 for(auto& rec5:g.meshes[1].records)for(auto& c:rec5.corners){
   const auto& v=(c.vertexIndex==2||c.vertexIndex==4)?c1:c2;
   c.r=v[0];c.g=v[1];c.b=v[2];
 }
 // 0x42BF8D..0x42C1DC: only kind 2 has alphaPeriod != 0. Top corners
 // and both flat side triangles share max(0, 191 + 64*cos(2*pi*phase/4)).
 if(rec.kind==2){
   const double alpha=std::max(0.0,191.0+64.0*std::cos(2.0*kPi*st.alphaPhase/4.0));
   for(auto& block:g.meshes)for(auto& triangle:block.records)for(auto& c:triangle.corners)c.a=alpha;
 }
}
namespace {
const SceneRgbaImage whiteSceneTexture{1,1,{255,255,255,255}};
}

void BackdropScene::prepareFluid(PreparedStage& prepared, const CameraBasis& camera,
                                 bool observeCandidates, bool describeDraws) const {
 if(!hasFluidState_||fluidRoots_.empty())return;
 prepared.active = true;
 auto& stats = prepared.stats;
 const auto context=predicateContext(camera);
 const auto scale=buildBillboardCameraChain(camera).projectionTransform[10];
 // FluidDepthPolicy selects provisional depth behavior. The model+0xF8
 // projection bias is independent of device ZBIAS.
 const auto optionsFor=[&](bool blend){
   auto o=options(false,blend);o.shade=SceneShade::Flat;
   if(fluidPolicy_==FluidDepthPolicy::LessWriteOff)o.depthWrite=false;
   else if(fluidPolicy_==FluidDepthPolicy::DepthDisabled){o.depthFunction=SceneDepthFunction::Always;o.depthWrite=false;}
   return o;
 };
 for(const auto& root:fluidRoots_){  // source order of the receivers
   if(root.record.entityIndex>=fluidState_.size())continue;
   const auto& state=fluidState_[root.record.entityIndex];
   SceneNode node=root.node;
   const auto first=[&](auto& self,SceneNode& n)->SceneNode*{if(n.type==1)return &n;for(auto& c:n.children)if(auto* r=self(self,c))return r;return nullptr;};
   SceneNode* mutableGeometry=first(first,node);
   if(!mutableGeometry)continue;
   applyFluidState(*mutableGeometry,root.record,state);
   std::map<std::size_t,std::array<double,3>> eyes;collectEyes(node,identity(),context,eyes);
   auto records=buildSceneMeshSource(node,identity(),camera.worldToView);
   if(!records){const auto message=webo::app::context(levelId_,root.source,node.sourceOffset)+records.error();stats.diagnostics.push_back(message);std::fprintf(stderr,"backdrop: %s\n",message.c_str());continue;}
   BackdropStats::FluidSubmission sub;sub.entityIndex=root.record.entityIndex;sub.height=state.height;sub.raw=!state.animated;sub.records=records->size();
   for(const auto& rec:*records){
     if (observeCandidates) prepared.candidates.push_back({prepared.draws.size(),
         rec.recordSourceOffset, rec.nodeSourceOffset, rec.blockSourceOffset,
         rec.corners.size() > 2 ? rec.corners.size() - 2 : 0});
     if(!passesFacingPredicate(rec,eyes)){++stats.predicateRejected;++sub.predicateRejected;continue;}
     auto opts=optionsFor(rec.blendSelector==1);
     opts.highlight = {SceneHighlightKind::Fluid, static_cast<std::uint32_t>(root.record.entityIndex)};
     const auto first = prepared.draws.size();
     for(std::size_t k=1;k+1<rec.corners.size();++k){
       std::array<SceneRasterVertex,3> v{};bool valid=true;const std::size_t ids[3]={0,k,k+1};
       for(int j=0;j<3;++j){
         const auto& c=rec.corners[ids[j]];const auto p=c.viewPosition;if(!(p[2]>0)){valid=false;break;}
         const auto a=c.attributes;
         auto adapted=adaptPackedSceneMeshVertex(camera.cx+camera.focalX*p[0]/p[2],camera.cy-camera.focalY*p[1]/p[2],p[2],scale,root.record.projectionBias,{a[2],a[3],a[4],a[5]},a[0],a[1]);
         if(!adapted){valid=false;break;}
         v[j]=*adapted;
       }
       if(!valid){++stats.rejected;++stats.projectionRejected;continue;}
       ++stats.rasterCalls;
       ++sub.rasterCalls;
       prepared.draws.push_back({{&whiteSceneTexture, v, opts}, &root.source,
                                 rec.recordSourceOffset, k - 1, stats.fluidSubmissions.size(), true});
     }
     if (describeDraws)
       prepared.describeRange(first, rec.nodeSourceOffset, {}, PreparedStage::DrawKind::Fluid);
   }
   stats.fluidSubmissions.push_back(sub);
 }
}
BackdropScene::PreparedStage BackdropScene::prepareStage(
    const CameraBasis& camera, const AssetLibrary& lib, Stage stage,
    bool trackHighlights, bool observeCandidates, bool describeDraws) const {
 PreparedStage prepared{};
 prepared.stage = stage;
 prepared.levelId = levelId_;
 if (stage == Stage::FluidGate) {
   prepareFluid(prepared, camera, observeCandidates, describeDraws);
   return prepared;
 }
 auto& stats = prepared.stats;
 const auto candidate = [&](std::uint64_t source, std::uint64_t node, std::uint64_t block,
                            std::uint64_t count) {
   if (observeCandidates) prepared.candidates.push_back({prepared.draws.size(), source, node, block, count});
 };
 const bool gadgetEffectPass = stage == Stage::GadgetEffect;
 const bool teleportEffectPass = stage == Stage::TeleportEffect;
 const auto includesRoot = [&](const OwnedRoot& root) {
   if (gadgetEffectPass)
     return root.source.owner == "tagged-gravity" || root.source.owner == "tagged-pressure-pad";
   if (teleportEffectPass) return root.visible && root.source.owner.starts_with("teleport-");
   return root.source.stage == stage;
 };
 if (stage != Stage::Primary && std::none_of(roots_.begin(), roots_.end(), includesRoot)) return prepared;
 prepared.active = true;
 // Resolve diagnostic identities through owned-tree ancestry too; offsets
 // identify source records but never select their rendering stage.
 std::map<std::size_t,const OwnedRoot*> nodeOwners;
 std::map<std::size_t,double> nodeBias;
 for(const auto& root:roots_){
   if (!includesRoot(root)) continue;
   const auto own=[&](const auto& self,const SceneNode& n)->void{
     nodeOwners.emplace(n.sourceOffset,&root);
     nodeBias.emplace(n.sourceOffset,root.projectionBias);
     for(const auto& child:n.children)self(self,child);
   };own(own,root.node);
 }
 const auto highlightOwner = [&](std::size_t node) -> SceneHighlightId {
   if (!trackHighlights || (stage != Stage::LegacyUnattributed && !gadgetEffectPass && !teleportEffectPass)) return {};
   return {SceneHighlightKind::BackdropRoot,
           static_cast<std::uint32_t>(nodeOwners.at(node) - roots_.data())};
 };
 const auto describe=[&](std::size_t node,std::size_t source){
   return webo::app::context(levelId_,nodeOwners.at(node)->source,source);
 };
 const auto diagnose=[&](const std::string& message){
   stats.diagnostics.push_back(message);
   std::fprintf(stderr,"backdrop: %s\n",message.c_str());
 };
 const auto getTexture = [&](const std::string& name, std::size_t node,
                             std::size_t source) -> const SceneRgbaImage* {
   if (name.empty()) return nullptr;
   // The flag-4 alpha surface upload (0x473A8D) sets alpha on nonzero words.
   const bool alphaFromRgb = (gadgetEffectPass && name == gravityShaftMaterial) ||
                             (teleportEffectPass && textureNameFlags(name) == 4);
   auto texture = stageTexture(lib, "TEXTURES\\" + name + ".BMP", alphaFromRgb);
   if (texture) return *texture;
   ++stats.missingTextures;
   diagnose(describe(node, source) + "missing texture " + name + ": " + texture.error());
   return nullptr;
 };
  const auto context=predicateContext(camera);
  const auto meshProjectionScale=buildBillboardCameraChain(camera).projectionTransform[10];
  std::map<std::size_t,std::array<double,3>> eyes;
   std::map<std::size_t,std::vector<SceneMeshRecordSource>> secondaryMeshes;
   std::vector<SceneMeshRecordSource> primaryMeshes, legacyMeshes;
  const auto secondaryRoot=std::find_if(roots_.begin(),roots_.end(),[](const auto& n){return n.source.stage==Stage::Secondary;});
  for (const auto& owned : roots_) {
    const auto& root = owned.node;
    const bool gravityRoot = owned.source.owner == "tagged-gravity";
    const bool pressurePadRoot = owned.source.owner == "tagged-pressure-pad";
    const bool teleportRoot = owned.source.owner.starts_with("teleport-");
    if (stage == Stage::Primary) ++stats.roots;
    // The primary pass counts all owned roots. Later passes visit only their own.
    // Local facing depends on ancestors within the owned root.
    if (!includesRoot(owned) || !owned.visible) {
      if (stage == Stage::Primary) {
        // Count records without transforming those outside this stage. Animation preserves topology.
        const auto count = [&](const auto& self, const SceneNode& n) -> std::size_t {
          std::size_t total = 0;
          if (n.type == 1) for (const auto& mesh : n.meshes) total += mesh.records.size();
          for (const auto& child : n.children) total += self(self, child);
          return total;
        };
        stats.meshRecords += count(count, root);
      }
      continue;
    }
    collectEyes(root, identity(), context, eyes);
    auto records = buildSceneMeshSource(root, identity(), camera.worldToView);
    if (!records) {
      const auto message = webo::app::context(levelId_, owned.source, root.sourceOffset) + records.error();
      stats.diagnostics.push_back(message);
      std::fprintf(stderr, "backdrop: %s\n", message.c_str());
      continue;
    }
    for (auto& rec : *records) {
      if (stage == Stage::Primary) ++stats.meshRecords;
      if (gravityRoot || pressurePadRoot) {
        const auto flags = textureNameFlags(rec.materialName);
        const bool additiveEffect = gravityRoot ? rec.blendSelector == 3 && flags == 4
            : rec.blendSelector == 2 && flags == 0 && !rec.materialName.empty();
        if (additiveEffect != gadgetEffectPass) continue;
      }
      if (teleportRoot) {
        const bool additiveEffect = rec.blendSelector == 2 || rec.blendSelector == 3;
        if (additiveEffect != teleportEffectPass) continue;
      }
      if (stage == Stage::Secondary) secondaryMeshes[rec.nodeSourceOffset].push_back(std::move(rec));
      else if (stage == Stage::Primary) primaryMeshes.push_back(std::move(rec));
      else legacyMeshes.push_back(std::move(rec));
    }
  }
  const auto drawMesh = [&](const SceneMeshRecordSource& rec) {
    candidate(rec.recordSourceOffset, rec.nodeSourceOffset, rec.blockSourceOffset,
              rec.corners.size() > 2 ? rec.corners.size() - 2 : 0);
    const bool isNull = rec.materialName.empty();
    if (isNull) ++stats.nullMaterials;
    const auto* tex = isNull ? &whiteSceneTexture :
        getTexture(rec.materialName, rec.nodeSourceOffset, rec.recordSourceOffset);
    if (!tex) return;
    const bool gravityEffect = gadgetEffectPass && nodeOwners.at(rec.nodeSourceOffset)->source.owner == "tagged-gravity";
    if(!passesFacingPredicate(rec,eyes,gravityEffect)){++stats.predicateRejected;return;}
    auto state=options(false,rec.blendSelector==1);
    if (gadgetEffectPass) {
      // Flag 4 modulates alpha for selector-3 coils (SrcAlpha/One). Plain
      // pad textures select texture alpha, with selector 2 adding One/One.
      state = blendOptions(false, rec.blendSelector);
      // Test the glow against opaque scenery. A transparent stripe must not
      // mask other glow or actors. Native device depth state is inherited.
      if (gravityEffect) state.depthWrite = false;
      if (textureNameFlags(rec.materialName) == 0) state.alphaOperation = SceneAlphaOperation::SelectTexture;
    } else if (teleportEffectPass) {
      const auto flags = textureNameFlags(rec.materialName);
      state = blendOptions((flags & 3) != 0, rec.blendSelector);
      if ((flags & (4 | 16)) == 0) state.alphaOperation = SceneAlphaOperation::SelectTexture;
      // Provisional clone occlusion for additive light. Solids and actor
      // silhouettes can hide it, and the light leaves both depths intact.
      state.depthWrite = false;
    } else if (rec.materialName.starts_with(mineBackdropMaterials)) {
      const auto flags = textureNameFlags(rec.materialName);
      state = blendOptions((flags & 3) != 0, rec.blendSelector);
      // 0x47261F: ~ and + modulate alpha. Other mine textures select texture alpha.
      if ((flags & (4 | 16)) == 0) state.alphaOperation = SceneAlphaOperation::SelectTexture;
    }
    state.highlight = highlightOwner(rec.nodeSourceOffset);
    state.highlightTextureRgb = !isNull;
    const auto first = prepared.draws.size();
    for(std::size_t k=1;k+1<rec.corners.size();++k){std::array<SceneRasterVertex,3> v{};bool valid=true;const std::size_t ids[3]={0,k,k+1};
    for(int j=0;j<3;++j){const auto& c=rec.corners[ids[j]];const auto p=c.viewPosition;if(!(p[2]>0)){valid=false;break;}auto a=c.attributes;
       auto adapted=adaptPackedSceneMeshVertex(
           camera.cx+camera.focalX*p[0]/p[2], camera.cy-camera.focalY*p[1]/p[2], p[2],
           meshProjectionScale, nodeBias.at(rec.nodeSourceOffset), {a[2],a[3],a[4],a[5]},
           a[0], a[1], rec.materialUvOffset);
      if(!adapted){diagnose(describe(rec.nodeSourceOffset,rec.recordSourceOffset)+"mesh vertex adapter: "+adapted.error());valid=false;break;}v[j]=*adapted;
    }
     if (valid) {
       ++stats.rasterCalls;
       prepared.draws.push_back({{tex, v, state}, &nodeOwners.at(rec.nodeSourceOffset)->source,
                                 rec.recordSourceOffset, k - 1, std::nullopt, isNull});
     } else {
       ++stats.rejected;
       ++stats.projectionRejected;
     }
   }
   if (describeDraws)
     prepared.describeRange(first, rec.nodeSourceOffset, rec.materialName, PreparedStage::DrawKind::Mesh);
  };
   const auto& meshes=stage==Stage::Primary?primaryMeshes:legacyMeshes;
   if(stage==Stage::Primary)stats.stageMeshRecords+=primaryMeshes.size();
   else if(stage!=Stage::Secondary)stats.stageMeshRecords+=legacyMeshes.size();
   else for(const auto& [offset,items]:secondaryMeshes){(void)offset;stats.stageMeshRecords+=items.size();}
   if(stage!=Stage::Secondary)for(const auto& rec:meshes)drawMesh(rec);
  const auto builtChain=buildBillboardCameraChain(camera);
  BillboardSourceInputs chain;chain.parentCamera.values=builtChain.parentCamera;
  chain.sourceTransform.values=builtChain.sourceTransform;
  chain.projectionTransform.values=builtChain.projectionTransform;chain.center={camera.cx,camera.cy};
   std::map<std::size_t,SceneBillboardSource> secondaryBillboards;
   std::vector<SceneBillboardSource> primaryBillboards, legacyBillboards;
   // Register branch-1 sprites with ancestor-composed inputs. Choose the start frame
   // and quad only when the Secondary dispatch draws the node.
   std::map<std::size_t,PendingSprite> pendingSprites;
   collectBillboards(chain,[&](const RootSource& source,const SceneBillboardSource& b){if(source.stage!=stage)return;if(source.stage==Stage::Secondary)secondaryBillboards.emplace(b.sourceOffset,b);else if(source.stage==Stage::Primary)primaryBillboards.push_back(b);else legacyBillboards.push_back(b);},[&](const std::string& message){stats.diagnostics.push_back(message);std::fprintf(stderr,"backdrop: %s\n",message.c_str());},
     [&](const RootSource& source,const SceneNode& n,const BillboardSourceInputs& in){if(source.stage==Stage::Secondary)pendingSprites.emplace(n.sourceOffset,PendingSprite{&n,in});}, stage);

   const auto drawSprite=[&](const PendingSprite& ps){
    const SceneNode& n=*ps.node;
    const auto info=sprites_.find(n.sourceOffset);if(info==sprites_.end())return;
    if(n.type2.visibility==0)return;  // 0x0047D0D9: invisible nodes draw nothing (and do not latch)
    const Sprite& clip=sheets_.at(info->second.sheetName).sprites[info->second.clip];
    auto& st=info->second.state;
    if(!st.firstDrawDone){
      CrtRand& rng=random_?*random_:fallbackRandom_;
      if(!firstDrawState_)firstDrawState_=rng.state();
      latchSpriteFirstDrawFrame(st,static_cast<std::uint32_t>(clip.animations.size()),rng.unit());
      ++telemetry().firstDrawLatches;
    }
    const std::size_t index=std::min<std::size_t>(st.temporal,clip.animations.size()-1);
    const SpriteFrame& image=clip.animations[index].frames[0];
    BackdropStats::SpriteSubmission sub;sub.sourceOffset=n.sourceOffset;sub.temporal=st.temporal;sub.mirror=info->second.mirror;
    const auto finish=[&]{stats.spriteSubmissions.push_back(std::move(sub));};
    auto quad=makeSceneSpriteQuad(n,ps.inputs,image,info->second.mirror);
    if(!quad){++stats.rejected;++stats.projectionRejected;sub.reason="geometry: "+quad.error();diagnose(describe(n.sourceOffset,n.sourceOffset)+"sprite geometry: "+quad.error());finish();return;}
    if (stage != Stage::Primary) candidate(n.sourceOffset, n.sourceOffset, UINT64_MAX, 2);
    const std::string asset = sheetAssetName(image.sheet);
    const auto texture = stageTexture(lib, asset, false);
    if (!texture) {
      ++stats.missingTextures;
      sub.reason = "missing texture " + asset;
      diagnose(describe(n.sourceOffset, n.sourceOffset) + "missing texture " + asset + ": " + texture.error());
      finish();
      return;
    }
    ++stats.sprites;
    const auto state=spriteOptions(quad->blendSelector);
    const auto first = prepared.draws.size();
    std::uint64_t bi=0;
    for(const auto& tri:{std::array<int,3>{0,3,2},std::array<int,3>{0,2,1}}){
      std::array<SceneRasterVertex,3> vv{};bool ok=true;
      for(int j=0;j<3;++j){const auto& q=quad->vertices[tri[j]];if(!(q.z>0)){ok=false;break;}vv[j]=billboardRasterVertex(q,builtChain,{255,255,255,255});}
      if(!ok){++stats.rejected;++stats.projectionRejected;++sub.culled;if(sub.reason.empty())sub.reason="culled: non-positive depth";++bi;continue;}
      ++stats.rasterCalls;
      ++stats.spriteRasterCalls;
      ++sub.rasterCalls;
      prepared.draws.push_back({{*texture, vv, state}, &nodeOwners.at(n.sourceOffset)->source,
                                n.sourceOffset, bi, stats.spriteSubmissions.size()});
      ++bi;
    }
    if (describeDraws)
      prepared.describeRange(first, n.sourceOffset, asset, PreparedStage::DrawKind::Sprite);
    finish();
   };
   if (stage == Stage::Primary)
     for (const auto& b : primaryBillboards) candidate(b.sourceOffset, b.sourceOffset, UINT64_MAX, 2);
   const auto drawBillboard = [&](const SceneBillboardSource& b) {
    if (stage != Stage::Primary) candidate(b.sourceOffset, b.sourceOffset, UINT64_MAX, 2);
    const std::string& name = b.textureName;
    const auto* tex = name.empty() ? &whiteSceneTexture : getTexture(name, b.sourceOffset, b.sourceOffset);
    if (!tex) { ++stats.nullMaterials; return; }
    ++stats.billboards;
    // The plain Light02 bitmap (flags 0) shares the caret texture's alpha row
    // in 0x47261F. The node's independent blend selector reaches 0x414D80,
    // where selector 2 adds the texture RGB to the backdrop with ONE/ONE.
    auto st=conditionalCaretFlags8Policy(blendOptions(false,b.blendSelector));
    // Provisional clone policy: depth-test additive light and retain the
    // underlying scene depth for later scenery and actor clipping.
    if (st.destinationBlend == SceneBlendFactor::One) st.depthWrite = false;
    st.highlight = highlightOwner(b.sourceOffset);
    st.highlightTextureRgb = !name.empty();
    const auto first = prepared.draws.size();
    std::uint64_t bi = 0;
    for (const auto& tri : {std::array<int,3>{0,3,2}, std::array<int,3>{0,2,1}}) {
      std::array<SceneRasterVertex,3> vv{};
      bool valid = true;
      for (int j = 0; j < 3; ++j) {
        const auto& q = b.slots[tri[j]];
        if (!(q.z > 0)) { valid = false; break; }
        vv[j] = billboardRasterVertex(q, builtChain, {255,255,255,255});
      }
      if (valid) {
        ++stats.rasterCalls;
        prepared.draws.push_back({{tex, vv, st}, &nodeOwners.at(b.sourceOffset)->source,
                                  b.sourceOffset, bi, std::nullopt});
      } else {
        ++stats.rejected;
        ++stats.projectionRejected;
      }
      ++bi;
    }
    if (describeDraws)
      prepared.describeRange(first, b.sourceOffset, name, PreparedStage::DrawKind::Billboard);
   };
   const auto& billboards=stage==Stage::Primary?primaryBillboards:legacyBillboards;
   if(stage==Stage::Primary)stats.stageBillboards+=primaryBillboards.size();
   else if(stage!=Stage::Secondary)stats.stageBillboards+=legacyBillboards.size();
   else{stats.stageBillboards+=secondaryBillboards.size();stats.stageSprites+=pendingSprites.size();}
   if(stage!=Stage::Secondary)for(const auto& b:billboards)drawBillboard(b);
   if(stage==Stage::Secondary&&secondaryRoot!=roots_.end())dispatchSecondaryChildren(secondaryRoot->node,nativeAffine(chain.parentCamera),nativeAffine(chain.sourceTransform),[&](const SecondaryDispatchItem& item){
    if(item.type==1){auto it=secondaryMeshes.find(item.nodeSourceOffset);if(it!=secondaryMeshes.end())for(const auto& rec:it->second)drawMesh(rec);}
    else if(item.type==2){auto it=secondaryBillboards.find(item.nodeSourceOffset);if(it!=secondaryBillboards.end())drawBillboard(it->second);
      else{auto sp=pendingSprites.find(item.nodeSourceOffset);if(sp!=pendingSprites.end())drawSprite(sp->second);}}
  },dispatchOptions());
 return prepared;
}
std::expected<const SceneRgbaImage*, std::string> BackdropScene::stageTexture(
    const AssetLibrary& library, const std::string& asset, bool alphaFromRgb) const {
  auto key = std::tuple{&library, asset, alphaFromRgb};
  if (const auto found = stageTextures_.find(key); found != stageTextures_.end())
    return &found->second;
  auto image = loadTexture(library, asset);
  if (!image) return std::unexpected(image.error());
  if (alphaFromRgb) {
    // Keep the black rows transparent before bilinear filtering.
    for (std::size_t p = 0; p < image->rgba.size(); p += 4)
      image->rgba[p + 3] = (image->rgba[p] || image->rgba[p + 1] || image->rgba[p + 2]) ? 255 : 0;
  }
  return &stageTextures_.emplace(std::move(key), rgba(std::move(*image))).first->second;
}

void BackdropScene::drawStage(Raster& target, const PreparedStage& prepared, BackdropStats& stats,
    const SceneRasterObserver* observer, std::span<double> depthOutput) {
  if (!prepared.active) return;
  const auto& prep = prepared.stats;
  for (const auto field : {&BackdropStats::roots, &BackdropStats::meshRecords,
      &BackdropStats::stageMeshRecords, &BackdropStats::nullMaterials, &BackdropStats::stageBillboards,
      &BackdropStats::billboards, &BackdropStats::predicateRejected, &BackdropStats::missingTextures,
      &BackdropStats::rejected, &BackdropStats::projectionRejected, &BackdropStats::rasterCalls,
      &BackdropStats::stageSprites, &BackdropStats::sprites, &BackdropStats::spriteRasterCalls})
    stats.*field += prep.*field;
  stats.diagnostics.insert(stats.diagnostics.end(), prep.diagnostics.begin(), prep.diagnostics.end());
  const auto firstSprite = stats.spriteSubmissions.size();
  const auto firstFluid = stats.fluidSubmissions.size();
  stats.spriteSubmissions.insert(stats.spriteSubmissions.end(),
      prep.spriteSubmissions.begin(), prep.spriteSubmissions.end());
  stats.fluidSubmissions.insert(stats.fluidSubmissions.end(),
      prep.fluidSubmissions.begin(), prep.fluidSubmissions.end());

  const bool fluid = prepared.stage == Stage::FluidGate;
  profile::ScenePass pass(fluid ? profile::FluidRaster : profile::BackdropRaster);
  auto& rgb = target.mutableFramebuffer().rgb;
  auto& depth = target.mutableDepthBuffer();
  SceneRgbaFramebuffer fb;
  fb.width = target.framebuffer().width;
  fb.height = target.framebuffer().height;
  fb.highlights = target.highlightPixels();
  const auto initialDepth = observer && !fluid ? depth : std::vector<double>{};
  const auto initialRgb = observer && fluid ? rgb : std::vector<std::uint8_t>{};
  if (observer) {
    fb.rgba.resize(depth.size() * 4);
    for (std::size_t i = 0; i < depth.size(); ++i) {
      std::copy_n(rgb.data() + 3*i, 3, fb.rgba.data() + 4*i);
      fb.rgba[4*i + 3] = 255;
    }
  } else {
    fb.rgb = rgb;
  }

  detail::RasterCommandBatch batch(fb.height, !observer);
  auto candidate = prepared.candidates.begin();
  const auto reportCandidates = [&](std::size_t beforeDraw) {
    if (!observer || !observer->candidate) return;
    while (candidate != prepared.candidates.end() && candidate->beforeDraw == beforeDraw) {
      observer->candidate(candidate->source, candidate->node, candidate->block, candidate->triangleCount);
      ++candidate;
    }
  };
  for (std::size_t i = 0; i < prepared.draws.size(); ++i) {
    reportCandidates(i);
    const auto& draw = prepared.draws[i];
    auto material = draw.triangle.material;
    BackdropStats::SpriteSubmission* sprite = nullptr;
    BackdropStats::FluidSubmission* receiver = nullptr;
    if (draw.submission) {
      if (fluid) receiver = &stats.fluidSubmissions[firstFluid + *draw.submission];
      else sprite = &stats.spriteSubmissions[firstSprite + *draw.submission];
    }
    if (!fluid && !sprite) material.depthOutput = depthOutput;
    if (prepared.stage == Stage::TeleportEffect) material.actorDepth = &target.actorDepth();

    SceneRasterObserver tagged;
    if (observer) {
      tagged.mask = observer->mask;
      tagged.source = draw.source;
      tagged.triangle = draw.ordinal;
      if (receiver || sprite || observer->accepted) {
        tagged.accepted = [&](const SceneRasterWriteEvent& event) {
          ++stats.acceptedColorWrites;
          stats.acceptedDepthWrites += event.depthAssigned;
          if (receiver) {
            ++receiver->acceptedColorWrites;
            receiver->acceptedDepthWrites += event.depthAssigned;
          }
          if (sprite) {
            ++stats.spriteAcceptedColorWrites;
            stats.spriteAcceptedDepthWrites += event.depthAssigned;
          }
          if (observer->accepted) observer->accepted(event);
        };
      }
    }
    const auto result = rasterizeSceneTriangle(fb, depth, *draw.triangle.texture,
        draw.triangle.vertices, material, observer ? &tagged : nullptr);
    if (result) {
      ++stats.meshTriangles;
      if (draw.nullMaterial) ++stats.nullMaterialTriangles;
      if (receiver) ++receiver->triangles;
      if (sprite) ++stats.spriteTriangles;
      continue;
    }
    ++stats.rejected;
    ++stats.rasterRejected;
    const auto reason = "raster rejected: " + result.error();
    if (sprite) {
      ++sprite->rejected;
      if (sprite->reason.empty()) sprite->reason = reason;
    }
    stats.diagnostics.push_back(context(prepared.levelId, *draw.root, draw.source) + reason);
    if (!fluid) std::fprintf(stderr, "backdrop: %s\n", stats.diagnostics.back().c_str());
  }
  reportCandidates(prepared.draws.size());
  {
    profile::Scope rasterTime(profile::active ? profile::active->sceneRaster : profile::BackdropRaster);
    const auto errors = fluid ? flushCountingChanges<3>(batch, rgb, fb.width, stats.fluidPixelsChanged) :
                                flushCountingChanges<1>(batch, depth, fb.width, stats.pixels);
    for (const auto& error : errors) {
      --stats.meshTriangles;
      ++stats.rejected;
      ++stats.rasterRejected;
      stats.diagnostics.push_back(error);
      std::fprintf(stderr, "backdrop: raster rejected: %s\n", error.c_str());
    }
  }
  if (observer)
    for (std::size_t i = 0; i < depth.size(); ++i)
      std::copy_n(fb.rgba.data() + 4*i, 3, rgb.data() + 3*i);
  for (std::size_t i = 0; i < initialDepth.size(); ++i)
    if (depth[i] != initialDepth[i]) ++stats.pixels;
  for (std::size_t i = 0; i < initialRgb.size(); i += 3)
    if (rgb[i] != initialRgb[i] || rgb[i+1] != initialRgb[i+1] || rgb[i+2] != initialRgb[i+2])
      ++stats.fluidPixelsChanged;
}

void BackdropScene::renderStage(Raster& target, const CameraBasis& camera, const AssetLibrary& lib,
    BackdropStats& stats, Stage stage, SceneRasterObserver* observer, std::vector<double>* depthOutput) const {
  const auto prepared = prepareStage(camera, lib, stage, !target.highlightPixels().empty(),
                                    observer && observer->candidate);
  drawStage(target, prepared, stats, observer, depthOutput ? std::span(*depthOutput) : std::span<double>{});
}

void BackdropScene::collectBillboards(const BillboardSourceInputs& chain,
    const std::function<void(const RootSource&,const SceneBillboardSource&)>& visitor,
    const std::function<void(const std::string&)>& error,
    const SpriteVisitor& sprite, std::optional<Stage> stage) const {
 for(const auto& owned:roots_){
   if (stage && owned.source.stage != *stage) continue;
   const auto walk=[&](const auto& self,const SceneNode& n,const Matrix& ancestors)->void{
     if(n.type==2){auto in=chain;in.parentCamera=multiply(chain.parentCamera,affine(ancestors));
       if(n.type2.branch!=0){
         // Branch-1 sprites are not SceneBillboardSource records; only a caller that can
         // realize them (renderStage) supplies a sprite visitor.
         if(sprite)sprite(owned.source,n,in);
       }else{
         auto b=makeSceneBillboardSource(n,in);
         if(b)visitor(owned.source,*b);
         else error(context(levelId_,owned.source,n.sourceOffset)+b.error());
       }
     }
     const Matrix next=compose(n.matrix,ancestors);
     for(const auto& child:n.children)self(self,child,next);
   };
   walk(walk,owned.node,identity());
 }
}
BackdropScene::SceneryTelemetry& BackdropScene::telemetry(){static SceneryTelemetry t;return t;}
void BackdropScene::advanceScenery(double frameMs,bool inLevel,bool paused){
 double dt=0.0;
 auto& tele=telemetry();++tele.advanceCalls;
 if(inLevel&&!paused&&std::isfinite(frameMs)&&frameMs>0.0){dt=std::min(frameMs,100.0)/1000.0;if(!sprites_.empty())++tele.advancesWithTime;}
 // 0x00437BDE adds dt to the secondary clip phase and wraps it at its duration.
 advanceSecondaryAnimation(dt);
 // 0x004764C4 uses t - previous as node delta, or t after a negative delta.
 sceneryClock_+=dt;
 double delta=sceneryClock_-sceneryPrevious_;
 if(delta<0.0)delta=sceneryClock_;
 sceneryPrevious_=sceneryClock_;
 for(auto& [offset,node]:sprites_){
   (void)offset;
   const Sprite& clip=sheets_.at(node.sheetName).sprites[node.clip];
   const auto before=node.state.temporal;
   advanceSpriteTemporal(node.state,delta,clip.rate,static_cast<std::uint32_t>(clip.animations.size()));
   if(node.state.temporal!=before)++tele.temporalSteps;
 }
}
const SpriteTemporalState* BackdropScene::spriteState(std::size_t sourceOffset) const {
 const auto it=sprites_.find(sourceOffset);
 return it==sprites_.end()?nullptr:&it->second.state;
}
void BackdropScene::visitBillboards(const BillboardSourceInputs& chain,const std::function<void(const SceneBillboardSource&)>& visitor) const {
 collectBillboards(chain,[&](const RootSource&,const SceneBillboardSource& b){visitor(b);},
     [](const std::string& message){std::fprintf(stderr,"backdrop: %s\n",message.c_str());});
}
void BackdropScene::visitKind5Predicates(const CameraBasis& camera,const std::function<void(std::size_t,bool)>& visitor) const {
  const auto context=predicateContext(camera);std::map<std::size_t,std::array<double,3>> eyes;
  for(const auto& root:roots_)collectEyes(root.node,identity(),context,eyes);
  for(const auto& root:roots_){auto records=buildSceneMeshSource(root.node,identity(),camera.worldToView);
    if(!records){std::fprintf(stderr,"backdrop: %s%s\n",webo::app::context(levelId_,root.source,root.node.sourceOffset).c_str(),records.error().c_str());continue;}
    for(const auto& rec:*records)if(rec.kind==5&&!rec.corners.empty())visitor(rec.recordSourceOffset,passesFacingPredicate(rec,eyes));}
}
void BackdropScene::visitSecondaryDispatch(const CameraBasis& camera,const std::function<void(const SecondaryDispatchItem&)>& visitor) const {
 const auto root=std::find_if(roots_.begin(),roots_.end(),[](const auto& n){return n.source.stage==Stage::Secondary;});if(root==roots_.end())return;
 const auto chain=buildBillboardCameraChain(camera);dispatchSecondaryChildren(root->node,nativeAffine(BillboardAffine3x4{chain.parentCamera}),nativeAffine(BillboardAffine3x4{chain.sourceTransform}),visitor,dispatchOptions());
}
// Dispatch applies 0x46C56B/0x46C5C4/0x47D0D9 visibility gates. The "cutme"
// frame-time LOD (0x45DA0D) requires a caller's setSecondaryHidden override.
SecondaryDispatchOptions BackdropScene::dispatchOptions() const {SecondaryDispatchOptions o;o.originalGates=true;o.hidden=secondaryHidden_;return o;}
void BackdropScene::visitRoots(const std::function<void(const RootSource&,const SceneNode&)>& visitor) const {
 for(const auto& root:roots_)visitor(root.source,root.node);
}
}
