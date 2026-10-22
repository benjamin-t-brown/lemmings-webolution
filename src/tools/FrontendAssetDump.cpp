#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "assets/Png.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
std::string json(std::string_view s) {
  std::string out = "\"";
  for (unsigned char c : s) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: if (c < 0x20) out += "?"; else out += static_cast<char>(c);
    }
  }
  return out + '"';
}

std::string safeFile(std::string name) {
  std::string out;
  for (unsigned char c : name) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '-')
      out += static_cast<char>(c);
    else {
      constexpr char hex[] = "0123456789ABCDEF";
      out += '~'; out += hex[c >> 4]; out += hex[c & 15];
    }
  }
  return out;
}
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "usage: FrontendAssetDump <game-dir> <out-dir>\n";
    return 2;
  }
  const fs::path game = argv[1], out = argv[2];
  if (!fs::is_directory(game) || !fs::is_regular_file(game / "lemmings.box")) {
    std::cerr << "invalid game directory (expected lemmings.box): " << game.string() << '\n';
    return 2;
  }
  auto lib = webo::AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) { std::cerr << "cannot open asset archives: " << lib.error() << '\n'; return 2; }
  std::error_code ec;
  fs::create_directories(out, ec);
  if (ec) { std::cerr << "cannot create output directory: " << ec.message() << '\n'; return 2; }

  std::vector<std::string> candidates = {
    "TEXTURES\\MAIN_LOGO.BMP", "TEXTURES\\_FONT.BMP", "TEXTURES\\_FONTFE.BMP", "TEXTURES\\_FONTSMALL.BMP"
  };
  for (const auto& name : lib->names()) {
    const auto slash = name.find_last_of("\\/");
    const auto base = name.substr(slash == std::string::npos ? 0 : slash + 1);
    if (!base.empty() && base.front() == '^' && base.find("FONT") != std::string::npos && base.ends_with(".BMP"))
      candidates.push_back(name);
  }
  std::sort(candidates.begin(), candidates.end());
  candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

  std::ofstream manifest(out / "manifest.json", std::ios::binary);
  if (!manifest) { std::cerr << "cannot write manifest\n"; return 2; }
  manifest << "{\n  \"assets\": [\n";
  bool first = true;
  for (const auto& name : candidates) {
    if (!first) manifest << ",\n";
    first = false;
    manifest << "    {\"archive_name\":" << json(name);
    if (!lib->contains(name)) { manifest << ",\"status\":\"missing\"}"; continue; }
    auto image = webo::loadTexture(*lib, name);
    if (!image) { manifest << ",\"status\":\"decode_error\",\"error\":" << json(image.error()) << "}"; continue; }
    const auto file = safeFile(name) + ".png";
    auto written = webo::writePng(out / file, *image);
    if (!written) { std::cerr << "PNG write failed for " << name << ": " << written.error() << '\n'; return 2; }
    manifest << ",\"status\":\"exported\",\"width\":" << image->width
             << ",\"height\":" << image->height << ",\"alpha\":" << json(std::string(webo::alphaModeName(image->alpha)))
             << ",\"output\":" << json(file) << "}";
    std::cout << name << " -> " << file << " " << image->width << 'x' << image->height
              << " alpha=" << webo::alphaModeName(image->alpha) << '\n';
  }
  manifest << "\n  ],\n  \"strings_language\": \"UK\",\n  \"strings_file\": \"LANGUAGE\\\\UK.LTX\",\n  \"strings_output\": \"UK.strings.json\"\n}\n";
  auto strings = webo::loadStrings(*lib, "UK");
  if (!strings) { std::cerr << "cannot load UK strings: " << strings.error() << '\n'; return 2; }
  std::ofstream text(out / "UK.strings.json", std::ios::binary);
  if (!text) { std::cerr << "cannot write UK strings\n"; return 2; }
  text << "[\n";
  for (std::size_t i = 0; i < strings->size(); ++i)
    text << "  {\"index\":" << i << ",\"text\":" << json((*strings)[i]) << "}" << (i + 1 == strings->size() ? "\n" : ",\n");
  text << "]\n";
  std::cout << "UK strings: " << strings->size() << "\n";
  return 0;
}
