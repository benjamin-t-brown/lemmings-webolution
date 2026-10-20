#include "app/ActorSprite.h"
#include "app/Camera.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/LightMap.h"
#include "assets/Png.h"
#include "assets/Sprite.h"

#include <format>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

// A4f actor CPU candidate fixture. Geometry is bounded against native data;
// tint comes from the level light map unless overridden with --tint.
int main(int argc, char** argv) {
  std::map<std::string, std::string> args;
  for (int i = 1; i < argc; ++i) {
    const std::string key = argv[i];
    if (!key.starts_with("--")) {
      std::cerr << "unexpected argument: " << key << '\n';
      return 2;
    }
    const bool nextIsFlag =
        i + 1 < argc && std::string_view{argv[i + 1]}.starts_with("--");
    if (i + 1 < argc && !nextIsFlag)
      args[key] = argv[++i];
    else
      args[key] = "1";
  }
  const auto get = [&](const char* key, const char* fallback = "") -> std::string {
    auto it = args.find(key);
    return it == args.end() ? fallback : it->second;
  };
  if (get("--game-dir").empty() || get("--out").empty()) {
    std::cerr << "usage: actorfixture --game-dir DIR --out PNG --action-index N "
                 "--temporal N --direction N --mirror 0|1 --x X --y Y --z Z "
                 "[--level COMPILED\\\\MM_CLIMB_002.LVL] [--tint R,G,B] [--no-light-lookup]\n";
    return 2;
  }
  try {
    const std::filesystem::path dir = get("--game-dir");
    auto library = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
    if (!library)
      throw std::runtime_error(library.error());
    auto resources = webo::loadSpriteSheet(*library, "SPRITES\\LEMMINGSHI.SPR");
    if (!resources)
      throw std::runtime_error(resources.error());
    const std::size_t actionIndex = std::stoul(get("--action-index", "13"));
    const std::size_t temporal = std::stoul(get("--temporal", "8"));
    const std::size_t direction = std::stoul(get("--direction", "1"));
    if (actionIndex >= resources->sprites.size())
      throw std::runtime_error("action index outside LEMMINGSHI.SPR");
    const auto& action = resources->sprites[actionIndex];
    if (temporal >= action.animations.size() ||
        direction >= action.animations[temporal].frames.size())
      throw std::runtime_error("temporal/direction outside selected SPR action");
    const auto& frame = action.animations[temporal].frames[direction];
    auto texture = webo::loadTexture(*library, webo::sheetAssetName(frame.sheet));
    if (!texture)
      throw std::runtime_error(texture.error());

    webo::app::CameraParams cameraParams;
    cameraParams.width = 640;
    cameraParams.height = 480;
    cameraParams.aspect = 4.0 / 3.0;
    const auto camera = webo::app::buildCamera(cameraParams);
    const std::array<double, 3> position{
        std::stod(get("--x")), std::stod(get("--y")), std::stod(get("--z"))};
    const bool mirror = get("--mirror", "1") == "1";
    const int scaleDelta = std::stoi(get("--scale-delta", "-1"));
    auto placement = webo::app::actorSpritePlacement(frame, position, camera, mirror, scaleDelta);
    if (!placement)
      throw std::runtime_error(placement.error());

    std::array<std::uint8_t, 3> vertexRgb{255, 255, 255};
    int mapIndex = -1;
    std::uint8_t mapByte = 0;
    std::string tintSource = "default_white";
    if (!get("--tint").empty()) {
      std::stringstream tintStream(get("--tint"));
      std::string part;
      int channel = 0;
      while (std::getline(tintStream, part, ',') && channel < 3)
        vertexRgb[static_cast<std::size_t>(channel++)] =
            static_cast<std::uint8_t>(std::stoi(part));
      tintSource = "cli_tint";
    } else if (args.find("--no-light-lookup") == args.end()) {
      const std::string levelName =
          get("--level", "COMPILED\\MM_CLIMB_002.LVL");
      auto light = webo::loadActorLightMap(*library, levelName);
      if (!light)
        throw std::runtime_error(light.error());
      mapIndex = webo::actorLightMapIndex(position[0], position[1]);
      if (mapIndex < 0 || static_cast<std::size_t>(mapIndex) >= light->map.size())
        throw std::runtime_error(std::format("light map index {} out of range", mapIndex));
      mapByte = light->map[static_cast<std::size_t>(mapIndex)];
      auto tint = webo::actorLightTintAt(*light, position[0], position[1]);
      if (!tint)
        throw std::runtime_error(tint.error());
      vertexRgb = *tint;
      tintSource = "level_light_map";
    } else {
      tintSource = "no_light_lookup";
    }

    webo::Image overlay;
    overlay.width = 640;
    overlay.height = 480;
    overlay.alpha = webo::AlphaMode::Embedded;
    overlay.rgba.assign(static_cast<std::size_t>(overlay.width) * overlay.height * 4, 0);
    if (auto rendered = webo::app::drawActorSprite(overlay, frame, *texture, *placement, mirror,
                                                   vertexRgb);
        !rendered)
      throw std::runtime_error(rendered.error());
    if (auto saved = webo::writePng(get("--out"), overlay); !saved)
      throw std::runtime_error(saved.error());

    std::cout << std::format(
        "{{\"candidate\":\"actor-sprite-cpu\",\"tint_source\":\"{}\","
        "\"action_index\":{},\"action\":\"{}\",\"temporal\":{},\"direction\":{},"
        "\"mirror\":{},\"frame\":[{},{},{},{}],\"unknown\":[{},{}],"
        "\"uv\":[{:.17g},{:.17g},{:.17g},{:.17g}],\"position\":[{:.17g},{:.17g},{:.17g}],"
        "\"origin\":[{:.17g},{:.17g}],\"rect\":[{:.17g},{:.17g},{:.17g},{:.17g}],"
        "\"scale_delta\":{},\"alpha\":\"{}\",\"vertex_tint\":[{}, {}, {}],"
        "\"light_map_index\":{},\"light_map_byte\":{}}}\n",
        tintSource, actionIndex, action.name, temporal, direction, mirror ? "true" : "false",
        frame.left, frame.top, frame.right, frame.bottom, frame.unknown0, frame.unknown1,
        frame.uLeft, frame.uTop, frame.uRight, frame.uBottom, position[0], position[1],
        position[2], placement->originX, placement->originY, placement->left, placement->top,
        placement->right, placement->bottom, scaleDelta, webo::alphaModeName(texture->alpha),
        vertexRgb[0], vertexRgb[1], vertexRgb[2], mapIndex, mapByte);
  } catch (const std::exception& e) {
    std::cerr << "actorfixture: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
