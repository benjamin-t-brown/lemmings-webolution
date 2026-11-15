#include "assets/Box.h"
#include "assets/Bitmap.h"
#include "assets/TextureCache.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace webo;
void check(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
int main() {
  try {
    const std::filesystem::path game(std::getenv("WEBOLUTION_GAME_DIR"));
    auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
    auto other = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
    check(bool(lib) && bool(other), "open installed asset overlays");
    std::string name;
    for (const auto& n : lib->names())
      if (n.ends_with(".BMP") && n.find('^') != std::string::npos) { name = n; break; }
    check(!name.empty(), "installed alpha-plane texture exists");
    auto reference = loadTexture(*lib, name);
    check(bool(reference), "uncached reference decodes");
    {
      TextureCacheScope cache(*lib);
      auto first = loadTexture(*lib, name);
      check(bool(first) && first->rgba == reference->rgba, "first read matches uncached alpha-plane pixels");
      first->rgba.assign(first->rgba.size(), 17);
      auto second = loadTexture(*lib, name);
      check(bool(second) && second->rgba == reference->rgba && second->alpha == reference->alpha,
            "caller changes cannot mutate cached pixels or alpha mode");
      check(cache.images.size() == 1 && cache.hits == 1, "second decode reused");
      auto separate = loadTexture(*other, name);
      check(bool(separate) && separate->rgba == reference->rgba && cache.hits == 1,
            "a different library cannot hit this scope");
      check(!loadTexture(*lib, "TEXTURES\\NO_SUCH_TEXTURE_FRAME_PROFILE.BMP"), "missing read still fails");
      check(cache.images.size() == 1, "failure is not cached");
      {
        TextureCacheScope nested(*other);
        auto image = loadTexture(*other, name);
        check(bool(image) && nested.images.size() == 1, "nested scope owns its overlay");
      }
      check(activeTextureCache == &cache, "nested scope restores its owner");
      auto third = loadTexture(*lib, name);
      check(bool(third) && third->rgba == reference->rgba && cache.hits == 2, "outer cache survives nesting");
    }
    check(activeTextureCache == nullptr, "scope exit retires cached images");
    std::cout << "texture cache ownership/alpha/isolation checks passed: " << name << '\n';
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
