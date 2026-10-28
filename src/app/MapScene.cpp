#include "app/MapScene.h"
#include "assets/Bitmap.h"
#include "assets/GameData.h"
#include "assets/Lrz.h"
#include <array>
#include <fstream>
#include <limits>
#include <sstream>

namespace webo::app {
namespace {
std::uint16_t read16(const std::byte* p) {
  return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(p[0]) |
      (static_cast<unsigned>(std::to_integer<std::uint8_t>(p[1])) << 8));
}
std::uint32_t read32(const std::byte* p) {
  return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[0])) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[1])) << 8) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[2])) << 16) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[3])) << 24);
}
struct IndexedMask { std::vector<std::uint8_t> pixels; std::uint8_t background; };
std::expected<IndexedMask, std::string> decodeMapMask(const AssetLibrary& lib) {
  auto packed = lib.read("TEXTURES\\LEVEL MASK.BMP");
  if (!packed) return std::unexpected("map mask: "+packed.error());
  auto decoded = lrzDecode(*packed);
  if (!decoded) return std::unexpected("map mask: "+decoded.error());
  const auto& bmp = *decoded;
  if (bmp.size() < 54 || std::to_integer<char>(bmp[0]) != 'B' || std::to_integer<char>(bmp[1]) != 'M')
    return std::unexpected("map mask: malformed BMP header");
  const auto offset=read32(bmp.data()+10), dib=read32(bmp.data()+14);
  if (dib < 40 || static_cast<std::size_t>(14)+dib > bmp.size() ||
      read32(bmp.data()+18)!=640 || read32(bmp.data()+22)!=480 ||
      read16(bmp.data()+26)!=1 || read16(bmp.data()+28)!=8 || read32(bmp.data()+30)!=0)
    return std::unexpected("map mask must be uncompressed 640x480 indexed 8bpp BMP");
  constexpr std::size_t pixels=640u*480u;
  if (offset < 14u+dib || offset > bmp.size() || pixels > bmp.size()-offset)
    return std::unexpected("map mask pixel data is truncated");
  IndexedMask result; result.pixels.resize(pixels);
  for (std::size_t row=0; row<480; ++row)
    for (std::size_t x=0; x<640; ++x)
      result.pixels[(479-row)*640+x]=std::to_integer<std::uint8_t>(bmp[offset+row*640+x]);
  result.background=std::to_integer<std::uint8_t>(bmp[offset]);
  return result;
}
constexpr std::int32_t kLinkEnd = 0x8000;
constexpr std::array<std::array<std::int32_t, 3>, 102> kMapping = {{
  {{0, 0, 0}},
  {{0, 1, 0}},
  {{0, 2, 0}},
  {{0, 3, 0}},
  {{0, 4, 0}},
  {{0, 5, 0}},
  {{0, 6, 0}},
  {{0, 7, 0}},
  {{0, 8, 0}},
  {{0, 9, 0}},
  {{0, 10, 0}},
  {{4, 4, 0}},
  {{0, 11, 0}},
  {{0, 12, 0}},
  {{0, 13, 0}},
  {{0, 14, 0}},
  {{0, 15, 0}},
  {{0, 16, 0}},
  {{0, 17, 0}},
  {{0, 18, 0}},
  {{0, 19, 0}},
  {{0, 20, 0}},
  {{0, 21, 0}},
  {{0, 22, 0}},
  {{0, 23, 0}},
  {{0, 24, 0}},
  {{1, 0, 0}},
  {{1, 1, 0}},
  {{1, 2, 0}},
  {{4, 12, 0}},
  {{1, 3, 0}},
  {{1, 4, 0}},
  {{1, 5, 0}},
  {{1, 6, 0}},
  {{1, 7, 0}},
  {{1, 8, 0}},
  {{1, 9, 0}},
  {{1, 10, 0}},
  {{1, 11, 0}},
  {{1, 12, 0}},
  {{1, 13, 0}},
  {{1, 14, 0}},
  {{1, 15, 0}},
  {{1, 16, 0}},
  {{1, 17, 0}},
  {{1, 18, 0}},
  {{4, 6, 0}},
  {{1, 19, 0}},
  {{1, 20, 0}},
  {{1, 21, 0}},
  {{1, 22, 0}},
  {{1, 23, 0}},
  {{1, 24, 0}},
  {{2, 0, 0}},
  {{2, 1, 0}},
  {{2, 2, 0}},
  {{2, 3, 0}},
  {{4, 15, 0}},
  {{2, 4, 0}},
  {{2, 5, 0}},
  {{2, 6, 0}},
  {{2, 7, 0}},
  {{2, 8, 0}},
  {{2, 9, 0}},
  {{2, 10, 0}},
  {{2, 11, 0}},
  {{2, 12, 0}},
  {{2, 13, 0}},
  {{4, 11, 0}},
  {{2, 14, 0}},
  {{2, 15, 0}},
  {{2, 16, 0}},
  {{2, 17, 0}},
  {{2, 19, 0}},
  {{2, 20, 0}},
  {{2, 21, 0}},
  {{2, 22, 0}},
  {{2, 23, 0}},
  {{4, 14, 0}},
  {{3, 0, 0}},
  {{3, 2, 0}},
  {{3, 4, 0}},
  {{3, 5, 0}},
  {{3, 6, 0}},
  {{3, 7, 0}},
  {{3, 8, 0}},
  {{3, 9, 0}},
  {{3, 10, 0}},
  {{3, 11, 1}},
  {{3, 12, 1}},
  {{3, 13, 1}},
  {{4, 13, 1}},
  {{3, 14, 1}},
  {{3, 15, 1}},
  {{3, 16, 1}},
  {{3, 17, 1}},
  {{3, 18, 1}},
  {{3, 20, 1}},
  {{3, 21, 1}},
  {{3, 22, 1}},
  {{3, 23, 1}},
  {{3, 24, 1}},
}};
constexpr std::array<std::array<std::int32_t, 6>, 7> kRects = {{
  {{0, 0, 34, 30, 12, 13}},
  {{35, 0, 30, 30, 15, 15}},
  {{64, 0, 34, 30, 12, 13}},
  {{98, 0, 30, 30, 15, 15}},
  {{128, 0, 34, 30, 12, 13}},
  {{162, 0, 30, 30, 15, 15}},
  {{193, 0, 28, 31, 14, 15}},
}};
constexpr std::array<std::array<std::int32_t, 7>, 103> kRecords = {{
  {{209, 205, 3, 4, 32768, 32768, 0}},
  {{209, 238, 4, 5, 32768, 32768, 0}},
  {{209, 271, 5, 6, 32768, 32768, 0}},
  {{242, 188, 7, 8, 0, 32768, 0}},
  {{242, 222, 8, 9, 0, 1, 0}},
  {{242, 254, 9, 10, 1, 2, 0}},
  {{242, 288, 10, 11, 2, 32768, 0}},
  {{275, 172, 12, 13, 3, 32768, 0}},
  {{275, 205, 13, 14, 3, 4, 0}},
  {{275, 238, 14, 15, 4, 5, 0}},
  {{275, 271, 15, 16, 5, 6, 0}},
  {{275, 304, 16, 17, 6, 32768, 0}},
  {{308, 156, 18, 19, 7, 32768, 0}},
  {{308, 188, 19, 20, 7, 8, 0}},
  {{308, 222, 20, 21, 8, 9, 0}},
  {{308, 254, 21, 22, 9, 10, 0}},
  {{308, 288, 22, 23, 10, 11, 0}},
  {{308, 320, 23, 24, 11, 32768, 0}},
  {{341, 139, 25, 26, 12, 32768, 0}},
  {{341, 172, 26, 27, 12, 13, 0}},
  {{341, 205, 27, 28, 13, 14, 0}},
  {{341, 238, 28, 29, 14, 15, 0}},
  {{341, 271, 29, 30, 15, 16, 0}},
  {{341, 304, 30, 31, 16, 17, 0}},
  {{341, 337, 31, 32, 17, 32768, 0}},
  {{374, 122, 33, 34, 18, 32768, 0}},
  {{374, 156, 34, 35, 18, 19, 0}},
  {{374, 188, 35, 36, 19, 20, 0}},
  {{374, 222, 36, 37, 20, 21, 0}},
  {{374, 254, 37, 38, 21, 22, 0}},
  {{374, 288, 38, 39, 22, 23, 0}},
  {{374, 320, 39, 40, 23, 24, 0}},
  {{374, 354, 40, 41, 24, 32768, 0}},
  {{407, 106, 42, 43, 25, 32768, 0}},
  {{407, 139, 43, 44, 25, 26, 0}},
  {{407, 172, 44, 45, 26, 27, 0}},
  {{407, 205, 45, 46, 27, 28, 0}},
  {{407, 238, 46, 47, 28, 29, 0}},
  {{407, 271, 47, 48, 29, 30, 0}},
  {{407, 304, 48, 49, 30, 31, 0}},
  {{407, 337, 49, 50, 31, 32, 0}},
  {{407, 370, 50, 51, 32, 32768, 0}},
  {{440, 90, 52, 53, 33, 32768, 0}},
  {{440, 122, 53, 54, 33, 34, 0}},
  {{440, 156, 54, 55, 34, 35, 0}},
  {{440, 188, 55, 56, 35, 36, 0}},
  {{440, 222, 56, 57, 36, 37, 0}},
  {{440, 254, 57, 58, 37, 38, 0}},
  {{440, 288, 58, 59, 38, 39, 0}},
  {{440, 320, 59, 60, 39, 40, 0}},
  {{440, 354, 60, 61, 40, 41, 0}},
  {{440, 386, 61, 62, 41, 32768, 0}},
  {{473, 73, 63, 64, 42, 32768, 0}},
  {{473, 106, 64, 65, 42, 43, 0}},
  {{473, 139, 65, 66, 43, 44, 0}},
  {{473, 172, 66, 67, 44, 45, 0}},
  {{473, 205, 67, 68, 45, 46, 0}},
  {{473, 238, 68, 69, 46, 47, 0}},
  {{473, 271, 69, 70, 47, 48, 0}},
  {{473, 304, 70, 71, 48, 49, 0}},
  {{473, 337, 71, 72, 49, 50, 0}},
  {{473, 370, 72, 73, 50, 51, 0}},
  {{473, 403, 73, 74, 51, 32768, 0}},
  {{506, 56, 75, 76, 52, 32768, 0}},
  {{506, 90, 76, 77, 52, 53, 0}},
  {{506, 122, 77, 78, 53, 54, 0}},
  {{506, 156, 78, 79, 54, 55, 0}},
  {{506, 188, 79, 80, 55, 56, 0}},
  {{506, 222, 80, 81, 56, 57, 0}},
  {{506, 254, 81, 82, 57, 58, 0}},
  {{506, 288, 82, 83, 58, 59, 0}},
  {{506, 320, 83, 84, 59, 60, 0}},
  {{506, 354, 84, 85, 60, 61, 0}},
  {{506, 386, 85, 86, 61, 62, 0}},
  {{506, 420, 86, 87, 62, 32768, 0}},
  {{539, 40, 88, 89, 63, 32768, 0}},
  {{539, 73, 89, 90, 63, 64, 0}},
  {{539, 106, 90, 91, 64, 65, 0}},
  {{539, 139, 91, 92, 65, 66, 0}},
  {{539, 172, 92, 93, 66, 67, 0}},
  {{539, 205, 93, 94, 67, 68, 0}},
  {{539, 238, 94, 95, 68, 69, 0}},
  {{539, 271, 95, 96, 69, 70, 0}},
  {{539, 304, 96, 97, 70, 71, 0}},
  {{539, 337, 97, 98, 71, 72, 0}},
  {{539, 370, 98, 99, 72, 73, 0}},
  {{539, 403, 99, 100, 73, 74, 0}},
  {{539, 436, 100, 101, 74, 32768, 0}},
  {{572, 24, 32768, 32768, 75, 32768, 0}},
  {{572, 56, 32768, 32768, 75, 76, 0}},
  {{572, 90, 32768, 32768, 76, 77, 0}},
  {{572, 122, 32768, 32768, 77, 78, 0}},
  {{572, 156, 32768, 32768, 78, 79, 0}},
  {{572, 188, 32768, 32768, 79, 80, 0}},
  {{572, 222, 32768, 32768, 80, 81, 0}},
  {{572, 254, 32768, 32768, 81, 82, 0}},
  {{572, 288, 32768, 32768, 82, 83, 0}},
  {{572, 320, 32768, 32768, 83, 84, 0}},
  {{572, 354, 32768, 32768, 84, 85, 0}},
  {{572, 386, 32768, 32768, 85, 86, 0}},
  {{572, 420, 32768, 32768, 86, 87, 0}},
  {{572, 452, 32768, 32768, 87, 32768, 0}},
  {{0, 0, 32768, 32768, 32768, 32768, 4}},
}};

std::expected<void, std::string> validateImage(const Image& image, const char* name) {
  if (image.width != 640 || image.height != 480 || image.rgba.size() != 640u*480u*4u)
    return std::unexpected(std::string(name)+" must decode to 640x480 RGBA");
  return {};
}
}

std::expected<std::array<std::int32_t, 102>, std::string> readMapFlags(const std::filesystem::path& path) {
  std::ifstream in(path); if (!in) return std::unexpected("cannot open flags file: "+path.string());
  std::array<std::int32_t,102> flags{};
  for (std::size_t i=0;i<flags.size();++i) {
    long long v; if (!(in>>v) || v<std::numeric_limits<std::int32_t>::min() || v>std::numeric_limits<std::int32_t>::max())
      return std::unexpected("flags file must contain exactly 102 signed 32-bit decimal integers");
    flags[i]=static_cast<std::int32_t>(v);
  }
  std::string extra; if (in>>extra) return std::unexpected("flags file has more than 102 values");
  if (!in.eof()) return std::unexpected("malformed flags file");
  return flags;
}

std::expected<MapScene, std::string> MapScene::build(const AssetLibrary& lib, const std::array<std::int32_t,102>& flags) {
  auto bg=loadTexture(lib,"TEXTURES\\LEVEL SELECT BACKGROUND.BMP");
  auto atlas=loadTexture(lib,"TEXTURES\\LEVEL SELECT OBJECTS.BMP");
  if(!bg) return std::unexpected("map background: "+bg.error());
  if(!atlas) return std::unexpected("map atlas: "+atlas.error());
  auto mask=decodeMapMask(lib);
  if(!mask) return std::unexpected(mask.error());
  auto levels=loadLevelTable(lib);
  if(!levels) return std::unexpected("map level table: "+levels.error());
  if(auto v=validateImage(*bg,"background");!v) return std::unexpected(v.error());
  if(auto v=validateImage(*atlas,"atlas");!v) return std::unexpected(v.error());
  MapScene scene; scene.image_=std::move(*bg);
  scene.mask_=std::move(mask->pixels); scene.maskBackground_=mask->background;
  std::array<std::int32_t,102> state{}; state.fill(1); state[0]=state[1]=state[2]=3;
  for(std::size_t i=0;i<102;++i) if(flags[i]!=0) {
    state[i]=2; for(int link: {kRecords[i][2],kRecords[i][3]})
      if(link!=kLinkEnd && link>=0 && link<102) state[static_cast<std::size_t>(link)]=3;
  }
  for(std::size_t i=0;i<102;++i) {
    const auto group=kMapping[i][0], slot=kMapping[i][1], base=kMapping[i][2];
    if(group<0 || slot<0 || static_cast<std::size_t>(group)>=levels->groups.size() ||
       static_cast<std::size_t>(slot)>=levels->groups[static_cast<std::size_t>(group)].size())
      return std::unexpected("map node mapping is outside the loaded level table");
    scene.levelIds_[i]=levels->groups[static_cast<std::size_t>(group)][static_cast<std::size_t>(slot)].id;
    const int row=base+(state[i]==2?2:state[i]==3?4:0);
    const auto& r=kRects[static_cast<std::size_t>(row)];
    const int dx=kRecords[i][0]-r[4], dy=kRecords[i][1]-r[5];
    (void)group; (void)slot;
    for(int y=0;y<r[3];++y) for(int x=0;x<r[2];++x) {
      const int sx=r[0]+x, sy=r[1]+y, tx=dx+x, ty=dy+y;
      if(sx<0||sy<0||sx>=640||sy>=480||tx<0||ty<0||tx>=640||ty>=480) continue;
      const auto si=(static_cast<std::size_t>(sy)*640+sx)*4, di=(static_cast<std::size_t>(ty)*640+tx)*4;
      const auto* src=atlas->rgba.data()+si; if(src[0]==0&&src[1]==0&&src[2]==0) continue;
      std::copy_n(src,4,scene.image_.rgba.data()+di);
    }
  }
  scene.states_=state; return scene;
}

std::optional<std::size_t> MapScene::pickNode(int x, int y) const {
  if (x<0 || x>=640 || y<0 || y>=480 || mask_.size()!=640u*480u) return std::nullopt;
  const auto index=mask_[static_cast<std::size_t>(y)*640+static_cast<std::size_t>(x)];
  if(index==maskBackground_ || index>=states_.size()) return std::nullopt;
  const auto node=static_cast<std::size_t>(index);
  if(states_[node]!=2 && states_[node]!=3) return std::nullopt;
  return node;
}

std::optional<std::string> MapScene::levelForNode(std::size_t node) const {
  if(node>=levelIds_.size() || levelIds_[node].empty() || levelIds_[node]=="blank") return std::nullopt;
  return levelIds_[node];
}
}  // namespace webo::app
