#include "save/SaveStore.h"

#include <cwctype>
#include <fstream>
#include <system_error>

#ifdef __EMSCRIPTEN__
#include "web/Browser.h"
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace webo::save {
namespace fs = std::filesystem;
namespace {

constexpr std::uintmax_t kMaxReadableSize = 1u << 20;

std::wstring comparisonKey(const fs::path& path) {
  std::error_code ec;
  fs::path resolved = fs::weakly_canonical(path, ec);
  if (ec) resolved = fs::absolute(path, ec).lexically_normal();
  std::wstring key = resolved.generic_wstring();
  while (key.size() > 1 && key.back() == L'/') key.pop_back();
#ifdef _WIN32
  for (auto& ch : key) ch = static_cast<wchar_t>(std::towlower(static_cast<wint_t>(ch)));
#endif
  return key;
}

std::expected<void, std::string> replaceFile(const fs::path& from, const fs::path& to) {
#ifdef __EMSCRIPTEN__
  if (!web::replaceSave(from.c_str(), to.c_str()))
    return std::unexpected("cannot persist " + to.string());
#elif defined(_WIN32)
  if (!MoveFileExW(from.c_str(), to.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    return std::unexpected("cannot replace " + to.string() + " (Windows error " +
                           std::to_string(GetLastError()) + ")");
#else
  std::error_code ec;
  fs::rename(from, to, ec);
  if (ec) return std::unexpected("cannot replace " + to.string() + ": " + ec.message());
#endif
  return {};
}

}  // namespace

bool pathIsInside(const fs::path& candidate, const fs::path& directory) {
  if (directory.empty()) return false;
  const auto c = comparisonKey(candidate), d = comparisonKey(directory);
  if (c == d) return true;
  return c.size() > d.size() && c.compare(0, d.size(), d) == 0 && (d.back() == L'/' || c[d.size()] == L'/');
}

FileSaveSink::FileSaveSink(fs::path target, fs::path installedGameDir)
    : target_(std::move(target)), installedGameDir_(std::move(installedGameDir)) {}

std::expected<void, std::string> FileSaveSink::guard() const {
  if (target_.empty()) return std::unexpected("save path is empty");
  if (pathIsInside(target_, installedGameDir_))
    return std::unexpected("refusing save path inside the installed game directory: " +
                           target_.string());
  return {};
}

std::expected<std::optional<std::vector<std::byte>>, std::string> FileSaveSink::read() {
  if (auto ok = guard(); !ok) return std::unexpected(ok.error());
  std::error_code ec;
  const bool present = fs::exists(target_, ec);
  if (ec) return std::unexpected("cannot inspect " + target_.string() + ": " + ec.message());
  if (!present) return std::optional<std::vector<std::byte>>{};
  if (!fs::is_regular_file(target_, ec))
    return std::unexpected(target_.string() + " is not a regular file");
  const auto size = fs::file_size(target_, ec);
  if (ec) return std::unexpected("cannot size " + target_.string() + ": " + ec.message());
  if (size > kMaxReadableSize)
    return std::unexpected(target_.string() + " is oversize: " + std::to_string(size) + " bytes");
  std::ifstream in(target_, std::ios::binary);
  if (!in) return std::unexpected("cannot open " + target_.string());
  std::vector<std::byte> bytes(static_cast<std::size_t>(size));
  if (!bytes.empty()) in.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  if (!in) return std::unexpected("cannot read " + target_.string());
  return std::optional<std::vector<std::byte>>{std::move(bytes)};
}

std::expected<void, std::string> FileSaveSink::write(std::span<const std::byte> bytes) {
  if (auto ok = guard(); !ok) return std::unexpected(ok.error());
  std::error_code ec;
  const fs::path directory = target_.parent_path();
  if (!directory.empty()) {
    fs::create_directories(directory, ec);
    if (ec) return std::unexpected("cannot create " + directory.string() + ": " + ec.message());
  }
  fs::path temp = target_;
  temp += ".tmp";  // same directory, so the final swap never crosses volumes
  {
    std::ofstream out(temp, std::ios::binary | std::ios::trunc);
    if (!out) return std::unexpected("cannot create temporary save " + temp.string());
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    out.flush();
    if (!out) {
      out.close();
      fs::remove(temp, ec);
      return std::unexpected("cannot write temporary save " + temp.string());
    }
  }
  if (auto replaced = replaceFile(temp, target_); !replaced) {
    fs::remove(temp, ec);  // the previous good file is untouched
    return replaced;
  }
  return {};
}

std::expected<SavedGame, std::string> SaveStore::load() {
  auto stored = sink_->read();
  if (!stored) return std::unexpected(stored.error());
  if (!*stored) return SavedGame{};
  auto parsed = SavedGame::parse(**stored);
  if (!parsed) return std::unexpected(parsed.error().message());
  return *parsed;
}

std::expected<void, std::string> SaveStore::save(const SavedGame& game) {
  const auto bytes = game.serialize();
  return sink_->write(bytes);
}

fs::path cloneSavePathUnder(const fs::path& localAppData) {
  return localAppData / "LemmingsWebolution" / "SavedGame.dat";
}

std::expected<fs::path, std::string> localAppDataDirectory() {
#ifdef _WIN32
  const DWORD needed = GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0);
  if (needed == 0) return std::unexpected("LOCALAPPDATA is not set");
  std::wstring value(needed, L'\0');
  const DWORD copied = GetEnvironmentVariableW(L"LOCALAPPDATA", value.data(), needed);
  if (copied == 0 || copied >= needed) return std::unexpected("cannot read LOCALAPPDATA");
  value.resize(copied);
  return fs::path(value);
#else
  if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg && *xdg) return fs::path(xdg);
  if (const char* home = std::getenv("HOME"); home && *home) return fs::path(home) / ".local" / "share";
  return std::unexpected("neither XDG_DATA_HOME nor HOME is set");
#endif
}

std::expected<std::shared_ptr<SaveStore>, std::string> makeDefaultSaveStore(
    const fs::path& installedGameDir) {
  auto base = localAppDataDirectory();
  if (!base) return std::unexpected(base.error());
  auto target = cloneSavePathUnder(*base);
  if (pathIsInside(target, installedGameDir))
    return std::unexpected("refusing save path inside the installed game directory: " +
                           target.string());
  return std::make_shared<SaveStore>(
      std::make_unique<FileSaveSink>(std::move(target), installedGameDir));
}

}  // namespace webo::save
