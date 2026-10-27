#include "app/MapScene.h"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using namespace webo;

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
  constexpr std::array<std::uint64_t,4> expected{
      0x32e6550faa1a737full, 0x2a1a426d3506ca16ull,
      0x9675eb561ebc9d5dull, 0x2ed439883e49b779ull};
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
    }
    if (fixture==2) for(auto s:scene->states()) if(s!=2) {
      std::cerr << "all-flags state fixture mismatch\n"; return 1;
    }
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
  std::cout << "PASS: four accepted SOURCE compositions, state fixtures, strict flag parser\n";
  return 0;
}
