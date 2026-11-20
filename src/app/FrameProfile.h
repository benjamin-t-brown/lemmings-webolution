// Set WEBOLUTION_FRAME_PROFILE to a CSV path for exclusive wall-clock
// timing. CSV writes occur outside the timed frame.
#pragma once
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstddef>

namespace webo::app::profile {
using Clock = std::chrono::steady_clock;
enum Cost { Other, Simulation, Preparation, TextureDecode, BackdropRaster,
            TubeRaster, Actors, FluidRaster, Hud, Upload, Copy, Present, Idle,
            BodyFootprint, BuilderBrick, Particles, Count };
struct State {
  std::array<double, Count> ms{};
  Cost cost = Other;
  Cost sceneRaster = BackdropRaster;
  Clock::time_point cursor;
  void charge(Clock::time_point now) {
    ms[cost] += std::chrono::duration<double, std::milli>(now - cursor).count();
    cursor = now;
  }
};
inline thread_local State* active = nullptr;
class Scope {
 public:
  explicit Scope(Cost cost) : state_(active) {
    if (state_) { state_->charge(Clock::now()); prior_ = state_->cost; state_->cost = cost; }
  }
  void stop() {
    if (state_) { state_->charge(Clock::now()); state_->cost = prior_; state_ = nullptr; }
  }
  ~Scope() { stop(); }
 private:
  State* state_;
  Cost prior_ = Other;
};
class ScenePass {
 public:
  explicit ScenePass(Cost cost) : state_(active) {
    if (state_) { prior_ = state_->sceneRaster; state_->sceneRaster = cost; }
  }
  ~ScenePass() { if (state_) state_->sceneRaster = prior_; }
 private:
  State* state_;
  Cost prior_ = BackdropRaster;
};
class Run {
 public:
  Run() {
    if (const char* path = std::getenv("WEBOLUTION_FRAME_PROFILE"); path && *path) {
      file_ = std::fopen(path, "a+");
      if (!file_) { std::perror("frame profile"); return; }
      std::fseek(file_, 0, SEEK_END);
      if (std::ftell(file_) == 0) std::fprintf(file_,
        "level,frame,alive,total_ms,other_ms,simulation_ms,preparation_ms,texture_decode_ms,backdrop_raster_ms,tube_raster_ms,actors_ms,fluid_raster_ms,hud_ms,upload_ms,copy_ms,present_ms,idle_ms,body_footprint_ms,builder_brick_ms,particles_ms\n");
    }
  }
  ~Run() { if (file_) std::fclose(file_); }
  class Frame {
   public:
    Frame(Run& run, const char* level, int frame, std::size_t alive)
      : run_(run), level_(level), frame_(frame), alive_(alive) {
      if (run_.file_) { state_.cursor = start_ = Clock::now(); active = &state_; }
    }
    ~Frame() {
      if (!run_.file_) return;
      const auto end = Clock::now(); state_.charge(end); active = nullptr;
      const double total = std::chrono::duration<double, std::milli>(end - start_).count();
      std::fprintf(run_.file_, "%s,%d,%zu,%.6f", level_, frame_, alive_, total);
      for (double ms : state_.ms) std::fprintf(run_.file_, ",%.6f", ms);
      std::fputc('\n', run_.file_);
    }
   private:
    Run& run_; const char* level_; int frame_; std::size_t alive_;
    State state_; Clock::time_point start_;
  };
 private:
  std::FILE* file_ = nullptr;
};
} // namespace webo::app::profile
