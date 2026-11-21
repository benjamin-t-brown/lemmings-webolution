// Presentation SDL shutdown that leaves an open audio device alone.
//
// The connected application owns one window across legs; standalone adapters
// own a local presentation. At either owner's exit, stop video/events without
// force-closing the separately owned Mixer. Without audio, SDL_Quit is enough.

#pragma once

#if defined(WEBOLUTION_HAVE_SDL2) && WEBOLUTION_HAVE_SDL2
#include <SDL.h>

namespace webo::app {

inline void quitWindowSubsystems() {
  if (SDL_WasInit(SDL_INIT_AUDIO) != 0)
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  else
    SDL_Quit();
}

}  // namespace webo::app
#endif
