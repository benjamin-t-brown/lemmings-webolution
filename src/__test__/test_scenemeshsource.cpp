#include "app/SceneMeshSource.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace {
int checks = 0, failures = 0;
void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) { ++failures; std::cerr << " FAIL " << what << '\n'; }
}
bool near(double a, double b) { return std::abs(a - b) <= 1e-12; }
using M = std::array<double, 12>;
std::array<double, 3> point(std::array<double, 3> p, const M& m) {
  std::array<double, 3> q{};
  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < 3; ++i) q[j] += p[i] * m[3*i+j];
    q[j] += m[9+j];
  }
  return q;
}
bool eq(const std::array<double, 3>& a, const std::array<double, 3>& b) {
  return near(a[0],b[0]) && near(a[1],b[1]) && near(a[2],b[2]);
}
void testTransformAndAttributes() {
  // Three deliberately noncommuting affine transforms; each is caller/source data.
  const M parent{1,2,0, 0,1,3, 4,0,1, 5,-2,7};
  const M rootM{0,1,0, -1,0,0, 0,0,2, 3,4,-1};
  const M childM{1,0,2, 0,3,0, -1,0,1, -2,5,6};
  const M view{2,0,1, 1,1,0, 0,-2,1, 9,8,7};
  webo::SceneNode root; root.type=0; root.sourceOffset=0x10; root.matrix=rootM;
  webo::SceneNode mesh; mesh.type=1; mesh.sourceOffset=0x20; mesh.matrix=childM;
  mesh.positions={{{1.25,-2.5,3.75}},{{-4,0.5,2}}};
  mesh.normals={{{0.125,-0.25,0.5}}};
  mesh.materials={{0x30,"logical/material",0x12345678},{0x31,"",9}};

  webo::SceneMeshBlock tri; tri.sourceOffset=0x40; tri.kind=5; tri.materialSlot=0;
  webo::SceneRecord rec; rec.sourceOffset=0x50; rec.normalIndex=0;
  rec.corners={{0x60,0,1,2,3,4,5,6}, {0x70,1,7,8,9,10,11,12},
               {0x80,0,13,14,15,16,17,18}};
  tri.records.push_back(rec); mesh.meshes.push_back(tri);
  webo::SceneMeshBlock quad; quad.sourceOffset=0x90; quad.kind=6; quad.materialSlot=1;
  webo::SceneRecord shared; shared.sourceOffset=0xa0; shared.normalIndex=0;
  shared.corners={{0xb0,1,21,22,31,32,33,34}, {0xc0,0,23,24,31,32,33,34},
                  {0xd0,1,25,26,31,32,33,34}, {0xe0,0,27,28,31,32,33,34}};
  quad.records.push_back(shared); mesh.meshes.push_back(quad);
  root.children.push_back(mesh);

  auto built=webo::buildSceneMeshSource(root,parent,view);
  check(built && built->size()==2,"two mesh records in depth-first file order");
  if (!built || built->size()!=2) return;
  const auto& a=(*built)[0];
  check(a.nodeSourceOffset==0x20 && a.blockSourceOffset==0x40 && a.recordSourceOffset==0x50,
        "candidate retains node/block/record source identity");
  check(a.kind==5 && a.materialSlot==0 && a.materialName=="logical/material" &&
        a.blendSelector==0x12345678,"material logical name and selector preserved");
  check(a.corners.size()==3 && a.corners[0].sourceOffset==0x60 &&
        a.corners[2].sourceOffset==0x80 && a.corners[2].vertexIndex==0,
        "corner order, offsets and shared vertex identity preserved");
  const auto local=mesh.positions[0];
  const auto world=point(point(point(local,childM),rootM),parent);
  const auto inView=point(world,view);
  check(eq(a.corners[0].localPosition,local) && eq(a.corners[0].worldPosition,world) &&
        eq(a.corners[0].viewPosition,inView),"child then root then explicit parent then view transforms");
  check(!eq(a.corners[0].viewPosition,point(point(point(local,childM),parent),view)),
        "noncommuting matrices distinguish hierarchy composition order");
  check(a.corners[0].normalIndex==0 && a.corners[0].localNormal==mesh.normals[0],
        "raw local normal/index retained without transform");
  check(a.corners[0].sourceAttributes==std::array<double,6>{1,2,3,4,5,6} &&
        a.corners[0].attributes==std::array<double,6>{13,14,15,16,17,18} &&
        a.corners[2].attributes==a.corners[0].attributes,
        "per-record repeated-index last-store wins while source attributes remain linked");
  const auto& b=(*built)[1];
  check(b.kind==6 && b.materialName.empty() && b.materialSlot==1 && b.blendSelector==9,
        "kind-6 shared RGBA and empty logical material are preserved without handle inference");
  check(b.corners.size()==4 && b.corners[0].attributes==std::array<double,6>{25,26,31,32,33,34} &&
        b.corners[1].attributes==std::array<double,6>{27,28,31,32,33,34},
        "kind-6 parser-expanded shared RGBA and alias UV winner are retained");
}
void testHandBuiltFailures() {
  const M identity{1,0,0,0,1,0,0,0,1,0,0,0};
  webo::SceneNode root; root.type=1; root.normals={{{0,0,1}}};
  root.positions={{{0,0,0}}}; root.materials={{0,"",0}};
  webo::SceneMeshBlock block; block.kind=5; block.materialSlot=0;
  webo::SceneRecord record; record.normalIndex=0;
  record.corners={{0,0,0,0,0,0,0,0},{0,1,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
  block.records={record}; root.meshes={block};
  check(!webo::buildSceneMeshSource(root,identity,identity),"invalid hand-built vertex index is an expected error");
  root.meshes[0].records[0].corners[1].vertexIndex=0;
  root.meshes[0].materialSlot=1;
  check(!webo::buildSceneMeshSource(root,identity,identity),"invalid hand-built material slot is an expected error");
  root.meshes[0].materialSlot=0; root.meshes[0].kind=4;
  check(!webo::buildSceneMeshSource(root,identity,identity),"unsupported hand-built mesh kind is an expected error");
}
void testRetailOptional() {
  const char* env=std::getenv("WEBOLUTION_GAME_DIR");
  if(!env||!*env) { std::cout << "retail scene mesh candidates skipped: WEBOLUTION_GAME_DIR unset\n"; return; }
  auto lib=webo::AssetLibrary::open(std::filesystem::path(env)/"lemmings.box",
                                    std::filesystem::path(env)/"patch.box");
  check(lib.has_value(),lib?"retail archive opens for source candidates":lib.error()); if(!lib)return;
  std::string file;
  for(const auto& n:webo::levelFileNames(*lib)) if(n.find("MM_CLIMB_002")!=std::string::npos){file=n;break;}
  check(!file.empty(),"MM_CLIMB_002 found for candidate comparison"); if(file.empty())return;
  auto bytes=lib->read(file); check(bytes.has_value(),"retail level bytes read for candidates"); if(!bytes)return;
  const std::array<std::size_t,4> roots{0x59644,0x91782,0x9ccd1,0xa0cdd};
  const std::array<std::size_t,4> records{923,82,16,156};
  const M identity{1,0,0,0,1,0,0,0,1,0,0,0};
  std::size_t totalRecords=0,totalCorners=0;
  for(std::size_t i=0;i<roots.size();++i){
    auto parsed=webo::readSceneNode(*bytes,roots[i]);
    check(parsed.has_value(),parsed?"retail source tree parses":parsed.error()); if(!parsed)continue;
    auto output=webo::buildSceneMeshSource(parsed->root,identity,identity);
    check(output.has_value(),output?"retail source candidates build":output.error()); if(!output)continue;
    std::size_t corners=0; for(const auto& r:*output) corners+=r.corners.size();
    check(output->size()==records[i],"candidate record count matches independent C3n source table");
    totalRecords+=output->size();totalCorners+=corners;
    if(i==0){
      bool found=false;
      for(const auto& r:*output) if(r.recordSourceOffset==0x634b7 && !r.corners.empty()){
        found=true;
        check(r.corners[0].sourceOffset==0x634bb && r.corners[0].sourceAttributes[0]==0.9895833134651184 &&
              r.corners[0].sourceAttributes[1]==0.9895833134651184,
              "C3n independently indexed first kind-5 corner offset/UV retained");
      }
      check(found,"C3n independently identified kind-5 record is emitted");
    }
  }
  check(totalRecords==1177 && totalCorners==4243,"four optional retail trees match C3n 1177-record/4243-corner census");
}
}
int main() {
  testTransformAndAttributes(); testHandBuiltFailures(); testRetailOptional();
  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
