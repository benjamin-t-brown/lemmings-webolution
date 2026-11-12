#pragma once

#include "hook_config.hpp"

namespace lrhook {

bool install_collision_grid_hook(Mode mode);
void report_collision_grid_hook();

// Sweeps the recovered routine against the original bytes reached through the
// installed trampoline, inside the running game process. Unlike the offline
// oracles this needs no address-mapping helper: the original code is already
// loaded at its recorded address. Returns true when every case matched.
bool selftest_collision_grid();

bool install_cylinder_coordinate_hook(Mode mode);
void report_cylinder_coordinate_hook();
bool selftest_cylinder_coordinate();

// Binds lemmings::set_active_allocator to the game's own CRT as a side effect,
// so install this before any other hook whose routine allocates. Installation
// itself allocates nothing and is safe from DllMain.
bool install_native_int_array_hook(Mode mode);
void report_native_int_array_hook();

// MUST NOT be called from DllMain. patch.dll is a static import, so DllMain
// runs before the executable's CRT is initialised; allocating through the
// game's `operator new` at that point enters an uninitialised heap and the
// process dies with STATUS_DLL_INIT_FAILED (0xC0000142). Call this once the
// game is running -- the harness uses the first DirectDrawCreate.
//
// This also arms the hook's comparison path: until it runs, the hook forwards
// straight to the original rather than taking a heap copy.
bool selftest_native_int_array();

} // namespace lrhook
