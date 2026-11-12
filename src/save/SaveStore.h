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

// Byte storage behind the codec. Injectable so tests (and a future web store)
// can substitute a failing or in-memory sink.
class SaveSink {
 public:
  virtual ~SaveSink() = default;
  // No value = no stored file (the original's open-fails path). An error means
  // the file exists but cannot be read; it must never be treated as "absent".
  virtual std::expected<std::optional<std::vector<std::byte>>, std::string> read() = 0;
  // Replaces the stored image. On error the previously stored bytes are intact.
  virtual std::expected<void, std::string> write(std::span<const std::byte> bytes) = 0;
};

// One file on disk. The write goes to a temporary file in the same directory and
// is then atomically swapped in (MoveFileExW REPLACE_EXISTING on Windows,
// rename elsewhere): clone policy, the original rewrites the file in place.
// A target inside `installedGameDir` is refused for both read and write.
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
  // Missing file -> all-zero state (matches the original open-fails path). A file
  // of the wrong size or one that cannot be read is an error; load never writes.
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
// Production store at the clone's own per-user path. Fails (instead of storing
// anywhere) if the resolved path would lie inside the installed game directory.
std::expected<std::shared_ptr<SaveStore>, std::string> makeDefaultSaveStore(
    const std::filesystem::path& installedGameDir);

}  // namespace webo::save
