#include "app/WindowsWindowIcon.h"

#include <SDL.h>
#include <windows.h>

#include <cstring>

#include "app/AppIconResource.h"

namespace webo::app {

void setWindowsWindowIcon(SDL_Window* window) {
  constexpr int size = 32;
  constexpr int rowBytes = size * 4;
  const auto module = GetModuleHandle(nullptr);
  const auto group = FindResource(module, MAKEINTRESOURCE(WEBOLUTION_APP_ICON_ID), RT_GROUP_ICON);
  if (!group) return;
  const auto groupData = static_cast<PBYTE>(LockResource(LoadResource(module, group)));
  if (!groupData) return;
  const int id = LookupIconIdFromDirectoryEx(groupData, TRUE, size, size, LR_DEFAULTCOLOR);
  if (!id) return;
  const auto frame = FindResource(module, MAKEINTRESOURCE(id), RT_ICON);
  if (!frame || SizeofResource(module, frame) < sizeof(BITMAPINFOHEADER) + size * rowBytes) return;
  const auto data = static_cast<const Uint8*>(LockResource(LoadResource(module, frame)));
  if (!data) return;

  BITMAPINFOHEADER header{};
  std::memcpy(&header, data, sizeof(header));
  if (header.biSize != sizeof(header) || header.biWidth != size || header.biHeight != size * 2 ||
      header.biPlanes != 1 || header.biBitCount != 32 || header.biCompression != BI_RGB)
    return;

  // The embedded frame stores BGRA rows from bottom to top, then an AND mask.
  const auto bitmap = data + sizeof(header);
  auto* surface = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_BGRA32);
  if (!surface) return;
  auto* pixels = static_cast<Uint8*>(surface->pixels);
  for (int y = 0; y < size; ++y)
    std::memcpy(pixels + y * surface->pitch, bitmap + (size - 1 - y) * rowBytes, rowBytes);
  SDL_SetWindowIcon(window, surface);
  SDL_FreeSurface(surface);
}

}  // namespace webo::app
