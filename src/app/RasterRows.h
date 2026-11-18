#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace webo::app::detail {

// Synchronous row partitioning: every pixel has one writer; the caller waits
// for all ranges. The caller also takes work.
class RasterRows {
 public:
#ifdef __EMSCRIPTEN__
  // One caller drains the same bands in flush(), including legacy error ranges.
  template<class F> void run(F&& function) { function(0); }
#else
  RasterRows() : workers_(std::clamp(std::thread::hardware_concurrency(), 1u, 16u) - 1),
                 errors_(workers_.size() + 1) {
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
      std::fill(errors_.begin(), errors_.end(), nullptr);
      job_ = &job;
      remaining_ = size() - 1;
      ++generation_;
    }
    ready_.notify_all();
    invoke(size() - 1);
    {
      std::unique_lock lock(mutex_);
      done_.wait(lock, [&] { return remaining_ == 0; });
      job_ = nullptr;
    }
    for (const auto& error : errors_) if (error) std::rethrow_exception(error);
  }
 private:
  unsigned size() const { return static_cast<unsigned>(errors_.size()); }
  void stop() {
    { std::lock_guard lock(mutex_); stopping_ = true; }
    ready_.notify_all();
    for (auto& worker : workers_) if (worker.joinable()) worker.join();
  }
  void invoke(unsigned i) {
    try { (*job_)(i); } catch (...) { errors_[i] = std::current_exception(); }
  }
  std::vector<std::thread> workers_;
  std::vector<std::exception_ptr> errors_;
  std::mutex mutex_;
  std::condition_variable ready_, done_;
  std::function<void(unsigned)>* job_ = nullptr;
  unsigned generation_ = 0, remaining_ = 0;
  bool stopping_ = false;
#endif
};

inline RasterRows& rasterRows() {
  thread_local RasterRows rows;
  return rows;
}

// A stage can defer its validated triangles while its texture storage remains
// alive. Bin by clipped Y extent, preserving submission order within each band.
// Short bands let idle cores take rows from a more heavily covered part of the
// image. Only the caller reduces draw counts and errors after the barrier.
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
  void add(int ymin, int ymax, Draw draw, Finish finish = {}) {
    commands_.push_back({ymin, ymax, std::move(draw), std::move(finish)});
  }
  // On extreme inputs the scene sampler can reject UV partway through a
  // triangle. Preserve the old eight independent error/early-return ranges.
  void preserveErrorRanges() { legacyRanges_ = true; }
  std::vector<std::string> flush() {
    return flush([](int, int, auto draw) { draw(); });
  }
  // A stage may measure its disjoint destination rows around the actual draw.
  // The callback executes on the worker that owns the band.
  template<class DrawBand>
  std::vector<std::string> flush(DrawBand drawBand) {
    if (commands_.empty()) return {};
    constexpr int bandHeight = 8;
    const int count = legacyRanges_ ? 8 : (height_ - 1) / bandHeight + 1;
    struct Entry { std::size_t command, pixels = 0; };
    struct Band {
      std::vector<Entry> entries;
      std::vector<std::pair<std::size_t, std::string>> errors;
    };
    std::vector<Band> bands(count);
    const auto bandOf = [&](int y) {
      return legacyRanges_ ? int(((std::int64_t(y) + 1) * 8 - 1) / height_) : y / bandHeight;
    };
    for (std::size_t i = 0; i < commands_.size(); ++i) {
      const int first = std::max(0, commands_[i].ymin);
      const int last = std::min(height_ - 1, commands_[i].ymax);
      if (first > last) continue;
      for (int b = bandOf(first); b <= bandOf(last); ++b)
        bands[b].entries.push_back({i});
    }
    std::atomic<int> next{};
    rasterRows().run([&](unsigned) {
      for (;;) {
        const int b = next.fetch_add(1, std::memory_order_relaxed);
        if (b >= count) break;
        const int first = legacyRanges_ ? int(std::int64_t(height_) * b / 8) : b * bandHeight;
        const int last = legacyRanges_ ? int(std::int64_t(height_) * (b+1) / 8) - 1 :
            std::min(height_ - 1, first + bandHeight - 1);
        auto& band = bands[b];
        if (band.entries.empty()) continue;
        drawBand(first, last, [&] {
          for (auto& entry : band.entries) {
            std::string error;
            commands_[entry.command].draw(first, last, entry.pixels, error);
            if (!error.empty()) band.errors.emplace_back(entry.command, std::move(error));
          }
        });
      }
    });
    for (const auto& band : bands) {
      for (const auto& entry : band.entries) commands_[entry.command].pixels += entry.pixels;
      for (const auto& [i, error] : band.errors)
        if (commands_[i].error.empty()) commands_[i].error = error;
    }
    std::vector<std::string> errors;
    for (auto& command : commands_) {
      if (command.finish) command.finish(command.pixels);
      if (!command.error.empty()) errors.push_back(std::move(command.error));
    }
    commands_.clear();
    legacyRanges_ = false;
    return errors;
  }
 private:
  struct Command {
    int ymin, ymax;
    Draw draw;
    Finish finish;
    std::size_t pixels = 0;
    std::string error{};
  };
  int height_;
  RasterCommandBatch* previous_;
  bool enabled_;
  bool legacyRanges_ = false;
  std::vector<Command> commands_;
};

} // namespace webo::app::detail
