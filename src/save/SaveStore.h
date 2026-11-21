#pragma once

#include "save/SavedGame.h"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace webo::save {

// Byte storage interface for file-backed or in-memory save images.
class SaveSink {
 public:
  virtual ~SaveSink() = default;
  // An absent file returns an empty optional, as on the original's open failure.
  // Read failures return an error.
  virtual std::expected<std::optional<std::vector<std::byte>>, std::string> read() = 0;
  // A write failure preserves the stored image.
  virtual std::expected<void, std::string> write(std::span<const std::byte> bytes) = 0;
};

// Clone policy: write beside the target, then replace it atomically.
// Windows uses MoveFileExW REPLACE_EXISTING. The original writes in place.
// Reject reads and writes inside the installedGameDir.
class FileSaveSink final : public SaveSink {
 public:
  explicit FileSaveSink(std::filesystem::path target, std::filesystem::path installedGameDir = {});
  std::expected<std::optional<std::vector<std::byte>>, std::string> read() override;
  std::expected<void, std::string> write(std::span<const std::byte> bytes) override;
  const std::filesystem::path& target() const { return target_; }

 private:
  std::expected<void, std::string> guard() const;
  std::filesystem::path target_, installedGameDir_;
};

class SaveStore {
 public:
  explicit SaveStore(std::unique_ptr<SaveSink> sink) : sink_(std::move(sink)) {}
  // An absent file yields the original all-zero state. Unreadable or
  // wrong-sized files return an error. Loading never writes.
  std::expected<SavedGame, std::string> load();
  std::expected<void, std::string> save(const SavedGame& game);

 private:
  std::unique_ptr<SaveSink> sink_;
};

// True when `candidate` resolves to `directory` or anything below it.
bool pathIsInside(const std::filesystem::path& candidate, const std::filesystem::path& directory);

// <localAppData>/LemmingsWebolution/SavedGame.dat
std::filesystem::path cloneSavePathUnder(const std::filesystem::path& localAppData);
// %LOCALAPPDATA% on Windows (XDG_DATA_HOME or ~/.local/share elsewhere).
std::expected<std::filesystem::path, std::string> localAppDataDirectory();
// Use the clone's per-user path and reject paths inside the installation.
std::expected<std::shared_ptr<SaveStore>, std::string> makeDefaultSaveStore(
    const std::filesystem::path& installedGameDir);

}  // namespace webo::save
