#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/SceneNode.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <new>
#include <string_view>
#include <cstdlib>

namespace {
int checks = 0, failures = 0;
bool allocationGuard = false;
std::size_t oversizedAllocation = 0;
void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) { ++failures; std::cerr << " FAIL " << what << '\n'; }
}
}  // namespace

void* operator new(std::size_t n) {
  if (allocationGuard && n > 1024 * 1024) {
    oversizedAllocation = n;
    throw std::bad_alloc();
  }
  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void* operator new[](std::size_t n) {
  if (allocationGuard && n > 1024 * 1024) {
    oversizedAllocation = n;
    throw std::bad_alloc();
  }
  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {
struct Bytes {
  std::vector<std::byte> b;
  void u8(std::uint8_t x) { b.push_back(std::byte{x}); }
  void u32(std::uint32_t x) { for (int i=0;i<4;++i) u8((x>>(8*i))&255); }
  void f64(double x) { auto p=reinterpret_cast<const std::byte*>(&x); b.insert(b.end(),p,p+8); }
  void str(std::string_view s) { u32(static_cast<std::uint32_t>(s.size())); for (unsigned char c:s) u8(c); }
  void vec3(double a,double c,double d) { f64(a); f64(c); f64(d); }
  void common(std::string_view name, std::uint32_t children=0) {
    for (double x : {1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12.}) f64(x);
    str(name); f64(13.25); f64(-14.5); u32(children);
  }
};

Bytes synthetic() {
  Bytes w;
  // root type 0 with a nested mesh node followed by a nested type-2 source record
  w.u32(0); w.common("root",2);
  w.u32(1); // mesh node
  w.u32(2); w.vec3(1.25,-2.5,3.75); w.vec3(-4.5,5.25,-6.75);
  w.u32(1); w.vec3(7.125,-8.25,9.5);
  w.u32(1); w.str("material/asymmetric"); w.u32(0x12345678);
  w.u32(3);
  for (std::uint32_t kind : {5u,6u,7u}) {
    w.u32(0xaabbccdd); w.str("PolyList 09"); w.u32(kind); w.u32(1); w.u32(0);
    w.u32(0); // leading normal index
    if (kind == 6) { w.f64(101.5); w.f64(102.5); w.f64(103.5); w.f64(104.5); }
    const unsigned n=kind==5?3:4;
    for (unsigned c=0;c<n;++c) {
      w.u32(c%2); w.f64(10.125+c); w.f64(-20.25-c);
      if (kind!=6) { w.f64(30.5+c); w.f64(40.25+c); w.f64(50.75+c); w.f64(60.875+c); }
    }
  }
  w.common("mesh-child");
  w.u32(2); // type 2 with branch 0, including zero-valued retained field
  w.f64(71.25); w.f64(-72.5); w.f64(73.75); w.u8(0xa5); w.u8(0);
  w.str("branch-data"); for(double x:{0.,-1.25,2.5,3.75}) w.f64(x); w.u32(0);
  w.common("type2-child");
  return w;
}
void testSynthetic() {
  auto bytes=synthetic();
  // Prefix demonstrates that caller-selected offsets, not root scans, control parsing.
  std::vector<std::byte> pref(7,std::byte{0xee}); pref.insert(pref.end(),bytes.b.begin(),bytes.b.end());
  auto parsed=webo::readSceneNode(pref,7);
  check(parsed.has_value(), parsed ? "synthetic parses" : parsed.error());
  if (!parsed) return;
  const auto& root=parsed->root;
  check(parsed->endOffset==pref.size() && root.endOffset==pref.size(),"consumed exact node bytes");
  check(root.name=="root" && root.matrix[1]==2. && root.headDoubles[0]==13.25 && root.headDoubles[1]==-14.5,"common fields preserved without defaults");
  check(root.children.size()==2 && root.children[0].name=="mesh-child" && root.children[1].name=="type2-child","recursive hierarchy/order");
  const auto& mesh=root.children[0];
  check(mesh.positions.size()==2 && mesh.positions[1][1]==5.25 && mesh.normals[0][2]==9.5,"asymmetric node-local vectors");
  check(mesh.materials[0].name=="material/asymmetric" && mesh.materials[0].blendSelector==0x12345678,"material string and distinct blend selector");
  check(mesh.meshes.size()==3 && mesh.meshes[0].kind==5 && mesh.meshes[1].kind==6 && mesh.meshes[2].kind==7,"native block kinds and count");
  const auto& k5=mesh.meshes[0].records[0];
  check(k5.normalIndex==0 && k5.corners.size()==3 && k5.corners[2].u==12.125 && k5.corners[2].v==-22.25 && k5.corners[2].a==62.875,"kind-5 leading normal, UV order and per-corner attributes");
  const auto& k6=mesh.meshes[1].records[0];
  check(k6.corners.size()==4 && k6.corners[0].r==101.5 && k6.corners[3].a==104.5 && k6.corners[3].u==13.125,"kind-6 shared attributes with four ordered UV/index corners");
  check(mesh.meshes[2].records[0].corners[0].r==30.5 && mesh.meshes[2].records[0].corners[3].g==43.25,"kind-7 per-corner attributes");
  const auto& t=root.children[1].type2;
  check(t.visibility==0xa5 && t.branch==0 && t.branchName=="branch-data" && t.branchDoubles[0]==0. && t.branchDoubles[3]==3.75 && t.trailingWord==0,"type-2 source payload preserves zeros and visibility");
}
void testErrors() {
  auto b=synthetic();
  check(!webo::readSceneNode(b.b,b.b.size()+1),"start past input rejected");
  for (std::size_t n=0;n<12;++n) check(!webo::readSceneNode(std::span(b.b).first(n),0),"truncated root rejected");
  for (std::size_t n=12;n<b.b.size();++n)
    check(!webo::readSceneNode(std::span(b.b).first(n),0),"every truncated mesh/type2/common prefix rejected");
  auto bad=b.b; bad[0]=std::byte{3};
  auto unknown=webo::readSceneNode(bad,0); check(!unknown && unknown.error().find("unsupported scene node type 3")!=std::string::npos,"unknown type fails closed");
  // Child count is the final u32 of a minimal node and cannot trigger allocation.
  Bytes huge; huge.u32(0); huge.common("x",0); // replace count at known final offset
  const auto countAt=huge.b.size()-4;
  for(int i=0;i<4;++i) huge.b[countAt+i]=std::byte{0xff};
  auto countErr=webo::readSceneNode(huge.b,0); check(!countErr && countErr.error().find("exceeds limit")!=std::string::npos,"malicious count rejected");

  // Exact 124-byte type-0 input: 9999 children cannot fit, and rejection must
  // happen without the multi-megabyte eager vector reservation from repair1.
  Bytes nearLimit; nearLimit.u32(0); nearLimit.common("",9999);
  check(nearLimit.b.size()==124,"near-limit child fixture is exactly 124 bytes");
  oversizedAllocation=0; allocationGuard=true;
  auto nearResult=webo::readSceneNode(nearLimit.b,0);
  allocationGuard=false;
  check(!nearResult && oversizedAllocation==0,"9999 impossible children return expected error with no >1MiB allocation");

  Bytes nested; nested.u32(0); nested.common("parent",2);
  nested.u32(0); nested.common("first",9999);
  nested.u32(0); nested.common("second",0);
  oversizedAllocation=0; allocationGuard=true;
  auto nestedResult=webo::readSceneNode(nested.b,0);
  allocationGuard=false;
  check(!nestedResult && oversizedAllocation==0,"nested impossible child promises reject without sibling reserve amplification");

  // Untrusted type-1 collection counts with no payload must fail before any
  // count-sized material/block/record reservation.
  Bytes manyMaterials; manyMaterials.u32(1); manyMaterials.u32(0); manyMaterials.u32(0); manyMaterials.u32(99999);
  Bytes manyBlocks; manyBlocks.u32(1); manyBlocks.u32(0); manyBlocks.u32(0); manyBlocks.u32(0); manyBlocks.u32(99999);
  Bytes manyRecords; manyRecords.u32(1); manyRecords.u32(0); manyRecords.u32(0); manyRecords.u32(1); manyRecords.str("m"); manyRecords.u32(0);
  manyRecords.u32(1); manyRecords.u32(0); manyRecords.str("PolyList 09"); manyRecords.u32(5); manyRecords.u32(99999); manyRecords.u32(0);
  for(auto* fixture:{&manyMaterials,&manyBlocks,&manyRecords}){
    oversizedAllocation=0;allocationGuard=true;auto result=webo::readSceneNode(fixture->b,0);allocationGuard=false;
    check(!result && oversizedAllocation==0,"impossible material/block/record count avoids oversized allocation");
  }
  // Invalid-index case built as a compact, exact one-record type-1 tree.
  Bytes x; x.u32(1); x.u32(1); x.vec3(0,0,0); x.u32(1); x.vec3(0,0,1); x.u32(1); x.str("m"); x.u32(0);
  x.u32(1); x.u32(0); x.str("PolyList 09"); x.u32(5); x.u32(1); x.u32(0); x.u32(0); x.u32(1);
  for(int i=0;i<6;++i) x.f64(i+1);
  x.u32(0);
  for(int i=0;i<6;++i) x.f64(i+2);
  x.u32(0);
  for(int i=0;i<6;++i) x.f64(i+3);
  x.common("invalid");
  auto badIdx=webo::readSceneNode(x.b,0); check(!badIdx && badIdx.error().find("vertex index 1 outside")!=std::string::npos,"invalid vertex index rejected");
  // Unsupported type-2 branch is explicit (payload cannot be safely shaped).
  Bytes branch; branch.u32(2); for(double d:{1.,2.,3.})branch.f64(d); branch.u8(1); branch.u8(1);
  auto branchErr=webo::readSceneNode(branch.b,0); check(!branchErr && branchErr.error().find("unsupported nonzero type-2 branch")!=std::string::npos,"unproved type-2 branch rejected");
}

void tally(const webo::SceneNode& n, std::size_t& nodes, std::size_t& meshes, std::size_t& records,
           std::array<std::size_t,8>& kinds) {
  ++nodes;
  if(n.type==1)++meshes;
  for(const auto& b:n.meshes){kinds[b.kind]+=b.records.size();records+=b.records.size();}
  for(const auto& c:n.children)tally(c,nodes,meshes,records,kinds);
}
void testRetail() {
  const char* env=std::getenv("WEBOLUTION_GAME_DIR");
  if(!env||!*env){std::cout<<"retail trees skipped: WEBOLUTION_GAME_DIR unset\n";return;}
  auto lib=webo::AssetLibrary::open(std::filesystem::path(env)/"lemmings.box",std::filesystem::path(env)/"patch.box");
  check(lib.has_value(),lib?"retail archive opens":lib.error()); if(!lib)return;
  std::string file;
  for(const auto& n:webo::levelFileNames(*lib)) if(n.find("MM_CLIMB_002")!=std::string::npos){file=n;break;}
  check(!file.empty(),"MM_CLIMB_002 found"); if(file.empty())return;
  auto blob=lib->read(file); check(blob.has_value(),"MM_CLIMB_002 bytes read"); if(!blob)return;
  struct Expected{std::size_t root,end,nodes,meshes,records;std::array<std::size_t,8> kinds;};
  // Independent C3n raw-source table: exact caller roots and consumed endpoints;
  // aggregate record census was obtained by its separate raw-byte checker.
  const Expected expected[]={{0x59644,0x9176e,4,3,923,{}}, {0x91782,0x98d41,30,12,82,{}},
                             {0x9ccd1,0x9db2b,4,3,16,{}}, {0xa0cdd,0xaaa43,2,1,156,{}}};
  std::size_t allNodes=0,allMeshes=0,allRecords=0;std::array<std::size_t,8> allKinds{};
  for(const auto&e:expected){
    auto r=webo::readSceneNode(*blob,e.root); check(r.has_value(),r?"retained root parses":r.error()); if(!r)continue;
    std::size_t nodes=0,meshes=0,records=0;std::array<std::size_t,8> kinds{};tally(r->root,nodes,meshes,records,kinds);
    std::cout<<std::format("retail 0x{:x}: nodes={} meshes={} records={} kinds={}/{}/{} end=0x{:x}\n",e.root,nodes,meshes,records,kinds[5],kinds[6],kinds[7],r->endOffset);
    check(r->endOffset==e.end,std::format("tree at 0x{:x} ends at independent offset 0x{:x}",e.root,e.end));
    check(nodes==e.nodes && meshes==e.meshes && records==e.records,std::format("tree at 0x{:x} node/mesh/record count",e.root));
    allNodes+=nodes;allMeshes+=meshes;allRecords+=records;for(size_t k=0;k<8;++k)allKinds[k]+=kinds[k];
    if(e.root==0x59644){
      check(r->root.name=="thenull"&&r->root.matrix[0]==10.&&r->root.headDoubles[0]==12691.934971574985,"independent root name/matrix/head values");
      check(r->root.children[0].name=="sand7"&&r->root.children[0].positions.size()==933&&r->root.children[0].positions[0][0]==5.071678161621094,"independent sand7 vertex count/value");
      const auto& block=r->root.children[0].meshes[0];
      check(block.kind==5&&block.records[0].sourceOffset==0x634b7&&block.records[0].normalIndex==0&&block.records[0].corners[0].sourceOffset==0x634bb&&block.records[0].corners[0].u==0.9895833134651184,"independent kind-5 offset/normal/UV");
    }
    if(e.root==0x9ccd1){
      bool found=false;
      auto findCube=[&](auto&& self,const webo::SceneNode& n)->void {
      if(n.name=="cube8"&&!n.meshes.empty()&&n.meshes[0].kind==6){
        const auto& rec=n.meshes[0].records[0];
        check(rec.sourceOffset==0x9cef6&&rec.normalIndex==0&&rec.corners.size()==4&&rec.corners[0].u==0.984375&&rec.corners[0].v==0.016560792922973633&&rec.corners[3].u==0.984375,"independent kind-6 source fields and UV order"); found=true;
      }
      for(const auto& c:n.children)self(self,c);
      };
      findCube(findCube,r->root);
      check(found,"retained tree has independently identified kind-6 cube8");
    }
  }
  check(allNodes==40&&allMeshes==19&&allRecords==1177,"four retained trees total 40 nodes, 19 meshes, 1177 records");
  check(allKinds[5]==465&&allKinds[6]==16&&allKinds[7]==696,"independent kind record census 465/16/696");
}
}
int main(){testSynthetic();testErrors();testRetail();std::cout<<std::format("{} checks, {} failures\n",checks,failures);return failures?1:0;}
