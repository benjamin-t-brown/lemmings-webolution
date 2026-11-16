#include "app/FrameProfile.h"
#include "app/BackdropScene.h"
#include "app/Raster.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneMeshSource.h"
#include "app/SceneMaterialFlags.h"
#include "app/SceneDepth.h"
#include "app/SceneDiffuse.h"
#include "app/SceneDispatch.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "sim/World.h"
#include <algorithm>
#include <array>
#include <bit>
#include <map>
#include <cmath>
#include <cstdio>
#include <format>
#include <stdexcept>
#include "lemmings/compiled_level.hpp"

namespace webo::app {
namespace {
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
BillboardAffine3x4 multiply(const BillboardAffine3x4&a,const BillboardAffine3x4&b){BillboardAffine3x4 o;for(int r=0;r<3;++r){for(int c=0;c<3;++c)for(int k=0;k<3;++k)o.values[4*r+c]+=a.values[4*r+k]*b.values[4*k+c];o.values[4*r+3]=a.values[4*r+3];for(int k=0;k<3;++k)o.values[4*r+3]+=a.values[4*r+k]*b.values[4*k+3];}return o;}
std::array<double,3> eyeLocal(const Matrix&m){double a=m[0],b=m[1],c=m[2],d=m[3],e=m[4],f=m[5],g=m[6],h=m[7],i=m[8];double det=a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g);if(std::abs(det)<1e-15)return {NAN,NAN,NAN};double q=1/det;double inv[9]={(e*i-f*h)*q,(c*h-b*i)*q,(b*f-c*e)*q,(f*g-d*i)*q,(a*i-c*g)*q,(c*d-a*f)*q,(d*h-e*g)*q,(b*g-a*h)*q,(a*e-b*d)*q};std::array<double,3> out{};for(int j=0;j<3;++j)for(int k=0;k<3;++k)out[j]-=m[9+k]*inv[3*k+j];return out;}
Matrix nativeAffine(const BillboardAffine3x4& a){return {a.values[0],a.values[4],a.values[8],a.values[1],a.values[5],a.values[9],a.values[2],a.values[6],a.values[10],a.values[3],a.values[7],a.values[11]};}
// Reuse the accepted camera builder: its parent includes the camera/projection
// chain and its source transform is S. Convert both affine matrices back to the
// SceneNode row-vector layout before composing the predicate context.
Matrix predicateContext(const CameraBasis& camera){const auto chain=buildBillboardCameraChain(camera);return compose(nativeAffine(BillboardAffine3x4{chain.parentCamera}),nativeAffine(BillboardAffine3x4{chain.sourceTransform}));}
void collectEyes(const SceneNode& n,const Matrix& parent,const Matrix& context,std::map<std::size_t,std::array<double,3>>& out){auto m=compose(n.matrix,parent);out[n.sourceOffset]=eyeLocal(compose(m,context));for(auto&c:n.children)collectEyes(c,m,context,out);}
// Kinds 4 and 5 share the facing predicate (0x46AE20 / 0x46AFA0, fluid research 6.3 B).
bool passesKind5(const SceneMeshRecordSource& rec,const std::map<std::size_t,std::array<double,3>>& eyes){if((rec.kind!=4&&rec.kind!=5)||rec.corners.empty())return true;const auto it=eyes.find(rec.nodeSourceOffset);if(it==eyes.end())return false;const auto&e=it->second;const auto&v=rec.corners.front().localPosition;const auto&n=rec.corners.front().localNormal;const double facing=(e[0]-v[0])*n[0]+(e[1]-v[1])*n[1]+(e[2]-v[2])*n[2];return facing>0;}
SceneRasterOptions options(bool alpha,bool blend,bool depthWrite=true){return {SceneFilter::Linear,SceneAddress::Wrap,alpha?SceneAlphaFunction::StrictGreater:SceneAlphaFunction::Disabled,0xef,blend,SceneBlendFactor::SrcAlpha,SceneBlendFactor::InvSrcAlpha,SceneDepthFunction::Less,depthWrite,SceneShade::Gouraud,SceneRgbOperation::Modulate,SceneAlphaOperation::Modulate,0.5,0.5};}
SceneRgbaImage rgba(const Image& im){return {im.width,im.height,im.rgba};}// texture_name_flags (decomp-lemmings/src/texture_creation.cpp:21, 0x00472xxx name rule): the
// first character of the basename selects the flag. Flags 1 and 2 share the `flags & 3` row of
// 0x0047261F that the sprite render state below implements.
std::uint32_t spriteTextureFlags(const std::string& sheet){
 const auto slash=sheet.find_last_of('\\');
 const char c=sheet.empty()?'\0':sheet[slash==std::string::npos?0:slash+1];
 switch(c){case '_':return 1;case '$':return 2;case '~':return 4;case '^':return 8;case '+':return 16;default:return 0;}
}
// Blend table 0x00414D80 keyed by the node's trailing selector (first dword of the sub-object).
SceneRasterOptions spriteOptions(std::uint32_t selector){
 auto o=options(true,selector!=0);
 // Conditional, labelled: texture flags 1 => `flags & 3` row of 0x0047261F: SELECTARG1 alpha,
 // alpha test GREATER with reference 0xEF (options() default); RGB modulates white diffuse.
 o.alphaOperation=SceneAlphaOperation::SelectTexture;
 if(selector==2){o.sourceBlend=SceneBlendFactor::One;o.destinationBlend=SceneBlendFactor::One;}
 else if(selector==3){o.sourceBlend=SceneBlendFactor::SrcAlpha;o.destinationBlend=SceneBlendFactor::One;}
 return o;
}
}
std::expected<BackdropScene,std::string> BackdropScene::build(const AssetLibrary& lib,std::span<const std::byte> bytes,std::string_view levelId){
 BackdropScene s;
 s.levelId_=levelId;
 // Admission is structural, never by level ID: every level is attempted with the
 // serialized-ownership extraction below. This is a portable renderer-capability
 // gate (the readers/adapters below can represent the level), not recovered
 // native admission; the original loads every level.
 // The byte storage belongs to the caller and remains alive throughout both
 // readers. Only the composed reader's documented format exception is mapped.
 lemmings::CompiledLevel level;
 try {
   level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()});
 } catch(const std::invalid_argument& e) {
   return std::unexpected(std::string(levelId)+" serialized level: "+e.what());
 }
 // A failure is either malformed data (hard error: truncation, bad references,
 // marker/end contradictions) or a form the current readers/renderer do not
 // represent (the level is then a diagnosed, successful, empty partial scene so
 // gameplay is unaffected). Reader diagnostics name unsupported forms with the
 // word "unsupported"; every existing check below is unchanged.
 struct Failure{bool unsupported;std::string message;};
 const auto formFailure=[](const std::string& label,const std::string& reason){return std::unexpected(Failure{reason.find("unsupported")!=std::string::npos,label+reason});};
 // Type-2 branch-1 (sprite) admission, scene-branch-impl-001 (contract: docs/scratch/
 // scene-branch-research-002.md section 10). Structural, never by level ID or partial subset:
 // SPRITES\<name>.SPR present and parsing, clip in range with >=1 temporal entry, every
 // temporal entry exactly one directional image (the early-return path of 0x0046840B; the
 // multi-image selection 0x00418350/0x00480AC8 is not implemented), its texture present with
 // a flags row this renderer implements. A missing sprite file is an explicit error (the
 // original exits); the clone reports it and keeps the level as a diagnosed empty scene.
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
       if((spriteTextureFlags(frames[0].sheet)&3)==0)return reject("texture '"+frames[0].sheet+"' flags row is not implemented");
       if(!lib.contains(sheetAssetName(frames[0].sheet)))return reject("error: texture "+sheetAssetName(frames[0].sheet)+" is missing");
     }
     SpriteNode node;node.sheetName=name;node.clip=clipIndex;node.mirror=n->type2.branchWords[0]!=0;
     s.sprites_.emplace(n->sourceOffset,std::move(node));
   }
   return {};
 };
 const auto add=[&](RootSource source,bool marker,double projectionBias=0.0)->std::expected<void,Failure>{
   const auto& e=source.extent;
   const auto label=context(levelId,source,e.typed_root_begin);
   if(!marker)return std::unexpected(Failure{false,label+"Object 09 marker mismatch"});
   auto parsed=readSceneNode(bytes,e.typed_root_begin);
   if(!parsed)return formFailure(label,parsed.error());
   if(parsed->endOffset!=e.tree_end)return std::unexpected(Failure{false,label+"recursive tree end differs from serialized reader"});
   // Secondary slot8 admission (scene-nested-impl-001, contract: docs/scratch/scene-nested-research-001.md
   // sections 1 and 3). Structural, never by level ID: the root is type 0, every node is type 0, 1 or 2,
   // and only type-0/type-1 nodes may own children (type-1 owners: 0x0046C55D draws own meshes, then
   // children in file order). A type-2 owner's matrix is applied twice to its children by the original
   // (0x47D0F2 and 0x46B6EB); that is evidenced but unexercised by any installed level, so it is not
   // implemented and the whole level stays a diagnosed empty partial scene.
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
   auto ok=add({Stage::Primary,"paired-primary",0,level.objects.source_extent},level.objects.marker_matches);
   if(!ok)return ok;
   if(level.objects.second_object_source_extent){
     ok=add({Stage::Secondary,"paired-secondary",0,*level.objects.second_object_source_extent},level.objects.second_object_marker_matches);
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
 // Playtests T2/T5: the tagged collection was absent from scene extraction.
 // Admit type-0 pressure pads, type-1 levers and type-3 gravity models, retaining each placed root.
 // Use the same mesh/billboard depth path as other placed scenery.
 for(std::size_t i=0;i<level.gadgets.tagged_entities.size();++i){
   const auto& gadget=level.gadgets.tagged_entities[i];
   if(gadget.type!=0&&gadget.type!=1&&gadget.type!=3)continue;
   const auto& model=gadget.model;
   if(!model||model->type!=0||!model->object){
     s.diagnostics_.push_back(std::format("{} tagged type {}[{}]: unsupported model",levelId,gadget.type,i));
     continue;
   }
   auto added=add({Stage::LegacyUnattributed,gadget.type==0?"tagged-pressure-pad":gadget.type==3?"tagged-gravity":"tagged-lever",i,model->object->source_extent},model->object->marker_matches);
   if(!added)return std::unexpected(added.error().message);
 }
 // The installed Crab.a3d models belong to named STAT entities, not scenery
 // or fluid receivers. Keep their stored cylinder placement and scene depth.
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
 // moving-hazards-001: named STAT and DYNA objects were parsed but omitted
 // from scene extraction. They share the ordinary placed-mesh depth path.
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
 // 0x42C556 submits the optional +0x160 drain and +0x164 supply in receiver
 // order, unconditionally with respect to height/request. Their inline roots
 // retain authored placement/orientation (unlike the world-space Fluid root).
 // Installed pipes have no animation tracks; do not invent a flow animation.
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
     // Model+0xF8 is the third stored double (0x451865). Drains use 0.0003;
     // supplies use zero. Preserve it independently of the clone depth policy.
     auto added=add({Stage::FluidPipe,owner,i,object.source_extent},object.marker_matches,
                    std::bit_cast<double>(model->double_bits[2]));
     if(!added)return std::unexpected(added.error().message);
   }
 }
 // Animated base `Fluid` models of the entities_42acac receivers (fluid-surface-impl-001; contract:
 // docs/scratch/fluid-surface-research-001.md sections 6.2 and 7.4). Structural, never by level ID:
 // kinds 0..2 (startup constants at 0x428848) with an inline type-0 base model whose first
 // type-1 node has the traced layout are admitted. A failure omits that receiver's fluid with a
 // diagnostic; it never empties the other roles (they are extracted and validated above).
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
   // Traced layout: >=6 vertices, >=3 normals, block[1] = kind-5 top surface whose corners all
   // reference the four animated vertex slots 2..5 (0x42B24D writes exactly those).
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
       // READ 47C93D/47CF5C: second array = two-component material tracks,
       // not colour tracks. Installed anti_grav has no transform/event tracks.
       std::size_t at=0;const auto& raw=aux.serialized_bytes;
       const auto integer=[&](std::size_t size){
         if(at>raw.size()||size>raw.size()-at)throw std::invalid_argument("truncated gravity UV animation");
         std::uint64_t n=0;for(std::size_t k=0;k<size;++k)n|=std::uint64_t(raw[at++])<<(8*k);return n;
       };
       const auto number=[&](){double n=std::bit_cast<double>(integer(8));if(!std::isfinite(n))throw std::invalid_argument("nonfinite gravity UV key");return n;};
       GravityPresentation::Binding binding{root,number(),{}};
       if(binding.duration<=0||integer(4)!=0)throw std::invalid_argument("unsupported gravity transform tracks/duration");
       std::vector<const SceneNode*> nodes;
       const auto collect=[&](auto& self,const SceneNode& n)->void{nodes.push_back(&n);for(const auto& c:n.children)self(self,c);};collect(collect,owned.node);
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
           // PROVISIONAL node-index adapter, shared with the hatch track
           // convention: serialized ids address the placed tree in preorder.
           // Real anti_grav tracks select its four coil panels (1,2,4,5).
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
   const auto collect=[&](auto& self,SceneNode& n)->void{nodes.push_back(&n);for(auto& c:n.children)self(self,c);};collect(collect,roots_.at(binding.root).node);
   // PROVISIONAL playable-v1 adapter: loop the stored UV clip on the existing
   // paused scenery clock, with a shared zero phase. Native per-object start
   // phase/contact-driven sound/activation are not established here.
   const double time=std::fmod(sceneryClock_,binding.duration);
   for(const auto& track:binding.tracks){
     std::size_t k=0;while(k+1<track.keys.size()&&time>track.keys[k].end)++k;
     const auto& key=track.keys[k];const double start=k?track.keys[k-1].end:0.0;
     const double t=key.end>start?std::clamp((time-start)/(key.end-start),0.0,1.0):0.0;
     // READ 476FB0/476A80: interval fraction, then base + fraction*delta.
     const double u=key.u+t*key.du,v=key.v+t*key.dv;
     for(const auto& c:track.corners){auto& uv=nodes[c.node]->meshes[c.mesh].records[c.record].corners[c.corner];uv.u=c.u+u;uv.v=c.v+v;}
   }
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
   // Same FILE-to-runtime lattice adapter as the accepted balloon departure.
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
           // 0x476277/0x47CDE7: third array = node, axis, turns/sec.
           // 0x476EE0 uses absolute clip time, not accumulated frame angles.
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
   // PROVISIONAL orientation adapter: preserve authored tilt/scale while
   // following radial heading, as in the accepted balloon departure path.
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
     presentation.bindings.push_back({i,pad.id,static_cast<std::int32_t>(pad.values[0]),root.node,0});
   }
 } catch(const std::invalid_argument& e){return std::unexpected(e.what());}
 return presentation;
}
void BackdropScene::setPressurePadStates(PressurePadPresentation& presentation,std::span<const std::pair<int,int>> idStates){
 for(auto& binding:presentation.bindings){
   int state=binding.initialState;
   for(const auto& [id,current]:idStates)if(id==binding.id){state=current;break;}
   if(state==binding.displayedState)continue;
   auto& root=roots_.at(binding.root).node;
   root=binding.original;
   if(state==1){
     // 0x431B93..0x431BF9 finds names containing "cut"; 0x4319E5
     // hides them after first activation. Keep the base cube visible.
     // Pruning only this owned model also suppresses a cut node's subtree.
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
   // 0x436E6A selects "lever" and direction (face == -1) XOR [+0xA0].
   // Load hook 0x43703F calls model vt+0x24 (0x45130A): forward
   // samples duration, reverse samples zero. Keep that settled face mapping.
   // PROVISIONAL presentation: show the settled endpoint immediately. Native
   // pull interpolation is not connected; no simulation clock is changed.
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
 // 0x43D09A deinterleaves FILE inner/outer pairs into the RUNTIME layers
 // consumed by 0x43D56C. Match ActorPose's cylinder-template accessor here;
 // do not pass the raw serialized blocks to the runtime-coordinate helper.
 for(std::size_t row=0;row<level.world.first_blocks.size();++row)
   for(std::size_t x=0;x<32;++x)for(std::size_t layer=0;layer<2;++layer)
     presentation.exitCoordinateRows[row][layer*32+x]=level.world.first_blocks[row][x*2+layer];
 for(std::size_t i=0;i<roots_.size();++i) {
   const auto& owned=roots_[i];
   if(owned.source.owner=="entities_425e50") {
     const auto& model=level.gadgets.entities_425e50.at(owned.source.entityIndex).model;
     if(!model||!model->object)continue;
     // Water hatch models name the same two-door clip "entrance_acid".
     // Both names/layouts are measured over all 144 installed hatches.
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
void BackdropScene::setEntranceExitPresentation(const EntranceExitPresentation& presentation,double openingProgress,double departureSeconds) {
 if(!std::isfinite(openingProgress)||!std::isfinite(departureSeconds))return;
 for(const auto& entrance:presentation.entrances)
   applyHatchAnimation(roots_[entrance.root].node,entrance.animation,openingProgress);
 for(const auto& exit:presentation.exits) {
   auto& matrix=roots_[exit.root].node.matrix;
   matrix=exit.matrix;
   const double time=std::max(0.0,departureSeconds);
   if(time==0.0)continue;
   // READ 0x426A0C..0x426A95: logical X += dt*0.5, Y += dt*5,
   // radial BLEND += dt*0.5; X wraps at 256 (0x4231DE). The blend is
   // not a world-space distance: the installed lattice converts it into
   // the large outward motion missing from the former Y-only departure.
   // Model slot+8 (0x451668 -> 0x4658D6 -> 0x43D56C) maps this triple.
   auto position=exit.cylinderPosition;
   position[0]=std::fmod(position[0]+0.5*time,256.0);
   position[1]+=5.0*time;
   position[2]+=0.5*time;
   const lemmings::WorldCoordinateMesh mesh{presentation.exitCoordinateHeight,presentation.exitCoordinateRows};
   const auto start=lemmings::map_world_coordinates(mesh,exit.cylinderPosition);
   const auto moved=lemmings::map_world_coordinates(mesh,position);
   for(std::size_t j=0;j<3;++j)matrix[9+j]+=moved[j]-start[j];
   // PROVISIONAL presentation adapter: turn the stored model with its radial
   // heading, preserving its scale/tilt. Native tangent heading and phase-
   // dependent 0.02 bob/roll are not reproduced. Path rates above are traced,
   // not fitted to the recording. Keep the accepted global Pending trigger
   // and results timer; per-type departure/removal remains outside this fix.
   const double angle=std::atan2(moved[2],moved[0])-std::atan2(start[2],start[0]);
   const double c=std::cos(angle),s=std::sin(angle);
   for(std::size_t row=0;row<3;++row){
     const double x=exit.matrix[3*row],z=exit.matrix[3*row+2];
     matrix[3*row]=c*x-s*z;matrix[3*row+2]=s*x+c*z;
   }
 }
}
std::vector<HatchHoverTarget> BackdropScene::hatchHoverTargets(
    const CameraBasis& camera, const Raster& raster) const {
 std::vector<HatchHoverTarget> targets;
 if (raster.framebuffer().width != camera.width || raster.framebuffer().height != camera.height) return targets;
 const auto& depth = raster.depthBuffer();
 const auto context = predicateContext(camera);
 for (const auto& owned : roots_) {
   if (owned.source.owner != "entities_425e50") continue;
   auto records = buildSceneMeshSource(owned.node, identity(), camera.worldToView);
   if (!records) continue;
   std::map<std::size_t,std::array<double,3>> eyes;
   collectEyes(owned.node, identity(), context, eyes);
   HatchHoverTarget h{owned.source.entityIndex, camera.width, camera.height, 0, 0};
   // SOURCE visibility: bounds of mesh pixels present in the completed scene
   // depth buffer. This follows animated doors and hides rear/occluded hatches.
   // PROVISIONAL rectangular hit area rather than native model-origin +/-20;
   // a 0.5 world-unit tolerance covers raster depth conversion/rounding.
   for (const auto& record : *records) {
     if (!passesKind5(record, eyes)) continue;
     for (std::size_t k = 1; k+1 < record.corners.size(); ++k) {
       const std::size_t ids[]{0,k,k+1};
       double sx[3], sy[3], z[3]; bool valid = true;
       for (int j = 0; j < 3; ++j) {
         const auto& p = record.corners[ids[j]].viewPosition;
         z[j] = p[2];
         if (!(z[j] > 0)) { valid = false; break; }
         sx[j] = camera.cx + camera.focalX*p[0]/z[j];
         sy[j] = camera.cy - camera.focalY*p[1]/z[j];
         if (!std::isfinite(sx[j]) || !std::isfinite(sy[j])) valid = false;
       }
       if (!valid) continue;
       const double area = (sx[1]-sx[0])*(sy[2]-sy[0])-(sy[1]-sy[0])*(sx[2]-sx[0]);
       if (std::abs(area) < 1e-9) continue;
       const int x0 = int(std::clamp(std::floor(std::min({sx[0],sx[1],sx[2]})), 0.0, double(camera.width)));
       const int x1 = int(std::clamp(std::ceil(std::max({sx[0],sx[1],sx[2]})), 0.0, double(camera.width)));
       const int y0 = int(std::clamp(std::floor(std::min({sy[0],sy[1],sy[2]})), 0.0, double(camera.height)));
       const int y1 = int(std::clamp(std::ceil(std::max({sy[0],sy[1],sy[2]})), 0.0, double(camera.height)));
       for (int y = y0; y < y1; ++y) for (int x = x0; x < x1; ++x) {
         const double px=x+0.5, py=y+0.5;
         const double b=((px-sx[0])*(sy[2]-sy[0])-(py-sy[0])*(sx[2]-sx[0]))/area;
         const double c=((sx[1]-sx[0])*(py-sy[0])-(sy[1]-sy[0])*(px-sx[0]))/area;
         const double a=1-b-c;
         if (a < 0 || b < 0 || c < 0) continue;
         const double plane=a*z[0]+b*z[1]+c*z[2];
         if (std::abs(plane-depth[std::size_t(y)*camera.width+x]) > 0.5) continue;
         h.left=std::min(h.left,x); h.top=std::min(h.top,y);
         h.right=std::max(h.right,x+1); h.bottom=std::max(h.bottom,y+1);
       }
     }
   }
   if (h.left < h.right && h.top < h.bottom) targets.push_back(h);
 }
 return targets;
}
std::vector<std::size_t> BackdropScene::fluidEntityIndices() const {
 std::vector<std::size_t> out;
 for(const auto& r:fluidRoots_)out.push_back(r.record.entityIndex);
 return out;
}
void BackdropScene::render(Raster& target,const CameraBasis& camera,const AssetLibrary& lib,BackdropStats& stats) const {
  renderStage(target,camera,lib,stats,Stage::Primary);
  renderStage(target,camera,lib,stats,Stage::LegacyUnattributed);
  renderStage(target,camera,lib,stats,Stage::FluidPipe);
  renderStage(target,camera,lib,stats,Stage::FluidGate);
  renderStage(target,camera,lib,stats,Stage::Secondary);
}
// 0x0042B24D animation tail + 0x0042C5C9 normal sign, applied to a pristine copy of the parsed model.
// Constants are the three rows written by 0x428848; pi is the traced promoted-f32
// literal 3.1415927410125732, never M_PI (fluid research 7.2).
void BackdropScene::applyFluidState(SceneNode& g,const FluidRecord& rec,const FluidGateState& st){
 constexpr double kPi=3.1415927410125732;
 // 0x428848 writes rows at 0x4BF6E8 + kind*0x48; 0x42B24D consumes
 // period +0, amplitude +8, lane count +0x38. Keep kind 0 arithmetic unchanged.
 const double period=rec.kind==1?8.0:rec.kind==2?0.8:2.0;
 const double amplitude=rec.kind==1?1.0:rec.kind==2?0.25:0.5;
 const std::uint32_t lanes=rec.kind==1?16u:rec.kind==2?2u:4u;
 const double h=st.height;
 // 0x42C5C9: normals 0..2 are negated whenever the height is zero (the facing predicate is then inverted).
 if(h==0.0)for(std::size_t k=0;k<3;++k)for(double& x:g.normals[k])x=-x;
 if(!st.animated)return;  // raw deserialized mesh until the first update has run
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
 // Lane 1 colours the corners on vertex slots 2 and 4, lane 2 those on slots 3 and 5 (6.2 item 4).
 // Corner alpha is left as stored: the alpha branch is skipped for kind 0.
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
void BackdropScene::renderFluid(Raster& target,const CameraBasis& camera,BackdropStats& stats,SceneRasterObserver* observer) const {
 profile::ScenePass profilePass(profile::FluidRaster);
 if(!hasFluidState_||fluidRoots_.empty())return;
 SceneRgbaFramebuffer fb;fb.width=camera.width;fb.height=camera.height;fb.rgba.resize(target.framebuffer().rgb.size()/3*4);
 auto& rgb=target.mutableFramebuffer().rgb;auto& depth=target.mutableDepthBuffer();
 const auto before=rgb;
 for(std::size_t i=0,n=rgb.size()/3;i<n;++i){fb.rgba[4*i]=rgb[3*i];fb.rgba[4*i+1]=rgb[3*i+1];fb.rgba[4*i+2]=rgb[3*i+2];fb.rgba[4*i+3]=255;}
 const SceneRgbaImage white{1,1,{255,255,255,255}};
 const auto context=predicateContext(camera);
 const auto scale=buildBillboardCameraChain(camera).projectionTransform[10];
 // Named provisional depth policy (see FluidDepthPolicy); the software projection bias below is the
 // traced model+0xF8 contribution, kept separate from any unknown device ZBIAS.
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
     if(observer&&observer->candidate)observer->candidate(rec.recordSourceOffset,rec.nodeSourceOffset,rec.blockSourceOffset,rec.corners.size()>2?rec.corners.size()-2:0);
     if(!passesKind5(rec,eyes)){++stats.predicateRejected;++sub.predicateRejected;continue;}
     const auto opts=optionsFor(rec.blendSelector==1);
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
       SceneRasterObserver tagged;
       if(observer){tagged=*observer;tagged.source=rec.recordSourceOffset;tagged.triangle=k-1;
         tagged.accepted=[&](const SceneRasterWriteEvent& e){++stats.acceptedColorWrites;stats.acceptedDepthWrites+=e.depthAssigned;++sub.acceptedColorWrites;sub.acceptedDepthWrites+=e.depthAssigned;if(observer->accepted)observer->accepted(e);};}
       ++stats.rasterCalls;++sub.rasterCalls;
       auto r=rasterizeSceneTriangle(fb,depth,white,v,opts,observer?&tagged:nullptr);
       if(r){++stats.meshTriangles;++sub.triangles;++stats.nullMaterialTriangles;}
       else{++stats.rejected;++stats.rasterRejected;stats.diagnostics.push_back(webo::app::context(levelId_,root.source,rec.recordSourceOffset)+"raster rejected: "+r.error());}
     }
   }
   stats.fluidSubmissions.push_back(sub);
 }
 for(std::size_t i=0,n=rgb.size()/3;i<n;++i){rgb[3*i]=fb.rgba[4*i];rgb[3*i+1]=fb.rgba[4*i+1];rgb[3*i+2]=fb.rgba[4*i+2];}
 for(std::size_t i=0,n=rgb.size()/3;i<n;++i)if(rgb[3*i]!=before[3*i]||rgb[3*i+1]!=before[3*i+1]||rgb[3*i+2]!=before[3*i+2])++stats.fluidPixelsChanged;
}
void BackdropScene::renderStage(Raster& target,const CameraBasis& camera,const AssetLibrary& lib,BackdropStats& stats,Stage stage,SceneRasterObserver* observer) const {
 if(stage==Stage::FluidGate){renderFluid(target,camera,stats,observer);return;}
 // SceneRaster and legacy Raster share the same linear view-depth units here;
 // the choice is conditional software integration, not an effective-D3D claim.
 SceneRgbaFramebuffer fb; fb.width=camera.width;fb.height=camera.height;fb.rgba.resize(target.framebuffer().rgb.size()/3*4);
 auto& rgb=target.mutableFramebuffer().rgb;auto& depth=target.mutableDepthBuffer();
 const auto initialDepth=depth;
 for(std::size_t i=0,n=rgb.size()/3;i<n;++i){fb.rgba[4*i]=rgb[3*i];fb.rgba[4*i+1]=rgb[3*i+1];fb.rgba[4*i+2]=rgb[3*i+2];fb.rgba[4*i+3]=255;}
 // Resolve diagnostic identities through owned-tree ancestry too; offsets
 // identify source records but never select their rendering stage.
 std::map<std::size_t,const RootSource*> nodeOwners;
 std::map<std::size_t,double> nodeBias;
 for(const auto& root:roots_){
   const auto own=[&](const auto& self,const SceneNode& n)->void{
     nodeOwners.emplace(n.sourceOffset,&root.source);
     nodeBias.emplace(n.sourceOffset,root.projectionBias);
     for(const auto& child:n.children)self(self,child);
   };own(own,root.node);
 }
 const auto describe=[&](std::size_t node,std::size_t source){
   return webo::app::context(levelId_,*nodeOwners.at(node),source);
 };
 const auto diagnose=[&](const std::string& message){
   stats.diagnostics.push_back(message);
   std::fprintf(stderr,"backdrop: %s\n",message.c_str());
 };
 std::map<std::string,SceneRgbaImage> textures;
 const auto getTexture=[&](const std::string& name,std::size_t node,std::size_t source)->const SceneRgbaImage*{
   if(name.empty()) return nullptr;
   auto it=textures.find(name);if(it!=textures.end())return &it->second;
   auto img=loadTexture(lib,"TEXTURES\\"+name+".BMP");if(!img){++stats.missingTextures;diagnose(describe(node,source)+"missing texture "+name+": "+img.error());return nullptr;}
   return &textures.emplace(name,rgba(*img)).first->second;
 };
 const SceneRgbaImage white{1,1,{255,255,255,255}};
  const auto context=predicateContext(camera);
  const auto meshProjectionScale=buildBillboardCameraChain(camera).projectionTransform[10];
  std::map<std::size_t,std::array<double,3>> eyes;
   std::map<std::size_t,std::vector<SceneMeshRecordSource>> secondaryMeshes;
   std::vector<SceneMeshRecordSource> primaryMeshes, legacyMeshes;
  const auto secondaryRoot=std::find_if(roots_.begin(),roots_.end(),[](const auto& n){return n.source.stage==Stage::Secondary;});
  for (const auto& owned : roots_) {
    const auto& root = owned.node;
    if (stage == Stage::Primary) ++stats.roots;
    // Primary retains the all-root census. Later passes need only their own
    // roots: they previously rebuilt and discarded the entire primary mesh.
    // Local facing predicates depend only on ancestors inside the owned root.
    if (stage != Stage::Primary && owned.source.stage != stage) continue;
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
      if (owned.source.stage != stage) continue;
      if (stage == Stage::Secondary) secondaryMeshes[rec.nodeSourceOffset].push_back(std::move(rec));
      else if (stage == Stage::Primary) primaryMeshes.push_back(std::move(rec));
      else legacyMeshes.push_back(std::move(rec));
    }
  }
   const auto drawMesh=[&](const SceneMeshRecordSource& rec){const bool observed=observer!=nullptr;if(observed&&observer->candidate)observer->candidate(rec.recordSourceOffset,rec.nodeSourceOffset,rec.blockSourceOffset,rec.corners.size()>2?rec.corners.size()-2:0);const SceneRgbaImage* tex=nullptr;const bool isNull=rec.materialName.empty();if(isNull){++stats.nullMaterials;tex=&white;}else tex=getTexture(rec.materialName,rec.nodeSourceOffset,rec.recordSourceOffset);if(!tex)return;
    if((rec.kind==4||rec.kind==5)&&!passesKind5(rec,eyes)){++stats.predicateRejected;return;}
    const auto state=options(false,rec.blendSelector==1);
    for(std::size_t k=1;k+1<rec.corners.size();++k){std::array<SceneRasterVertex,3> v{};bool valid=true;const std::size_t ids[3]={0,k,k+1};
    for(int j=0;j<3;++j){const auto& c=rec.corners[ids[j]];const auto p=c.viewPosition;if(!(p[2]>0)){valid=false;break;}auto a=c.attributes;
       auto adapted=adaptPackedSceneMeshVertex(camera.cx+camera.focalX*p[0]/p[2],camera.cy-camera.focalY*p[1]/p[2],p[2],meshProjectionScale,nodeBias.at(rec.nodeSourceOffset),{a[2],a[3],a[4],a[5]},a[0],a[1]);
      if(!adapted){diagnose(describe(rec.nodeSourceOffset,rec.recordSourceOffset)+"mesh vertex adapter: "+adapted.error());valid=false;break;}v[j]=*adapted;
    }
     if(valid){SceneRasterObserver tagged;if(observed){tagged=*observer;tagged.source=rec.recordSourceOffset;tagged.triangle=k-1;if(observer->accepted)tagged.accepted=[&](const SceneRasterWriteEvent& e){++stats.acceptedColorWrites;stats.acceptedDepthWrites+=e.depthAssigned;if(observer->accepted)observer->accepted(e);};}++stats.rasterCalls;auto r=rasterizeSceneTriangle(fb,depth,*tex,v,state,observed?&tagged:nullptr);if(r){++stats.meshTriangles;if(isNull)++stats.nullMaterialTriangles;}else {++stats.rejected;++stats.rasterRejected;diagnose(describe(rec.nodeSourceOffset,rec.recordSourceOffset)+"raster rejected: "+r.error());}}else {++stats.rejected;++stats.projectionRejected;}
   }
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
   // Branch-1 sprites are registered by the same collection walk (with their ancestor-composed
   // inputs) and realized at their Secondary dispatch slot: the first-draw frame, and so the
   // quad, can only be chosen when the node is actually drawn.
   std::map<std::size_t,PendingSprite> pendingSprites;
   collectBillboards(chain,[&](const RootSource& source,const SceneBillboardSource& b){if(source.stage!=stage)return;if(source.stage==Stage::Secondary)secondaryBillboards.emplace(b.sourceOffset,b);else if(source.stage==Stage::Primary)primaryBillboards.push_back(b);else legacyBillboards.push_back(b);},[&](const std::string& message){stats.diagnostics.push_back(message);std::fprintf(stderr,"backdrop: %s\n",message.c_str());},
     [&](const RootSource& source,const SceneNode& n,const BillboardSourceInputs& in){if(source.stage==Stage::Secondary)pendingSprites.emplace(n.sourceOffset,PendingSprite{&n,in});});
   std::map<std::string,SceneRgbaImage> spriteTextures;
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
    if(stage!=Stage::Primary&&observer&&observer->candidate)observer->candidate(n.sourceOffset,n.sourceOffset,UINT64_MAX,2);
    const std::string asset=sheetAssetName(image.sheet);
    auto cached=spriteTextures.find(asset);
    if(cached==spriteTextures.end()){
      auto img=loadTexture(lib,asset);
      if(!img){++stats.missingTextures;sub.reason="missing texture "+asset;diagnose(describe(n.sourceOffset,n.sourceOffset)+"missing texture "+asset+": "+img.error());finish();return;}
      cached=spriteTextures.emplace(asset,rgba(*img)).first;
    }
    ++stats.sprites;
    const auto state=spriteOptions(quad->blendSelector);
    std::uint64_t bi=0;
    for(const auto& tri:{std::array<int,3>{0,3,2},std::array<int,3>{0,2,1}}){
      std::array<SceneRasterVertex,3> vv{};bool ok=true;
      for(int j=0;j<3;++j){const auto& q=quad->vertices[tri[j]];if(!(q.z>0)){ok=false;break;}vv[j]=billboardRasterVertex(q,builtChain,{255,255,255,255});}
      if(!ok){++stats.rejected;++stats.projectionRejected;++sub.culled;if(sub.reason.empty())sub.reason="culled: non-positive depth";++bi;continue;}
      SceneRasterObserver tagged;
      if(observer){tagged=*observer;tagged.source=n.sourceOffset;tagged.triangle=bi;
        tagged.accepted=[&](const SceneRasterWriteEvent& e){++stats.acceptedColorWrites;stats.acceptedDepthWrites+=e.depthAssigned;++stats.spriteAcceptedColorWrites;stats.spriteAcceptedDepthWrites+=e.depthAssigned;if(observer->accepted)observer->accepted(e);};}
      ++stats.rasterCalls;++stats.spriteRasterCalls;++sub.rasterCalls;
      auto r=rasterizeSceneTriangle(fb,depth,cached->second,vv,state,observer?&tagged:nullptr);
      if(r){++stats.meshTriangles;++stats.spriteTriangles;}
      else{++stats.rejected;++stats.rasterRejected;++sub.rejected;if(sub.reason.empty())sub.reason="raster rejected: "+r.error();diagnose(describe(n.sourceOffset,n.sourceOffset)+"raster rejected: "+r.error());}
      ++bi;
    }
    finish();
   };
   if(stage==Stage::Primary&&observer&&observer->candidate)for(const auto& b:primaryBillboards)observer->candidate(b.sourceOffset,b.sourceOffset,UINT64_MAX,2);
   const auto drawBillboard=[&](const SceneBillboardSource& b){if(stage!=Stage::Primary&&observer&&observer->candidate)observer->candidate(b.sourceOffset,b.sourceOffset,UINT64_MAX,2);const std::string name=b.textureName;const SceneRgbaImage* tex=name.empty()?&white:getTexture(name,b.sourceOffset,b.sourceOffset);if(!tex){++stats.nullMaterials;return;}++stats.billboards;
    // C3r's accepted normal-constructor census identifies all 17 retained type-2
    // resources as caret leaves => flags 8. Apply that conditional row here;
    // this is not a claim about cache hits or effective native device state.
    const auto st=conditionalCaretFlags8Policy(options(true,b.blendSelector==1));
      std::uint64_t bi=0;for(const auto& tri:{std::array<int,3>{0,3,2},std::array<int,3>{0,2,1}}){std::array<SceneRasterVertex,3> vv{};bool ok=true;for(int j=0;j<3;++j){auto q=b.slots[tri[j]];if(!(q.z>0)){ok=false;break;}vv[j]=billboardRasterVertex(q,builtChain,{255,255,255,255});}if(ok){SceneRasterObserver tagged;if(observer){tagged=*observer;tagged.source=b.sourceOffset;tagged.triangle=bi;if(observer->accepted)tagged.accepted=[&](const SceneRasterWriteEvent& e){++stats.acceptedColorWrites;stats.acceptedDepthWrites+=e.depthAssigned;if(observer->accepted)observer->accepted(e);};}++stats.rasterCalls;auto r=rasterizeSceneTriangle(fb,depth,*tex,vv,st,observer?&tagged:nullptr);if(r)++stats.meshTriangles;else {++stats.rejected;++stats.rasterRejected;diagnose(describe(b.sourceOffset,b.sourceOffset)+"raster rejected: "+r.error());}}else {++stats.rejected;++stats.projectionRejected;}++bi;}};
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
 for(std::size_t i=0,n=rgb.size()/3;i<n;++i){rgb[3*i]=fb.rgba[4*i];rgb[3*i+1]=fb.rgba[4*i+1];rgb[3*i+2]=fb.rgba[4*i+2];}
  for(std::size_t i=0;i<depth.size();++i)if(depth[i]!=initialDepth[i])++stats.pixels;
}
void BackdropScene::collectBillboards(const BillboardSourceInputs& chain,
    const std::function<void(const RootSource&,const SceneBillboardSource&)>& visitor,
    const std::function<void(const std::string&)>& error,
    const SpriteVisitor& sprite) const {
 for(const auto& owned:roots_){
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
 // 0x00437BDE: scene clock += dt (no scene-duration wrap is parsed, a documented limit);
 // 0x004764C4: node delta = t - previous, t itself after a negative delta.
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
    for(const auto& rec:*records)if(rec.kind==5&&!rec.corners.empty())visitor(rec.recordSourceOffset,passesKind5(rec,eyes));}
}
void BackdropScene::visitSecondaryDispatch(const CameraBasis& camera,const std::function<void(const SecondaryDispatchItem&)>& visitor) const {
 const auto root=std::find_if(roots_.begin(),roots_.end(),[](const auto& n){return n.source.stage==Stage::Secondary;});if(root==roots_.end())return;
 const auto chain=buildBillboardCameraChain(camera);dispatchSecondaryChildren(root->node,nativeAffine(BillboardAffine3x4{chain.parentCamera}),nativeAffine(BillboardAffine3x4{chain.sourceTransform}),visitor,dispatchOptions());
}
// Production dispatch gates (0x46C56B/0x46C5C4/0x47D0D9). The runtime "cutme" frame-time LOD (0x45DA0D)
// is not connected: unless a caller installs setSecondaryHidden, those type-1 nodes stay visible, a
// recorded limit (no admitted target level has one).
SecondaryDispatchOptions BackdropScene::dispatchOptions() const {SecondaryDispatchOptions o;o.originalGates=true;o.hidden=secondaryHidden_;return o;}
void BackdropScene::visitRoots(const std::function<void(const RootSource&,const SceneNode&)>& visitor) const {
 for(const auto& root:roots_)visitor(root.source,root.node);
}
}
