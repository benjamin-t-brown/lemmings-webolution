#include "app/BackdropScene.h"
#include "app/Raster.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneMeshSource.h"
#include "app/SceneMaterialFlags.h"
#include "app/SceneDepth.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include <algorithm>
#include <array>
#include <map>
#include <cmath>

namespace webo::app {
namespace {
constexpr std::array<std::size_t,4> roots={0x59644,0x91782,0x9ccd1,0xa0cdd};
constexpr std::array<std::size_t,4> ends={0x9176e,0x98d41,0x9db2b,0xaaa43};
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
bool passesKind5(const SceneMeshRecordSource& rec,const std::map<std::size_t,std::array<double,3>>& eyes){if(rec.kind!=5||rec.corners.empty())return true;const auto it=eyes.find(rec.nodeSourceOffset);if(it==eyes.end())return false;const auto&e=it->second;const auto&v=rec.corners.front().localPosition;const auto&n=rec.corners.front().localNormal;const double facing=(e[0]-v[0])*n[0]+(e[1]-v[1])*n[1]+(e[2]-v[2])*n[2];return facing>0;}
SceneRasterOptions options(bool alpha,bool blend,bool depthWrite=true){return {SceneFilter::Linear,SceneAddress::Wrap,alpha?SceneAlphaFunction::StrictGreater:SceneAlphaFunction::Disabled,0xef,blend,SceneBlendFactor::SrcAlpha,SceneBlendFactor::InvSrcAlpha,SceneDepthFunction::Less,depthWrite,SceneShade::Gouraud,SceneRgbOperation::Modulate,SceneAlphaOperation::Modulate,0.5,0.5};}
SceneRasterVertex vertex(double x,double y,double z,double rhw,double u,double v,std::array<double,4> color){return {x,y,z,rhw,color,u,v};}
SceneRgbaImage rgba(const Image& im){return {im.width,im.height,im.rgba};}
}
std::expected<BackdropScene,std::string> BackdropScene::build(const AssetLibrary&,std::span<const std::byte> bytes,std::string_view levelId){
 BackdropScene s;
 // Retained offsets are accepted only for the level whose file layout was checked.
 if(levelId!="MM_CLIMB_002") return s;
 if(bytes.size()<0xaaa43) return std::unexpected("MM_CLIMB_002 retained scene extent truncated");
 for(std::size_t i=0;i<roots.size();++i){auto n=readSceneNode(bytes,roots[i]);if(!n)return std::unexpected("retained root unsupported: "+n.error());if(n->endOffset!=ends[i])return std::unexpected("retained root end/hash guard mismatch");s.roots_.push_back(std::move(n->root));}
 return s;
}
void BackdropScene::render(Raster& target,const CameraBasis& camera,const AssetLibrary& lib,BackdropStats& stats) const {
 // SceneRaster and legacy Raster share the same linear view-depth units here;
 // the choice is conditional software integration, not an effective-D3D claim.
 SceneRgbaFramebuffer fb; fb.width=camera.width;fb.height=camera.height;fb.rgba.resize(target.framebuffer().rgb.size()/3*4);
 auto& rgb=target.mutableFramebuffer().rgb;auto& depth=target.mutableDepthBuffer();
 const auto initialDepth=depth;
 for(std::size_t i=0,n=rgb.size()/3;i<n;++i){fb.rgba[4*i]=rgb[3*i];fb.rgba[4*i+1]=rgb[3*i+1];fb.rgba[4*i+2]=rgb[3*i+2];fb.rgba[4*i+3]=255;}
 std::map<std::string,SceneRgbaImage> textures;
 const auto getTexture=[&](const std::string& name)->const SceneRgbaImage*{
   if(name.empty()) return nullptr;
   auto it=textures.find(name);if(it!=textures.end())return &it->second;
   auto img=loadTexture(lib,"TEXTURES\\"+name+".BMP");if(!img){++stats.missingTextures;return nullptr;}
   return &textures.emplace(name,rgba(*img)).first->second;
 };
 const SceneRgbaImage white{1,1,{255,255,255,255}};
 const auto context=predicateContext(camera);
  std::map<std::size_t,std::array<double,3>> eyes;
 for(const auto& root:roots_){collectEyes(root,identity(),context,eyes);++stats.roots;auto records=buildSceneMeshSource(root,identity(),camera.worldToView);if(!records)continue;
  for(const auto& rec:*records){++stats.meshRecords;const SceneRgbaImage* tex=nullptr;const bool isNull=rec.materialName.empty();if(isNull){++stats.nullMaterials;tex=&white;}else tex=getTexture(rec.materialName);if(!tex)continue;
   if(rec.kind==5&&!passesKind5(rec,eyes)){++stats.predicateRejected;continue;}
   const auto state=options(false,rec.blendSelector==1);
   for(std::size_t k=1;k+1<rec.corners.size();++k){std::array<SceneRasterVertex,3> v{};bool valid=true;const std::size_t ids[3]={0,k,k+1};
    for(int j=0;j<3;++j){const auto& c=rec.corners[ids[j]];const auto p=c.viewPosition;if(!(p[2]>0)){valid=false;break;}auto a=c.attributes;
     // Source channels are passed without clamping; modulo conversion models the declared byte packing candidate.
     std::array<double,4> d{};for(int q=0;q<4;++q){double x=a[q+2];if(x>=0&&x<=1)x*=255;d[q]=static_cast<unsigned char>(static_cast<long long>(x));}
     v[j]=vertex(camera.cx+camera.focalX*p[0]/p[2],camera.cy-camera.focalY*p[1]/p[2],p[2],0.25/p[2],a[0],a[1],d);
    }
    if(valid){auto r=rasterizeSceneTriangle(fb,depth,*tex,v,state);if(r){++stats.meshTriangles;if(isNull)++stats.nullMaterialTriangles;}else ++stats.rejected;}else ++stats.rejected;
  }
  }
  } // Finish all root mesh passes before the one scene-wide billboard traversal.
  const auto builtChain=buildBillboardCameraChain(camera);
  BillboardSourceInputs chain;chain.parentCamera.values=builtChain.parentCamera;
  chain.sourceTransform.values=builtChain.sourceTransform;
  chain.projectionTransform.values=builtChain.projectionTransform;chain.center={camera.cx,camera.cy};
  visitBillboards(chain,[&](const SceneBillboardSource& b){const std::string name=b.textureName;const SceneRgbaImage* tex=name.empty()?&white:getTexture(name);if(!tex){++stats.nullMaterials;return;}++stats.billboards;
    // C3r's accepted normal-constructor census identifies all 17 retained type-2
    // resources as caret leaves => flags 8. Apply that conditional row here;
    // this is not a claim about cache hits or effective native device state.
    const auto st=conditionalCaretFlags8Policy(options(true,b.blendSelector==1));
     for(const auto& tri:{std::array<int,3>{0,3,2},std::array<int,3>{0,2,1}}){std::array<SceneRasterVertex,3> vv{};bool ok=true;for(int j=0;j<3;++j){auto q=b.slots[tri[j]];if(!(q.z>0)){ok=false;break;}vv[j]=billboardRasterVertex(q,builtChain,{255,255,255,255});}if(ok){auto r=rasterizeSceneTriangle(fb,depth,*tex,vv,st);if(r)++stats.meshTriangles;else ++stats.rejected;}else ++stats.rejected;}});
 for(std::size_t i=0,n=rgb.size()/3;i<n;++i){rgb[3*i]=fb.rgba[4*i];rgb[3*i+1]=fb.rgba[4*i+1];rgb[3*i+2]=fb.rgba[4*i+2];}
  for(std::size_t i=0;i<depth.size();++i)if(depth[i]!=initialDepth[i])++stats.pixels;
}
 void BackdropScene::visitBillboards(const BillboardSourceInputs& chain,const std::function<void(const SceneBillboardSource&)>& visitor) const {
 const auto walk=[&](const auto& self,const SceneNode& n,const Matrix& ancestors)->void{
  if(n.type==2){auto in=chain;in.parentCamera=multiply(chain.parentCamera,affine(ancestors));auto b=makeSceneBillboardSource(n,in);if(b)visitor(*b);}
  const Matrix next=compose(n.matrix,ancestors);for(const auto& child:n.children)self(self,child,next);
 };
 for(const auto& root:roots_)walk(walk,root,identity());
 }
void BackdropScene::visitKind5Predicates(const CameraBasis& camera,const std::function<void(std::size_t,bool)>& visitor) const {
  const auto context=predicateContext(camera);std::map<std::size_t,std::array<double,3>> eyes;
  for(const auto& root:roots_)collectEyes(root,identity(),context,eyes);
  for(const auto& root:roots_){auto records=buildSceneMeshSource(root,identity(),camera.worldToView);if(!records)continue;for(const auto& rec:*records)if(rec.kind==5&&!rec.corners.empty())visitor(rec.recordSourceOffset,passesKind5(rec,eyes));}
}
}
