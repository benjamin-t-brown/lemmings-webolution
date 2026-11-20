// Ignore only the union of actual Climber sprite texels when comparing UI animation modes.
// Plate, stock, HUD and scene pixels must match exactly.
#pragma once
#include "app/SkillPanel.h"
#include "assets/Box.h"
#include <stdexcept>

namespace webo::test {
inline std::vector<std::size_t> climberIconComparisonPixels(const AssetLibrary& library) {
  auto sheet = loadSpriteSheet(library, "SPRITES\\LEMMINGSHI.SPR");
  if (!sheet || !sheet->find("climb")) throw std::runtime_error("Climber comparison sprite missing");
  Image mask; mask.width=640; mask.height=480; mask.rgba.resize(640*480*4);
  for (const auto& entry : sheet->find("climb")->animations) {
    if (entry.frames.size() <= 2) throw std::runtime_error("Climber facing missing");
    const auto& frame=entry.frames[2];
    auto texture=loadTexture(library,sheetAssetName(frame.sheet));
    if (!texture) throw std::runtime_error(texture.error());
    for (std::size_t i=0;i<texture->rgba.size();i+=4)
      texture->rgba[i]=texture->rgba[i+1]=texture->rgba[i+2]=255;
    app::drawSkillPanelIcon(mask,*texture,frame,39,40,-1);
  }
  std::vector<std::size_t> pixels;
  for (std::size_t i=0;i<mask.rgba.size();i+=4) if(mask.rgba[i]) pixels.push_back(i);
  if (pixels.empty()) throw std::runtime_error("empty Climber comparison footprint");
  return pixels;
}
inline void normalizeClimberIcon(Image& image,const std::vector<std::size_t>& pixels) {
  for (auto at:pixels) for(int c=0;c<4;++c) image.rgba.at(at+c)=0;
}
}
