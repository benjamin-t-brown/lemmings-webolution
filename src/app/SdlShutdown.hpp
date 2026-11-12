// Window-leg SDL shutdown that leaves an open audio device alone.
//
// Each frontend/level leg ends its window with SDL_Quit(), which also force-
// stops the audio subsystem and would close the one Mixer the application owns
// across legs. When audio is up, stop only the video and event subsystems the
// leg started; otherwise keep the previous SDL_Quit() exactly.

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
