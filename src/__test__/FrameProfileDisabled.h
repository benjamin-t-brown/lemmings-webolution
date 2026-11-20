// Benchmark control that compiles out frame timer scopes.
#pragma once
#include <cstddef>
namespace webo::app::profile {
enum Cost { Other, Simulation, Preparation, TextureDecode, BackdropRaster,
            TubeRaster, Actors, FluidRaster, Hud, Upload, Copy, Present, Idle, BodyFootprint, BuilderBrick, Particles, Count };
struct State { Cost cost = Other; Cost sceneRaster = BackdropRaster; };
inline constexpr State* active = nullptr;
struct Scope { explicit Scope(Cost) {} void stop() {} };
struct ScenePass { explicit ScenePass(Cost) {} };
struct Run { struct Frame { Frame(Run&, const char*, int, std::size_t) {} }; };
}
