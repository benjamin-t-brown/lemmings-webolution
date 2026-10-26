#include "app/BackdropScene.h"
#include "app/Camera.h"
#include "app/Raster.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>

int main(){
 int checks=0,fail=0;auto check=[&](bool ok,const char* s){++checks;if(!ok){++fail;std::cerr<<"FAIL "<<s<<"\n";}};
 auto libdir=std::getenv("WEBOLUTION_GAME_DIR");if(!libdir){std::cout<<"SKIP: WEBOLUTION_GAME_DIR unset\n";return 0;}
 auto lib=webo::AssetLibrary::open(std::filesystem::path(libdir)/"lemmings.box",std::filesystem::path(libdir)/"patch.box");check(bool(lib),"open retail archive");if(!lib)return 1;
 std::string level;for(auto& n:webo::levelFileNames(*lib))if(n.find("MM_CLIMB_002")!=std::string::npos){level=n;break;}check(!level.empty(),"find supported level");if(level.empty())return 1;
 auto bytes=lib->read(level);check(bool(bytes),"read supported level");if(!bytes)return 1;
 auto scene=webo::app::BackdropScene::build(*lib,*bytes,"MM_CLIMB_002");check(bool(scene),"parse guarded roots");if(!scene)return 1;
 auto unsupported=webo::app::BackdropScene::build(*lib,*bytes,"MM_DIG_000");check(bool(unsupported)&&unsupported->empty(),"unsupported level is clean empty scene");
 webo::app::CameraParams cp;cp.width=640;cp.height=480;cp.aspect=4.0/3.0;auto cam=webo::app::buildCamera(cp);webo::app::Raster raster(640,480);raster.clear(0,0,0);webo::app::BackdropStats stats;scene->render(raster,cam,*lib,stats);
 std::size_t nonblack=0;for(std::size_t i=0;i<raster.framebuffer().rgb.size();i+=3)if(raster.framebuffer().rgb[i]||raster.framebuffer().rgb[i+1]||raster.framebuffer().rgb[i+2])++nonblack;
  check(stats.roots==4,"four retained roots");check(stats.meshRecords==1177,"source records preserved");check(stats.nullMaterials==304,"known null materials counted");check(stats.nullMaterialTriangles>0,"known null records draw with source diffuse");check(stats.billboards==17,"17 production billboards submitted once");check(stats.predicateRejected>0,"kind5 source predicate rejects records");check(nonblack>0,"scene-on changes pixels");
  // Inject accepted raw camera operands (camera affine + scale vectors), then
  // exercise the same production camera-chain builder used by render.
  auto measured=cam;
  const double rawAffine[12]={-.7071066498756409,0,-.70710688829422,.1384189873933792,.9806530475616455,-.13841894268989563,.6934265494346619,-.19575399160385132,-.6934263110160828,-266.94415283203125,113.39199829101562,266.9440612792969};
  for(int r=0;r<3;++r)for(int c=0;c<3;++c)measured.worldToView[3*r+c]=rawAffine[3*c+r];
  auto r32=[](double x){return static_cast<double>(static_cast<float>(x));};
  for(int c=0;c<3;++c){double a=r32(-rawAffine[9]*measured.worldToView[c]),b=r32(-rawAffine[10]*measured.worldToView[3+c]),d=r32(-rawAffine[11]*measured.worldToView[6+c]);measured.worldToView[9+c]=r32(r32(a+b)+d);}
  measured.projectionVector={2.2373132705688477,2.2373132705688477,.0015625000232830644};
  measured.inverseProjectionVector={.44696468114852905,.44696468114852905,640.0};
  const auto accepted=webo::app::buildBillboardCameraChain(measured);
  webo::app::BillboardSourceInputs fixture;fixture.parentCamera.values=accepted.parentCamera;fixture.sourceTransform.values=accepted.sourceTransform;fixture.projectionTransform.values=accepted.projectionTransform;fixture.center={319,239};
  struct Expected{std::size_t offset;double x0,y0,z,x2,y2;};
  constexpr Expected expected[]={
   {0x967a2,127.37244539895102,-7.5552263641560842,.3698773301636632,202.73000234921099,67.802330586103864},
   {0x96892,862.70247254781691,10.966628623825756,.38952624715678619,934.258759642379,82.52291571838785},
   {0x96980,420.94764617361795,-7.2425655210323043,.41622268001780843,487.91432845826847,59.724116763618241},
   {0x96a70,706.19933409166538,-15.357001908385769,.3759984189587533,780.33010317736193,58.773767177310845},
   {0x96b60,1115.3076558822909,22.433535311223892,.38781761183993063,1187.1792035414589,94.305082970392021},
   {0x96c4e,655.10880950056401,-4.3251483449317902,.47505202660794832,713.78249240523974,54.348534559743996},
   {0x96d3e,903.12886036036309,3.6443655675308548,.32643976939627428,988.51383665772869,89.029341864896395},
   {0x96e2c,1078.4305169480206,13.601623687498204,.30710859537286272,1169.1901127143894,104.36121945386702},
   {0x96f1a,319.67562286483309,-3.4486737934319649,.37981904632779889,393.06070516100317,69.936408502738118},
   {0x9700a,555.36774497109297,21.840463747516822,.35489033321959595,633.90764691643926,100.38036569286314},
   {0x970f8,556.36829349626908,-1.6516393458855134,.45721606339017906,617.33083222630501,59.310899384150417},
   {0x971e6,724.73276754338394,47.399796090019294,.33254099580413038,808.55116083657003,131.21818938320541},
   {0x9889b,2715.9668866267125,227.06857688166409,.2029905804129879,2893.3370378390682,404.43865536853832},
   {0x98989,3309.3078106140065,233.73034744087417,.18756840518309889,3507.011399393281,431.43385866095969},
   {0x98a77,2912.8623954078857,271.78073605089281,.16287490203768185,3125.9726223435209,484.89088361065069},
   {0x98b65,4065.3013900066812,288.58874348591513,.13677256317581699,4335.8233906724345,559.11063195681481},
   {0x98c53,5511.5211241090419,324.44743809550602,.092126823060699095,5869.0170500969925,681.94321557742774}};
  std::size_t tuples=0,fields=0,matched=0;double maxError=0;bool first=false;
  scene->visitBillboards(fixture,[&](const webo::app::SceneBillboardSource& b){++tuples;std::cout<<std::setprecision(17)<<"BILLBOARD,"<<std::hex<<b.sourceOffset<<std::dec;for(const auto& v:b.slots){for(double x:{v.x,v.y,v.z,v.u,v.v}){++fields;std::cout<<","<<x;} }std::cout<<"\n";for(const auto& e:expected)if(e.offset==b.sourceOffset){++matched;const double want[20]={e.x0,e.y0,e.z,0,0,e.x0,e.y2,e.z,0,1,e.x2,e.y2,e.z,1,1,e.x2,e.y0,e.z,1,0};int k=0;for(const auto& v:b.slots)for(double x:{v.x,v.y,v.z,v.u,v.v})maxError=std::max(maxError,std::abs(x-want[k++]));if(b.sourceOffset==0x967a2)first=true;}});
  check(tuples==17&&fields==340&&matched==17,"production tree walk emits all 17/340 C3o slot fields");check(first&&maxError<=1e-9,"production builder reproduces every retained C3o field within 1e-9");
  auto omitted=fixture;omitted.sourceTransform.values={1,0,0,0,0,1,0,0,0,0,1,0};bool mutationDiff=false;scene->visitBillboards(omitted,[&](const webo::app::SceneBillboardSource& b){if(b.sourceOffset==0x967a2)mutationDiff=std::abs(b.slots[0].x-127.37244539895102)>1e-9;});check(mutationDiff,"omitting S alone fails retained tuple regression");
  auto noCamera=fixture;noCamera.parentCamera.values={1,0,0,0,0,1,0,0,0,0,1,0};bool ancestorDiff=false;scene->visitBillboards(noCamera,[&](const webo::app::SceneBillboardSource& b){if(b.sourceOffset==0x967a2)ancestorDiff=std::abs(b.slots[0].x-127.37244539895102)>1e-9;});check(ancestorDiff,"omitting camera/ancestor chain alone fails retained tuple regression");
 std::cout<<checks<<" checks, "<<fail<<" failures; records="<<stats.meshRecords<<" null="<<stats.nullMaterials<<" null-triangles="<<stats.nullMaterialTriangles<<" billboards="<<stats.billboards<<" changed="<<nonblack<<"\n";return fail?1:0;
}
