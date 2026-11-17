#pragma once

#include <array>
#include <condition_variable>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace webo::app::detail {

inline constexpr unsigned kRasterStripes = 8;

// Synchronous row partitioning: every pixel has one writer; the caller waits
// for all ranges. Workers never touch scene state, assets, RNG, observers or
// simulation. The caller participates in the last range.
class RasterRows {
 public:
  RasterRows() {
    try {
      for (unsigned i = 0; i < workers_.size(); ++i)
        workers_[i] = std::thread([this, i] {
          unsigned seen = 0;
          std::unique_lock lock(mutex_);
          for (;;) {
            ready_.wait(lock, [&] { return stopping_ || generation_ != seen; });
            if (stopping_) return;
            seen = generation_;
            lock.unlock();
            invoke(i);
            lock.lock();
            if (--remaining_ == 0) done_.notify_one();
          }
        });
    } catch (...) {
      stop();
      throw;
    }
  }
  ~RasterRows() { stop(); }
  template<class F> void run(F&& function) {
    std::function<void(unsigned)> job(std::forward<F>(function));
    {
      std::lock_guard lock(mutex_);
      errors_.fill(nullptr);
      job_ = &job;
      remaining_ = kRasterStripes - 1;
      ++generation_;
    }
    ready_.notify_all();
    invoke(kRasterStripes - 1);
    {
      std::unique_lock lock(mutex_);
      done_.wait(lock, [&] { return remaining_ == 0; });
      job_ = nullptr;
    }
    for (const auto& error : errors_) if (error) std::rethrow_exception(error);
  }
 private:
  void stop() {
    { std::lock_guard lock(mutex_); stopping_ = true; }
    ready_.notify_all();
    for (auto& worker : workers_) if (worker.joinable()) worker.join();
  }
  void invoke(unsigned i) {
    try { (*job_)(i); } catch (...) { errors_[i] = std::current_exception(); }
  }
  std::array<std::thread, kRasterStripes - 1> workers_;
  std::array<std::exception_ptr, kRasterStripes> errors_{};
  std::mutex mutex_;
  std::condition_variable ready_, done_;
  std::function<void(unsigned)>* job_ = nullptr;
  unsigned generation_ = 0, remaining_ = 0;
  bool stopping_ = false;
};

inline RasterRows& rasterRows() {
  thread_local RasterRows rows;
  return rows;
}

// A stage can defer its validated triangles while its texture storage remains
// alive. Each worker visits the complete command list in submission order, but
// owns disjoint framebuffer rows. A single barrier replaces per-triangle waits.
class RasterCommandBatch;
inline thread_local RasterCommandBatch* commandBatch = nullptr;
class RasterCommandBatch {
 public:
  using Draw = std::function<void(int, int, std::size_t&, std::string&)>;
  using Finish = std::function<void(std::size_t)>;
  explicit RasterCommandBatch(int height, bool enabled = true)
      : height_(height), previous_(commandBatch), enabled_(enabled) {
    if (enabled_) commandBatch = this;
  }
  ~RasterCommandBatch() { if (enabled_) commandBatch = previous_; }
  void add(Draw draw, Finish finish = {}) {
    commands_.push_back({std::move(draw), std::move(finish), {}, {}});
  }
  std::vector<std::string> flush() {
    if (commands_.empty()) return {};
    rasterRows().run([&](unsigned i) {
      const int first = height_*i/kRasterStripes, last = height_*(i+1)/kRasterStripes-1;
      for (auto& command : commands_)
        command.draw(first, last, command.pixels[i], command.errors[i]);
    });
    std::vector<std::string> errors;
    for (auto& command : commands_) {
      std::size_t pixels = 0;
      for (auto count : command.pixels) pixels += count;
      if (command.finish) command.finish(pixels);
      for (const auto& error : command.errors) if (!error.empty()) {
        errors.push_back(error); break;
      }
    }
    commands_.clear();
    return errors;
  }
 private:
  struct Command {
    Draw draw;
    Finish finish;
    std::array<std::size_t, kRasterStripes> pixels;
    std::array<std::string, kRasterStripes> errors;
  };
  int height_;
  RasterCommandBatch* previous_;
  bool enabled_;
  std::deque<Command> commands_;
};

} // namespace webo::app::detail
