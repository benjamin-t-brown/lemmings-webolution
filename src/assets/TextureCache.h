// Decoded textures are immutable within one application leg. The scope is
// bound to one live archive library, so cache entries cannot survive a reload,
// retry or a different patch overlay. Callers still receive independent Images.
#pragma once
#include "assets/Bitmap.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

namespace webo {
class TextureCacheScope;
inline thread_local TextureCacheScope* activeTextureCache = nullptr;
class TextureCacheScope {
 public:
  explicit TextureCacheScope(const AssetLibrary& library)
      : library(&library), prior_(activeTextureCache) { activeTextureCache = this; }
  ~TextureCacheScope() {
    activeTextureCache = prior_;
    if (const char* profile = std::getenv("WEBOLUTION_FRAME_PROFILE"); profile && *profile) {
      std::size_t bytes = 0;
      for (const auto& [name, image] : images) { (void)name; bytes += image.rgba.size(); }
      std::fprintf(stderr, "texture-cache: entries=%zu decoded-bytes=%zu hits=%zu\n",
                   images.size(), bytes, hits);
    }
  }
  TextureCacheScope(const TextureCacheScope&) = delete;
  TextureCacheScope& operator=(const TextureCacheScope&) = delete;
  const AssetLibrary* library;
  std::unordered_map<std::string, Image> images;
  std::size_t hits = 0;
 private:
  TextureCacheScope* prior_;
};
} // namespace webo
