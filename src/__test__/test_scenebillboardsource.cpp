#include "app/SceneBillboardSource.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <map>
#include <string_view>

namespace {
int checks = 0, failures = 0;
void check(bool ok, std::string_view label) {
  ++checks;
  if (!ok) { ++failures; std::cerr << "FAIL " << label << '\n'; }
}
bool near(double a, double b) { return std::abs(a - b) < 1e-11; }
webo::app::BillboardAffine3x4 identity() {
  return {{{1,0,0,0, 0,1,0,0, 0,0,1,0}}};
}
webo::SceneNode fixture() {
  webo::SceneNode n;
  n.type = 2; n.sourceOffset = 0xabc; n.name = "source-node";
  n.matrix = {1,0,0, 0,1,0, 0,0,1, 2,3,4};
  n.type2.branch = 0; n.type2.visibility = 1; n.type2.branchName = "texture/example";
  n.type2.leading = {4, 8, 6}; // unequal side widths and height
  n.type2.branchDoubles = {0.13, 0.27, 0.61, 0.89};
  n.type2.trailingWord = 0;
  return n;
}
void geometry() {
  auto n = fixture();
  webo::app::BillboardSourceInputs in;
  // Noncommuting affine inputs: translation + shear/scale in caller-selected order.
  in.parentCamera.values = {2,1,0,5, 0,3,1,-2, 1,0,2,7};
  in.sourceTransform.values = {1,0,1,11, 0,1,0,13, 0,0,1,17};
  in.projectionTransform.values = {1,2,0,0, -1,1,0,0, .5,.25,1,20};
  in.center = {320,240};
  auto got = webo::app::makeSceneBillboardSource(n, in);
  check(bool(got), "asymmetric geometry accepted");
  if (!got) return;
  check(got->sourceOffset == 0xabc && got->sourceName == "source-node" && got->textureName == "texture/example" &&
        got->blendSelector == 0 && got->sourceVisibility == 1, "identity and source fields retained");
  check(got->adapterOrder == std::array<unsigned,4>{0,3,2,1}, "adapter order 0,3,2,1");
  // node origin -> parent/camera -> S: (2,3,4)->(12,11,17)->(40,24,34)
  // P is the explicit projection applied after generated source-space offsets.
  const std::array<std::array<double,3>,4> expected{{
    {38,21,32}, {36,27,30}, {44,27,30}, {42,21,32}}};
  for (std::size_t i=0;i<4;++i)
    for (std::size_t j=0;j<3;++j)
      check(near(got->generatedPoints[i][j], expected[i][j]), "generated point affine composition");

  // Explicitly computed projection results; unequal depths expose reciprocal pairing.
  const std::array<std::array<double,3>,4> projected{{
    {80,-17,76.25}, {90,-9,74.75}, {98,-17,78.75}, {84,-21,78.25}}};
  check(near(got->slots[0].x, 320 + projected[0][0]/projected[0][2]), "slot0 X uses P0/P0.z");
  check(near(got->slots[1].x, 320 + projected[1][0]/projected[1][2]), "slot1 X uses P1/P1.z");
  check(near(got->slots[2].x, 320 + projected[2][0]/projected[1][2]), "slot2 X shared P1.z");
  check(near(got->slots[3].x, 320 + projected[3][0]/projected[0][2]), "slot3 X shared P0.z");
  check(near(got->slots[0].y, 240 - projected[1][1]/projected[1][2]), "slot0 crossed Y source P1");
  check(near(got->slots[1].y, 240 - projected[0][1]/projected[0][2]), "slot1 crossed Y source P0");
  check(near(got->slots[2].y, 240 - projected[3][1]/projected[0][2]), "slot2 crossed Y source P3");
  check(near(got->slots[3].y, 240 - projected[2][1]/projected[1][2]), "slot3 crossed Y source P2");
  const double uv[4][2]={{.13,.89},{.13,.61},{.27,.61},{.27,.89}};
  for (int i=0;i<4;++i) check(near(got->slots[i].u,uv[i][0]) && near(got->slots[i].v,uv[i][1]), "native UV tuple");
  check(got->slots[0].z == projected[0][2] && got->slots[1].z == projected[1][2] &&
        got->slots[2].z == projected[2][2] && got->slots[3].z == projected[3][2],
        "each slot retains own P.z");
}
void invalid() {
  auto n=fixture(); webo::app::BillboardSourceInputs in;
  in.parentCamera=identity(); in.sourceTransform=identity(); in.projectionTransform=identity();
  in.projectionTransform.values[11]=20;
  check(bool(webo::app::makeSceneBillboardSource(n,in)), "identity transforms accepted");
  n.type=1; check(!webo::app::makeSceneBillboardSource(n,in), "non-type2 rejected");
  n=fixture(); n.type2.branch=1; check(!webo::app::makeSceneBillboardSource(n,in), "nonzero branch rejected");
  n=fixture(); in.projectionTransform.values[11]=0;
  check(!webo::app::makeSceneBillboardSource(n,in), "zero projection divisor rejected");
  in.projectionTransform=identity(); in.parentCamera.values[0]=std::numeric_limits<double>::infinity();
  check(!webo::app::makeSceneBillboardSource(n,in), "nonfinite input rejected");
  in.parentCamera=identity(); in.projectionTransform=identity(); in.projectionTransform.values[11]=-3;
  auto negative=webo::app::makeSceneBillboardSource(n,in);
  check(bool(negative) && negative->slots[0].z < 0, "negative finite depth preserved, not clamped");
}

void nativeMatrixLayout() {
  auto n=fixture();
  n.matrix={1,0,0, 0,1,0, 0,0,1, 10,20,30}; // native row-vector12: 3x3, then translation
  webo::app::BillboardSourceInputs in;
  in.parentCamera=identity(); in.sourceTransform=identity(); in.projectionTransform=identity();
  in.center={0,0};
  auto got=webo::app::makeSceneBillboardSource(n,in);
  check(bool(got), "native identity-plus-translation matrix accepted");
  if(!got)return;
  check(got->sourceOffset==0xabc, "sourceOffset identity retained");
  const std::array<std::array<double,3>,4> expected{{
    {8,17,28}, {6,23,26}, {14,23,26}, {12,17,28}}};
  for(std::size_t i=0;i<4;++i) for(std::size_t j=0;j<3;++j)
    check(near(got->generatedPoints[i][j],expected[i][j]),"native matrix translation and point offsets");
}

struct RetailExpected {
  std::size_t offset;
  std::array<std::array<double,5>,4> slots; // x,y,z,u,v from independent repaired C3o table
};
constexpr RetailExpected retailExpected[] = {
  {0x967a2, {{{127.37244539895102,-7.5552263641560842,.3698773301636632,0,0}, {127.37244539895102,67.802330586103864,.3698773301636632,0,1}, {202.73000234921099,67.802330586103864,.3698773301636632,1,1}, {202.73000234921099,-7.5552263641560842,.3698773301636632,1,0}}}},
  {0x96892, {{{862.70247254781691,10.966628623825756,.38952624715678619,0,0}, {862.70247254781691,82.52291571838785,.38952624715678619,0,1}, {934.258759642379,82.52291571838785,.38952624715678619,1,1}, {934.258759642379,10.966628623825756,.38952624715678619,1,0}}}},
  {0x96980, {{{420.94764617361795,-7.2425655210323043,.41622268001780843,0,0}, {420.94764617361795,59.724116763618241,.41622268001780843,0,1}, {487.91432845826847,59.724116763618241,.41622268001780843,1,1}, {487.91432845826847,-7.2425655210323043,.41622268001780843,1,0}}}},
  {0x96a70, {{{706.19933409166538,-15.357001908385769,.3759984189587533,0,0}, {706.19933409166538,58.773767177310845,.3759984189587533,0,1}, {780.33010317736193,58.773767177310845,.3759984189587533,1,1}, {780.33010317736193,-15.357001908385769,.3759984189587533,1,0}}}},
  {0x96b60, {{{1115.3076558822909,22.433535311223892,.38781761183993063,0,0}, {1115.3076558822909,94.305082970392021,.38781761183993063,0,1}, {1187.1792035414589,94.305082970392021,.38781761183993063,1,1}, {1187.1792035414589,22.433535311223892,.38781761183993063,1,0}}}},
  {0x96c4e, {{{655.10880950056401,-4.3251483449317902,.47505202660794832,0,0}, {655.10880950056401,54.348534559743996,.47505202660794832,0,1}, {713.78249240523974,54.348534559743996,.47505202660794832,1,1}, {713.78249240523974,-4.3251483449317902,.47505202660794832,1,0}}}},
  {0x96d3e, {{{903.12886036036309,3.6443655675308548,.32643976939627428,0,0}, {903.12886036036309,89.029341864896395,.32643976939627428,0,1}, {988.51383665772869,89.029341864896395,.32643976939627428,1,1}, {988.51383665772869,3.6443655675308548,.32643976939627428,1,0}}}},
  {0x96e2c, {{{1078.4305169480206,13.601623687498204,.30710859537286272,0,0}, {1078.4305169480206,104.36121945386702,.30710859537286272,0,1}, {1169.1901127143894,104.36121945386702,.30710859537286272,1,1}, {1169.1901127143894,13.601623687498204,.30710859537286272,1,0}}}},
  {0x96f1a, {{{319.67562286483309,-3.4486737934319649,.37981904632779889,0,0}, {319.67562286483309,69.936408502738118,.37981904632779889,0,1}, {393.06070516100317,69.936408502738118,.37981904632779889,1,1}, {393.06070516100317,-3.4486737934319649,.37981904632779889,1,0}}}},
  {0x9700a, {{{555.36774497109297,21.840463747516822,.35489033321959595,0,0}, {555.36774497109297,100.38036569286314,.35489033321959595,0,1}, {633.90764691643926,100.38036569286314,.35489033321959595,1,1}, {633.90764691643926,21.840463747516822,.35489033321959595,1,0}}}},
  {0x970f8, {{{556.36829349626908,-1.6516393458855134,.45721606339017906,0,0}, {556.36829349626908,59.310899384150417,.45721606339017906,0,1}, {617.33083222630501,59.310899384150417,.45721606339017906,1,1}, {617.33083222630501,-1.6516393458855134,.45721606339017906,1,0}}}},
  {0x971e6, {{{724.73276754338394,47.399796090019294,.33254099580413038,0,0}, {724.73276754338394,131.21818938320541,.33254099580413038,0,1}, {808.55116083657003,131.21818938320541,.33254099580413038,1,1}, {808.55116083657003,47.399796090019294,.33254099580413038,1,0}}}},
  {0x9889b, {{{2715.9668866267125,227.06857688166409,.2029905804129879,0,0}, {2715.9668866267125,404.43865536853832,.2029905804129879,0,1}, {2893.3370378390682,404.43865536853832,.2029905804129879,1,1}, {2893.3370378390682,227.06857688166409,.2029905804129879,1,0}}}},
  {0x98989, {{{3309.3078106140065,233.73034744087417,.18756840518309889,0,0}, {3309.3078106140065,431.43385866095969,.18756840518309889,0,1}, {3507.011399393281,431.43385866095969,.18756840518309889,1,1}, {3507.011399393281,233.73034744087417,.18756840518309889,1,0}}}},
  {0x98a77, {{{2912.8623954078857,271.78073605089281,.16287490203768185,0,0}, {2912.8623954078857,484.89088361065069,.16287490203768185,0,1}, {3125.9726223435209,484.89088361065069,.16287490203768185,1,1}, {3125.9726223435209,271.78073605089281,.16287490203768185,1,0}}}},
  {0x98b65, {{{4065.3013900066812,288.58874348591513,.13677256317581699,0,0}, {4065.3013900066812,559.11063195681481,.13677256317581699,0,1}, {4335.8233906724345,559.11063195681481,.13677256317581699,1,1}, {4335.8233906724345,288.58874348591513,.13677256317581699,1,0}}}},
  {0x98c53, {{{5511.5211241090419,324.44743809550602,.092126823060699095,0,0}, {5511.5211241090419,681.94321557742774,.092126823060699095,0,1}, {5869.0170500969925,681.94321557742774,.092126823060699095,1,1}, {5869.0170500969925,324.44743809550602,.092126823060699095,1,0}}}},
};

void collect(const webo::SceneNode& n, std::map<std::size_t,const webo::SceneNode*>& found) {
  if(n.type==2) found[n.sourceOffset]=&n;
  for(const auto& child:n.children) collect(child,found);
}

void retail17() {
  const char* dir=std::getenv("WEBOLUTION_GAME_DIR");
  if(!dir||!*dir) { std::cout<<"C3o retained17 comparison skipped: WEBOLUTION_GAME_DIR unset\n"; return; }
  const auto base=std::filesystem::path(dir);
  auto lib=webo::AssetLibrary::open(base/"lemmings.box",base/"patch.box");
  check(bool(lib), "retail archives open"); if(!lib)return;
  std::string level;
  for(const auto& name:webo::levelFileNames(*lib)) if(name.find("MM_CLIMB_002")!=std::string::npos){level=name;break;}
  check(!level.empty(), "retained level found"); if(level.empty())return;
  auto bytes=lib->read(level); check(bool(bytes), "retained level bytes read"); if(!bytes)return;
  auto root=webo::readSceneNode(*bytes,0x91782);
  check(bool(root), "secondary retained tree parses"); if(!root)return;
  std::map<std::size_t,const webo::SceneNode*> nodes; collect(root->root,nodes);
  webo::app::BillboardSourceInputs in;
  in.parentCamera.values={-4.746057093143463,1.819797347616661e-23,-4.746058583259582,1.4210854715202e-14,
    .9290599077939987,6.582083702087402,-.9290596097707747,-25.03416633605957,
    .003250437148381025,-.0009175968443742022,-.003250435984227806,.1839417815208435};
  in.sourceTransform.values={1.489882230758667,0,0,0, 0,1.489882230758667,0,0, 0,0,2133.333251953125,0};
  in.projectionTransform.values={.67119395732879772,0,0,0, 0,.67119395732879716,0,0, 0,0,.00046875001862644884,0};
  in.center={319,239};
  for(const auto& ref:retailExpected) {
    auto it=nodes.find(ref.offset);
    check(it!=nodes.end(), "retained type2 found by sourceOffset"); if(it==nodes.end())continue;
    auto draw=webo::app::makeSceneBillboardSource(*it->second,in);
    check(bool(draw), "retained source geometry produced"); if(!draw)continue;
    check(draw->sourceOffset==ref.offset, "retained output sourceOffset matches source");
    for(std::size_t slot=0;slot<4;++slot) {
      const auto& v=draw->slots[slot];
      const double actual[5]={v.x,v.y,v.z,v.u,v.v};
      for(std::size_t field=0;field<5;++field)
        check(std::abs(actual[field]-ref.slots[slot][field])<=1e-9, "retail slot XY/Z/UV within 1e-9");
    }
  }
}
}
int main() {
  geometry(); invalid(); nativeMatrixLayout(); retail17();
  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? 1 : 0;
}
