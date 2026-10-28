#include "app/MapScene.h"
#include "assets/Lrz.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

static std::uint16_t le16(const std::byte* p) {
  return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(p[0]) |
      (static_cast<unsigned>(std::to_integer<std::uint8_t>(p[1])) << 8));
}
static std::uint32_t le32(const std::byte* p) {
  return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[0])) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[1])) << 8) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[2])) << 16) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[3])) << 24);
}

struct RawMask {
  std::vector<std::uint8_t> topDown;
  std::uint8_t background = 0;
};

static std::optional<RawMask> readRawMask(const AssetLibrary& lib) {
  auto packed=lib.read("TEXTURES\\LEVEL MASK.BMP");
  if(!packed) return std::nullopt;
  auto decoded=lrzDecode(*packed);
  if(!decoded || decoded->size()<54) return std::nullopt;
  const auto& bmp=*decoded;
  const std::uint32_t offset=le32(bmp.data()+10), dib=le32(bmp.data()+14);
  if(std::to_integer<char>(bmp[0])!='B' || std::to_integer<char>(bmp[1])!='M' ||
     dib<40 || static_cast<std::size_t>(14)+dib>bmp.size() ||
     le32(bmp.data()+18)!=640 || le32(bmp.data()+22)!=480 ||
     le16(bmp.data()+26)!=1 || le16(bmp.data()+28)!=8 || le32(bmp.data()+30)!=0 ||
     offset<14u+dib || offset>bmp.size() || 640u*480u>bmp.size()-offset)
    return std::nullopt;
  RawMask mask; mask.topDown.resize(640u*480u);
  mask.background=std::to_integer<std::uint8_t>(bmp[offset]);
  for(std::size_t storedY=0;storedY<480;++storedY)
    for(std::size_t x=0;x<640;++x)
      mask.topDown[(479-storedY)*640+x]=std::to_integer<std::uint8_t>(bmp[offset+storedY*640+x]);
  return mask;
}

static std::uint64_t fnv1a(const std::vector<std::uint8_t>& bytes) {
  std::uint64_t h = 14695981039346656037ull;
  for (auto b : bytes) { h ^= b; h *= 1099511628211ull; }
  return h;
}

int main() {
  const char* configured = std::getenv("WEBOLUTION_GAME_DIR");
  const fs::path game = configured ? configured : "C:/progs/Lemmings Revolution";
  auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) { std::cerr << lib.error() << '\n'; return 1; }
  const auto rawMask=readRawMask(*lib);
  if(!rawMask) { std::cerr << "could not independently decode indexed LEVEL MASK.BMP\n"; return 1; }
  std::size_t node0Pixels=0, node0BboxHoles=0;
  for(int y=195;y<212;++y) for(int x=201;x<218;++x) {
    const auto index=rawMask->topDown[static_cast<std::size_t>(y)*640+x];
    node0Pixels+=index==0;
    node0BboxHoles+=index!=0;
  }
  if(node0Pixels!=221 || node0BboxHoles!=68) {
    std::cerr << "node 0 mask/bounding-box oracle mismatch: " << node0Pixels << "/" << node0BboxHoles << '\n';
    return 1;
  }
  constexpr std::array<std::uint64_t,4> expected{
      0x32e6550faa1a737full, 0x2a1a426d3506ca16ull,
      0x9675eb561ebc9d5dull, 0x2ed439883e49b779ull};
  std::optional<std::pair<int,int>> lockedPixel;
  for (int fixture=0; fixture<4; ++fixture) {
    std::array<std::int32_t,102> flags{};
    if (fixture==1) flags[0]=1;
    if (fixture==2) flags.fill(1);
    if (fixture==3) for (int i=0;i<102;++i) flags[i]=(i%7==2);
    auto scene=app::MapScene::build(*lib,flags);
    if (!scene || scene->image().width!=640 || scene->image().height!=480 ||
        fnv1a(scene->image().rgba)!=expected[fixture]) {
      std::cerr << "map SOURCE fixture mismatch " << fixture << '\n'; return 1;
    }
    if (fixture==0) {
      int state1=0,state3=0; for(auto s:scene->states()) { state1+=s==1; state3+=s==3; }
      if(state1!=99||state3!=3) { std::cerr << "none state fixture mismatch\n"; return 1; }
      if(scene->pickNode(206,204)!=0 || scene->levelForNode(0)!="MM_CLIMB_002" ||
         scene->pickNode(0,0) || scene->pickNode(-1,204) || scene->pickNode(640,204) ||
         scene->pickNode(206,-1) || scene->pickNode(206,480) || scene->levelForNode(102) ||
         scene->levelForNode(999)) {
        std::cerr << "indexed map selection / retail node mapping mismatch\n"; return 1;
      }
    }
    for(int y=0;y<480;++y) for(int x=0;x<640;++x) {
      const auto index=rawMask->topDown[static_cast<std::size_t>(y)*640+x];
      std::optional<std::size_t> want;
      if(index!=rawMask->background && index<scene->states().size() &&
         (scene->states()[index]==2 || scene->states()[index]==3)) want=index;
      if(scene->pickNode(x,y)!=want) {
        std::cerr << "exhaustive indexed mask pick mismatch at " << x << ',' << y
                  << " fixture " << fixture << '\n'; return 1;
      }
      if(fixture==0 && !lockedPixel && index!=rawMask->background && index<102 &&
         scene->states()[index]==1) lockedPixel=std::pair{x,y};
    }
    if (fixture==1 && (scene->pickNode(206,204)!=0 || scene->states()[0]!=2)) {
      std::cerr << "flagged node eligibility transition mismatch\n"; return 1;
    }
    if (fixture==2 && scene->pickNode(206,204)!=0) {
      std::cerr << "fully unlocked node selection mismatch\n"; return 1;
    }
    if (fixture==2) for(auto s:scene->states()) if(s!=2) {
      std::cerr << "all-flags state fixture mismatch\n"; return 1;
    }
  }
  if(!lockedPixel) { std::cerr << "raw mask has no locked-node sample\n"; return 1; }
  auto locked=app::MapScene::build(*lib,std::array<std::int32_t,102>{});
  std::array<std::int32_t,102> allUnlockedFlags{}; allUnlockedFlags.fill(1);
  auto unlocked=app::MapScene::build(*lib,allUnlockedFlags);
  if(!locked || !unlocked || locked->pickNode(lockedPixel->first,lockedPixel->second) ||
     unlocked->pickNode(lockedPixel->first,lockedPixel->second)!=
       rawMask->topDown[static_cast<std::size_t>(lockedPixel->second)*640+lockedPixel->first]) {
    std::cerr << "locked-node mask sample eligibility transition mismatch\n"; return 1;
  }
  const auto temp=fs::temp_directory_path()/"webolution-map-flags-test.txt";
  { std::ofstream out(temp); for(int i=0;i<102;++i) out << (i?" ":"") << (i==17?-7:0); }
  auto parsed=app::readMapFlags(temp); fs::remove(temp);
  if(!parsed || (*parsed)[17]!=-7 || (*parsed)[16]!=0) { std::cerr << "signed flags parse failed\n"; return 1; }
  { std::ofstream out(temp); out << "0 1 nope"; }
  parsed=app::readMapFlags(temp); fs::remove(temp);
  if(parsed) { std::cerr << "malformed flags accepted\n"; return 1; }
  { std::ofstream out(temp); for(int i=0;i<103;++i) out << "0 "; }
  parsed=app::readMapFlags(temp); fs::remove(temp);
  if(parsed) { std::cerr << "extra flags accepted\n"; return 1; }
  std::cout << "PASS: four accepted SOURCE compositions, indexed mask picking, retail node mapping, state fixtures, strict flag parser\n";
  return 0;
}
