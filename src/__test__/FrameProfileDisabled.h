// Benchmark control that compiles out frame timer scopes.
#pragma once
#include <chrono>
#include <cstddef>
namespace webo::app::profile {
using Clock = std::chrono::steady_clock;
enum Cost { Other, Simulation, Preparation, TextureDecode, BackdropRaster,
            TubeRaster, Actors, FluidRaster, Hud, Upload, Copy, Present, Idle, BodyFootprint, BuilderBrick, Particles,
            GlScene, GlCompose, GlWindowCopy, GlSwap, GlReadback, GlQuery, Count };
struct State { Cost cost = Other; Cost sceneRaster = BackdropRaster; };
inline constexpr State* active = nullptr;
inline void glDraw() {}
inline void glUpload(std::size_t) {}
struct Scope { explicit Scope(Cost) {} void stop() {} };
struct ScenePass { explicit ScenePass(Cost) {} };
struct Run { struct Frame { Frame(Run&, const char*, int, std::size_t) {} }; };
}
