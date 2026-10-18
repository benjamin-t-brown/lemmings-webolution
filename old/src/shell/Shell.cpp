#include "Shell.h"

#include "audio/Mixer.h"

#include <SDL2/SDL_keycode.h>

#include <algorithm>
#include <cmath>
#include <format>

namespace webo::shell {
namespace {

constexpr int kMainCount = 5;
constexpr std::size_t kMainItems[kMainCount] = {
    Ltx::StartNew, Ltx::PlayCurrent, Ltx::Options, Ltx::LoadGame, Ltx::Quit,
};

gfx::Colour dim{0.55f, 0.55f, 0.6f, 1};
gfx::Colour bright{1, 1, 0.75f, 1};
gfx::Colour locked{0.35f, 0.35f, 0.4f, 1};
gfx::Colour ok{0.45f, 0.95f, 0.55f, 1};

}  // namespace

void Shell::setLevelTable(LevelTable table) {
  table_ = std::move(table);
  if (progress_.slots.empty())
    progress_.newGame(table_);
  else
    progress_.resizeFor(table_);
}

void Shell::setProgress(Progress progress) {
  progress_ = std::move(progress);
  hasSave_ = true;
  if (!table_.groups.empty())
    progress_.resizeFor(table_);
}

void Shell::persist() const {
  (void)saveProgress(progress_, progressPath_);
}

void Shell::applyVolumes(audio::Mixer& mixer) const {
  mixer.setMusicVolume(progress_.musicVolume);
  mixer.setSfxVolume(progress_.sfxVolume);
}

const LevelInfo* Shell::infoForId(std::string_view id) const {
  for (const auto& i : infos_)
    if (i.id == id)
      return &i;
  return nullptr;
}

void Shell::showResults(ResultsInfo info) {
  results_ = info;
  screen_ = Screen::Results;
  highlight_ = 0;
  if (info.won)
    persist();
}

void Shell::returnToMap() {
  screen_ = Screen::Map;
  highlight_ = 0;
}

void Shell::returnToMain() {
  screen_ = Screen::Main;
  highlight_ = 0;
}

void Shell::activateMain() {
  switch (highlight_) {
    case 0:  // Start New Game
      screen_ = Screen::ConfirmNew;
      highlight_ = 0;
      break;
    case 1:  // Play Current Game
      if (!hasSave_) {
        progress_.newGame(table_);
        hasSave_ = true;
        persist();
      }
      screen_ = Screen::Map;
      mapGroup_ = 0;
      mapSlot_ = std::max(0, firstPlayableSlot(table_, 0));
      highlight_ = 0;
      break;
    case 2:
      screen_ = Screen::Options;
      optionsRow_ = 0;
      highlight_ = 0;
      break;
    case 3:  // Load Game
      if (auto loaded = loadProgress(progressPath_)) {
        progress_ = std::move(*loaded);
        hasSave_ = true;
        screen_ = Screen::Map;
        mapGroup_ = 0;
        mapSlot_ = std::max(0, firstPlayableSlot(table_, 0));
      }
      break;
    case 4:
      quit_ = true;
      break;
    default:
      break;
  }
}

void Shell::activateConfirm(bool yes) {
  if (yes) {
    progress_.newGame(table_);
    hasSave_ = true;
    persist();
    screen_ = Screen::Map;
    mapGroup_ = 0;
    mapSlot_ = std::max(0, firstPlayableSlot(table_, 0));
  } else {
    screen_ = Screen::Main;
    highlight_ = 0;
  }
}

void Shell::activateOptions() {
  switch (optionsRow_) {
    case 0: {  // language cycle
      if (languages_.empty())
        break;
      auto it = std::find(languages_.begin(), languages_.end(), progress_.language);
      if (it == languages_.end() || ++it == languages_.end())
        progress_.language = languages_.front();
      else
        progress_.language = *it;
      languageDirty_ = true;
      persist();
      break;
    }
    case 1:
      progress_.musicVolume = progress_.musicVolume >= 0.95f ? 0.0f
                                                            : std::min(1.0f, progress_.musicVolume + 0.2f);
      persist();
      break;
    case 2:
      progress_.sfxVolume = progress_.sfxVolume >= 0.95f ? 0.0f
                                                        : std::min(1.0f, progress_.sfxVolume + 0.2f);
      persist();
      break;
    case 3:
      persist();
      break;
    case 4:
      screen_ = Screen::Main;
      highlight_ = 0;
      break;
    default:
      break;
  }
}

std::optional<PlayRequest> Shell::activateMapSelect() {
  if (!progress_.canPlay(mapGroup_, mapSlot_))
    return std::nullopt;
  if (mapGroup_ < 0 || mapGroup_ >= static_cast<int>(table_.groups.size()))
    return std::nullopt;
  const auto& row = table_.groups[static_cast<std::size_t>(mapGroup_)];
  if (mapSlot_ < 0 || mapSlot_ >= static_cast<int>(row.size()))
    return std::nullopt;
  PlayRequest req;
  req.levelId = row[static_cast<std::size_t>(mapSlot_)].id;
  req.group = mapGroup_;
  req.slot = mapSlot_;
  if (req.levelId.empty() || req.levelId == "blank")
    return std::nullopt;
  screen_ = Screen::Playing;
  return req;
}

std::optional<PlayRequest> Shell::handleKey(int sdlKey) {
  if (screen_ == Screen::Playing)
    return std::nullopt;

  const bool up = sdlKey == SDLK_UP;
  const bool down = sdlKey == SDLK_DOWN;
  const bool left = sdlKey == SDLK_LEFT;
  const bool right = sdlKey == SDLK_RIGHT;
  const bool enter = sdlKey == SDLK_RETURN || sdlKey == SDLK_KP_ENTER;
  const bool esc = sdlKey == SDLK_ESCAPE;

  if (screen_ == Screen::Main) {
    if (up)
      highlight_ = (highlight_ + kMainCount - 1) % kMainCount;
    else if (down)
      highlight_ = (highlight_ + 1) % kMainCount;
    else if (enter)
      activateMain();
    else if (esc) {
      highlight_ = 4;
      quit_ = true;
    }
    return std::nullopt;
  }

  if (screen_ == Screen::ConfirmNew) {
    if (up || down || left || right)
      highlight_ = 1 - highlight_;
    else if (enter)
      activateConfirm(highlight_ == 0);
    else if (esc)
      activateConfirm(false);
    return std::nullopt;
  }

  if (screen_ == Screen::Options) {
    constexpr int kRows = 5;
    if (up)
      optionsRow_ = (optionsRow_ + kRows - 1) % kRows;
    else if (down)
      optionsRow_ = (optionsRow_ + 1) % kRows;
    else if (enter || right)
      activateOptions();
    else if (esc) {
      screen_ = Screen::Main;
      highlight_ = 0;
    }
    return std::nullopt;
  }

  if (screen_ == Screen::Map) {
    if (esc) {
      screen_ = Screen::Main;
      highlight_ = 0;
      return std::nullopt;
    }
    if (left)
      mapGroup_ = (mapGroup_ + Progress::kGroups - 1) % Progress::kGroups;
    if (right)
      mapGroup_ = (mapGroup_ + 1) % Progress::kGroups;
    if (up || down) {
      const int dir = up ? -1 : 1;
      if (mapGroup_ < static_cast<int>(table_.groups.size())) {
        const int n = static_cast<int>(table_.groups[static_cast<std::size_t>(mapGroup_)].size());
        for (int tries = 0; tries < n; ++tries) {
          mapSlot_ = (mapSlot_ + dir + n) % n;
          const auto& id =
              table_.groups[static_cast<std::size_t>(mapGroup_)][static_cast<std::size_t>(mapSlot_)]
                  .id;
          if (!id.empty() && id != "blank")
            break;
        }
      }
    }
    if (enter)
      return activateMapSelect();
    return std::nullopt;
  }

  if (screen_ == Screen::Results) {
    if (up || down)
      highlight_ = 1 - highlight_;
    else if (enter) {
      if (highlight_ == 0)
        returnToMap();
      else
        returnToMain();
    } else if (esc)
      returnToMap();
    return std::nullopt;
  }

  return std::nullopt;
}

void Shell::draw(gfx::Renderer& r, const gfx::Font& font, unsigned fontTex, int vw,
                 int vh) const {
  r.beginScreenUi(vw, vh);
  r.drawQuad(0, 0, static_cast<float>(vw), static_cast<float>(vh), {0.05f, 0.06f, 0.1f, 1});

  const float scale = vw >= 900 ? 3.0f : 2.0f;
  const float line = gfx::Font::kGlyph * scale + 6;
  float y = 48;

  auto title = [&](std::string_view t) {
    const int tw = font.measure(t, scale);
    r.drawText(font, fontTex, static_cast<float>(vw - tw) * 0.5f, y, scale, t, bright);
    y += line * 1.5f;
  };

  auto row = [&](std::string_view t, bool hi, gfx::Colour c) {
    if (hi)
      r.drawQuad(40, y - 2, static_cast<float>(vw) - 80, line, {0.15f, 0.25f, 0.45f, 0.9f});
    r.drawText(font, fontTex, 56, y, scale, t, hi ? bright : c);
    y += line;
  };

  if (screen_ == Screen::Main) {
    title("Lemmings Revolution");
    y += line * 0.5f;
    for (int i = 0; i < kMainCount; ++i)
      row(text(kMainItems[static_cast<std::size_t>(i)]), i == highlight_, dim);
  } else if (screen_ == Screen::ConfirmNew) {
    title(text(Ltx::StartNew));
    y += line;
    row(text(Ltx::Yes), highlight_ == 0, dim);
    row(text(Ltx::No), highlight_ == 1, dim);
  } else if (screen_ == Screen::Options) {
    title(text(Ltx::Options));
    const std::string langLine =
        std::format("{}: {}", text(Ltx::LanguageSel), progress_.language);
    const std::string musLine =
        std::format("{}: {}%", text(Ltx::Music), static_cast<int>(progress_.musicVolume * 100));
    const std::string sfxLine =
        std::format("{}: {}%", text(Ltx::Sfx), static_cast<int>(progress_.sfxVolume * 100));
    row(langLine, optionsRow_ == 0, dim);
    row(musLine, optionsRow_ == 1, dim);
    row(sfxLine, optionsRow_ == 2, dim);
    row(text(Ltx::SaveGame), optionsRow_ == 3, dim);
    row(text(Ltx::BackMain), optionsRow_ == 4, dim);
  } else if (screen_ == Screen::Map) {
    const std::size_t diffIdx =
        Ltx::DiffEasy + static_cast<std::size_t>(std::clamp(mapGroup_, 0, 4));
    // Easy/Medium/Hard/Nightmare/Very Hard — retail uses 32.. and 45 Special.
    std::string_view groupName = text(diffIdx);
    if (mapGroup_ == 4)
      groupName = text(45);  // Very Hard / Special band
    title(groupName);
    r.drawText(font, fontTex, 56, y, scale * 0.75f,
               std::format("{} / {}  (Left/Right group, Enter play, Esc back)", mapGroup_ + 1,
                           Progress::kGroups),
               dim);
    y += line;

    if (mapGroup_ < static_cast<int>(table_.groups.size())) {
      const auto& g = table_.groups[static_cast<std::size_t>(mapGroup_)];
      const int maxShow = std::min(static_cast<int>(g.size()), 12);
      int shown = 0;
      // Centre list around mapSlot_.
      int start = std::max(0, mapSlot_ - 5);
      for (int s = start; s < static_cast<int>(g.size()) && shown < maxShow; ++s) {
        const auto& id = g[static_cast<std::size_t>(s)].id;
        if (id.empty() || id == "blank")
          continue;
        const LevelInfo* info = infoForId(id);
        const std::string label =
            info ? std::format("{}. {}", s + 1, info->name) : std::format("{}. {}", s + 1, id);
        const SlotState st = progress_.state(mapGroup_, s);
        gfx::Colour c = locked;
        if (st == SlotState::Unlocked)
          c = dim;
        if (st == SlotState::Completed)
          c = ok;
        row(label, s == mapSlot_, c);
        ++shown;
      }
    }
  } else if (screen_ == Screen::Results) {
    title(results_.won ? text(Ltx::Stormed) : text(Ltx::Failed));
    r.drawText(font, fontTex, 56, y, scale,
               std::format("{}: {} / {}", text(Ltx::Saved), results_.saved, results_.saveTarget),
               dim);
    y += line;
    r.drawText(font, fontTex, 56, y, scale, std::format("{}: {}", text(Ltx::Out), results_.out),
               dim);
    y += line;
    r.drawText(font, fontTex, 56, y, scale,
               std::format("{}: {}s", text(Ltx::TimeRemaining), results_.timeLeftSec), dim);
    y += line * 1.5f;
    row(text(Ltx::Continue), highlight_ == 0, dim);
    row(text(Ltx::BackMain), highlight_ == 1, dim);
  }
}

}  // namespace webo::shell
