#include "app/BackdropScene.h"
#include "app/Camera.h"
#include "app/Raster.h"
#include "app/SceneFrame.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <vector>
#include <bit>

int main(){
 int checks=0,fail=0;auto check=[&](bool ok,const char* s){++checks;if(!ok){++fail;std::cerr<<"FAIL "<<s<<"\n";}};
  std::vector<std::string> scheduled;
  webo::app::scheduleSceneFrame([&]{scheduled.push_back("primary");},[&]{scheduled.push_back("legacy-unattributed");},[&]{scheduled.push_back("tube");},[&]{scheduled.push_back("secondary");});
  check(scheduled==std::vector<std::string>({"primary","legacy-unattributed","tube","secondary"}),"production frame scheduler orders primary, explicitly unattributed legacy, tube, secondary exactly once");
  scheduled.clear();
  webo::app::scheduleSceneFrame([&]{scheduled.push_back("primary");},[&]{scheduled.push_back("legacy-unattributed");},[&]{scheduled.push_back("secondary");},[&]{scheduled.push_back("tube");});
  check(scheduled!=std::vector<std::string>({"primary","legacy-unattributed","tube","secondary"}),"secondary-before-tube isolated scheduler mutation fails ordering oracle");
  auto libdir=std::getenv("WEBOLUTION_GAME_DIR");if(!libdir){std::cout<<"SKIP: WEBOLUTION_GAME_DIR unset\n";return 0;}
 auto lib=webo::AssetLibrary::open(std::filesystem::path(libdir)/"lemmings.box",std::filesystem::path(libdir)/"patch.box");check(bool(lib),"open retail archive");if(!lib)return 1;
  std::string level;for(auto& n:webo::levelFileNames(*lib))if(n.find("MM_CLIMB_002")!=std::string::npos){level=n;break;}check(!level.empty(),"find supported level");if(level.empty())return 1;
 auto bytes=lib->read(level);check(bool(bytes),"read supported level");if(!bytes)return 1;
 auto scene=webo::app::BackdropScene::build(*lib,*bytes,"MM_CLIMB_002");check(bool(scene),"parse guarded roots");if(!scene)return 1;
 auto unsupported=webo::app::BackdropScene::build(*lib,*bytes,"MM_DIG_000");check(bool(unsupported)&&unsupported->empty(),"unsupported level is clean empty scene");
  webo::app::CameraParams cp;cp.width=640;cp.height=480;cp.aspect=4.0/3.0;auto cam=webo::app::buildCamera(cp);webo::app::Raster raster(640,480);raster.clear(0,0,0);webo::app::BackdropStats stats;scene->render(raster,cam,*lib,stats);
   { webo::app::Raster packedRaster(640,480);packedRaster.clear(0,0,0);webo::app::BackdropStats packedStats;scene->renderStage(packedRaster,cam,*lib,packedStats,webo::app::BackdropScene::Stage::Primary);std::uint64_t h=1469598103934665603ull;auto add=[&](std::uint8_t b){h=(h^b)*1099511628211ull;};for(auto b:packedRaster.framebuffer().rgb)add(b);for(double d:packedRaster.mutableDepthBuffer()){auto bits=std::bit_cast<std::uint64_t>(d);for(int i=0;i<8;++i)add(static_cast<std::uint8_t>(bits>>(8*i)));}check(h==0x1fd0bfb82ab39e95ull,"actual BackdropScene primary packed RGB+depth output matches deterministic production fixture"); }
  std::size_t nonblack=0;for(std::size_t i=0;i<raster.framebuffer().rgb.size();i+=3)if(raster.framebuffer().rgb[i]||raster.framebuffer().rgb[i+1]||raster.framebuffer().rgb[i+2])++nonblack;
   webo::app::Raster censusRaster(640,480);censusRaster.clear(0,0,0);webo::app::BackdropStats primaryCensus,legacyCensus,secondaryCensus;
   scene->renderStage(censusRaster,cam,*lib,primaryCensus,webo::app::BackdropScene::Stage::Primary);
   scene->renderStage(censusRaster,cam,*lib,legacyCensus,webo::app::BackdropScene::Stage::LegacyUnattributed);
   scene->renderStage(censusRaster,cam,*lib,secondaryCensus,webo::app::BackdropScene::Stage::Secondary);
   std::cout<<"STAGE_CENSUS primary="<<primaryCensus.stageMeshRecords<<"/"<<primaryCensus.stageBillboards<<" legacy="<<legacyCensus.stageMeshRecords<<"/"<<legacyCensus.stageBillboards<<" secondary="<<secondaryCensus.stageMeshRecords<<"/"<<secondaryCensus.stageBillboards<<"\n";
   check(primaryCensus.stageMeshRecords+legacyCensus.stageMeshRecords+secondaryCensus.stageMeshRecords==1177&&primaryCensus.stageBillboards+legacyCensus.stageBillboards+secondaryCensus.stageBillboards==17,"actual production stage renders census every retained mesh and billboard contribution once across primary, legacy, secondary");
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
  // C3p's accepted retained-camera context; compare each production record in
  // source order, not merely the aggregate skip count.
  constexpr char decisions[]="111111111111010000000011111000111111111111111110001010111011011110101010111111111111111111111111111111111111111111100001111111100001111011111111111111111101111111011111001110010110001111001111111111111111111111111110011111000111111110000000011111111000000000000000011111111000000001111111111111111111111111111111110101010101010101100110000111100110000111100110000111100110000111100111100001100110011000011110011000011110011000011110011000011110000000000110011110011";
  std::size_t predCount=0,predPass=0,lastOffset=0;bool predMatch=true;
  scene->visitKind5Predicates(measured,[&](std::size_t offset,bool pass){std::cout<<"PRED,"<<std::hex<<offset<<std::dec<<","<<(pass?"pass":"skip")<<"\n";if(predCount>=sizeof(decisions)-1||pass!=(decisions[predCount]=='1')||(predCount&&offset<=lastOffset))predMatch=false;++predCount;predPass+=pass;lastOffset=offset;});
  check(predCount==465&&predPass==306&&predMatch,"camera-composed production predicate matches every C3p source decision");
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
   constexpr std::size_t mixedExpected[]={0x923fa,0x91805,0x94650,0x92fbf,0x940ea,0x93b83,0x951e5,0x9574c,0x95cb3,0x96c4e,0x970f8,0x96980,0x96892,0x96b60,0x96f1a,0x96a70,0x967a2,0x9700a,0x971e6,0x96d3e,0x96e2c,0x9889b,0x98989,0x98334,0x98106,0x972d6,0x98a77,0x98b65,0x98c53};
   constexpr double mixedKeys[]={25905.176536021387,22502.353192521823,22431.703384107284,21610.095133418126,5109.523792355868,5024.711206558943,4263.397375480879,3750.857037674893,1524.3676625630542,1034.208066614527,996.1580132619259,908.7054655425873,851.7530777021577,848.107989171042,831.044383423042,822.8937120264558,809.8353897824879,777.863130905595,730.1845463138582,717.1685971609725,675.928760883085,459.86776566749995,427.7705283118633,416.551120658287,392.89448818299644,392.89439406051355,373.3235802169622,319.3442697832317,221.07182469267002};
   std::size_t dispatchCount=0;bool dispatchMatch=true;double prior=1e300;
   scene->visitSecondaryDispatch(measured,[&](const webo::app::SecondaryDispatchItem& item){if(dispatchCount>=std::size(mixedExpected)||item.nodeSourceOffset!=mixedExpected[dispatchCount]||!(item.key<prior)||!std::isfinite(item.key)||(dispatchCount<std::size(mixedKeys)&&std::abs(item.key-mixedKeys[dispatchCount])>1e-5))dispatchMatch=false;prior=item.key;++dispatchCount;});
   check(dispatchCount==std::size(mixedExpected)&&dispatchMatch,"actual retained secondary dispatch matches independent C3y source-offset sequence and every reference key");
   auto parsedRoot=webo::readSceneNode(*bytes,0x91782);check(bool(parsedRoot),"parse secondary source tree for isolated bias mutation");
   if(parsedRoot){auto row=[](const std::array<double,12>& a){return std::array<double,12>{a[0],a[4],a[8],a[1],a[5],a[9],a[2],a[6],a[10],a[3],a[7],a[11]};};const auto parent=row(accepted.parentCamera),rotation=row(accepted.sourceTransform);
    std::vector<std::size_t> baseOrder,mutantOrder,restoredOrder;webo::app::dispatchSecondaryChildren(parsedRoot->root,parent,rotation,[&](const auto& x){baseOrder.push_back(x.nodeSourceOffset);});
    auto changed=std::find_if(parsedRoot->root.children.begin(),parsedRoot->root.children.end(),[](const auto& n){return n.sourceOffset==0x972d6;});if(changed!=parsedRoot->root.children.end()){const double saved=changed->headDoubles[1];changed->headDoubles[1]=50000;webo::app::dispatchSecondaryChildren(parsedRoot->root,parent,rotation,[&](const auto& x){mutantOrder.push_back(x.nodeSourceOffset);});changed->headDoubles[1]=saved;webo::app::dispatchSecondaryChildren(parsedRoot->root,parent,rotation,[&](const auto& x){restoredOrder.push_back(x.nodeSourceOffset);});}
    check(baseOrder.size()==29&&mutantOrder!=baseOrder&&restoredOrder==baseOrder,"isolated source +108 bias mutation changes shared production dispatch order and restores exactly");
   }
 std::cout<<checks<<" checks, "<<fail<<" failures; records="<<stats.meshRecords<<" null="<<stats.nullMaterials<<" null-triangles="<<stats.nullMaterialTriangles<<" billboards="<<stats.billboards<<" changed="<<nonblack<<"\n";return fail?1:0;
}
