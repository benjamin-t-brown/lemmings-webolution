#include "Rar20.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <new>
#include <optional>
#include <utility>

namespace webo {
namespace {

namespace fs = std::filesystem;
using Bytes = std::span<const std::byte>;

// RAR 2.x layout and LZ codes: Inferno #04, "Format RAR 2.x. Technical information."
// Audio and solid state: https://github.com/bitplane/rar-research/blob/master/doc/RAR15_40_FORMAT_SPECIFICATION.md
constexpr std::array kMarker{std::byte{0x52}, std::byte{0x61}, std::byte{0x72},
                             std::byte{0x21}, std::byte{0x1a}, std::byte{0x07}, std::byte{0}};
constexpr unsigned kMainHeader = 0x73;
constexpr unsigned kFileHeader = 0x74;
constexpr unsigned kCommentHeader = 0x75;
constexpr unsigned kEndHeader = 0x7b;
constexpr unsigned kStored = 0x30;
constexpr unsigned kLongBlock = 0x8000;
constexpr unsigned kLargeFile = 0x0100;
constexpr unsigned kArchiveSolid = 0x0008;
constexpr unsigned kFileSolid = 0x0010;
constexpr unsigned kWindowSize = 1u << 20;
constexpr unsigned kWindowMask = kWindowSize - 1;
// Bound allocations and expansion for user-supplied browser files.
constexpr std::size_t kMaxFileSize = 64u << 20;
constexpr std::size_t kMaxSolidOutput = 256u << 20;

struct DecodeError { std::string message; };

[[noreturn]] void fail(std::string message) { throw DecodeError{std::move(message)}; }

unsigned byte(Bytes bytes, std::size_t offset) {
  return std::to_integer<unsigned>(bytes[offset]);
}

unsigned u16(Bytes bytes, std::size_t offset) {
  return byte(bytes, offset) | (byte(bytes, offset + 1) << 8);
}

std::uint32_t u32(Bytes bytes, std::size_t offset) {
  return u16(bytes, offset) | (std::uint32_t(u16(bytes, offset + 2)) << 16);
}

constexpr auto kCrcTable = [] {
  std::array<std::uint32_t, 256> table{};
  for (unsigned i = 0; i < table.size(); ++i) {
    std::uint32_t value = i;
    for (unsigned bit = 0; bit < 8; ++bit)
      value = (value >> 1) ^ ((value & 1) ? 0xedb88320u : 0);
    table[i] = value;
  }
  return table;
}();

class Crc32 {
public:
  void add(unsigned value) { state_ = (state_ >> 8) ^ kCrcTable[(state_ ^ value) & 0xff]; }
  std::uint32_t value() const { return ~state_; }

private:
  std::uint32_t state_ = 0xffffffffu;
};

std::uint32_t crc32(Bytes bytes) {
  Crc32 crc;
  for (auto value : bytes)
    crc.add(std::to_integer<unsigned>(value));
  return crc.value();
}

void checkHeaderCrc(Bytes header) {
  if ((crc32(header.subspan(2)) & 0xffff) != u16(header, 0))
    fail("RAR header CRC mismatch");
}

void checkCommentHeader(Bytes comment) {
  if (comment.size() < 13 || byte(comment, 2) != kCommentHeader ||
      u16(comment, 5) != comment.size())
    fail("truncated or invalid RAR comment header");
  checkHeaderCrc(comment.first(13));
}

struct Block {
  Bytes header;
  Bytes data;
  unsigned type;
  unsigned flags;

  std::size_t size() const { return header.size() + data.size(); }
};

Block readBlock(Bytes remaining) {
  if (remaining.size() < 7)
    fail("truncated RAR block header");
  const unsigned headerSize = u16(remaining, 5);
  if (headerSize < 7 || headerSize > remaining.size())
    fail("truncated or invalid RAR header size");
  const auto header = remaining.first(headerSize);
  const unsigned type = byte(header, 2);
  const unsigned flags = u16(header, 3);
  std::uint64_t dataSize = 0;
  if (flags & kLongBlock) {
    if (header.size() < 11)
      fail("truncated RAR block data size");
    dataSize = u32(header, 7);
    if (type == kFileHeader && (flags & kLargeFile)) {
      if (header.size() < 40)
        fail("truncated RAR large file header");
      dataSize |= std::uint64_t(u32(header, 32)) << 32;
    }
  }
  remaining = remaining.subspan(headerSize);
  if (dataSize > remaining.size())
    fail("truncated RAR block data");
  return {header, remaining.first(static_cast<std::size_t>(dataSize)), type, flags};
}

struct Member {
  Bytes packed;
  std::string_view name;
  std::uint64_t size;
  std::uint32_t crc;
  unsigned version;
  unsigned method;
  unsigned flags;

  unsigned windowCode() const { return (flags >> 5) & 7; }
  bool isDirectory() const { return windowCode() == 7; }
  bool isSolid(bool archiveSolid) const {
    return version >= 20 ? (flags & kFileSolid) != 0 : archiveSolid;
  }
};

Member readMember(const Block& block) {
  const auto header = block.header;
  const bool large = (block.flags & kLargeFile) != 0;
  const unsigned nameOffset = large ? 40 : 32;
  if (header.size() < nameOffset || !(block.flags & kLongBlock))
    fail("truncated or invalid RAR file header");
  const unsigned nameSize = u16(header, 26);
  if (nameSize == 0 || nameSize > header.size() - nameOffset)
    fail("truncated or invalid RAR filename");
  const auto nameBytes = header.subspan(nameOffset, nameSize);
  if (block.flags & 0x0008) {
    const auto commentStart = nameOffset + nameSize;
    checkHeaderCrc(header.first(commentStart));
    checkCommentHeader(header.subspan(commentStart));
  } else {
    checkHeaderCrc(header);
  }
  std::uint64_t size = u32(header, 11);
  if (large)
    size |= std::uint64_t(u32(header, 36)) << 32;
  return {block.data,
          {reinterpret_cast<const char*>(nameBytes.data()), nameSize},
          size, u32(header, 16), byte(header, 24), byte(header, 25), block.flags};
}

// Bit codes are MSB first. Lookahead can zero-pad the final partial word,
// but consuming bits always checks against the actual packed member size.
class Bits {
public:
  explicit Bits(Bytes data) : data_(data) {}

  unsigned peek(unsigned count) {
    while (held_ < count && !data_.empty()) {
      buffer_ = (buffer_ << 8) | byte(data_, 0);
      held_ += 8;
      data_ = data_.subspan(1);
    }
    const auto word = held_ >= count ? buffer_ >> (held_ - count) : buffer_ << (count - held_);
    return word & ((1u << count) - 1);
  }

  void discard(unsigned count) {
    if (count > held_)
      fail("truncated RAR compressed stream");
    held_ -= count;
  }

  unsigned read(unsigned count) {
    const auto value = peek(count);
    discard(count);
    return value;
  }

  bool canReadTrailer() const { return data_.size() >= 5 || data_.size() * 8 + held_ >= 40; }

private:
  Bytes data_;
  std::uint32_t buffer_ = 0;
  unsigned held_ = 0;
};

class Huffman {
public:
  void build(std::span<const unsigned> lengths) {
    counts_.fill(0);
    fast_.fill({});
    for (const unsigned len : lengths)
      if (len != 0)
        ++counts_[len];

    unsigned code = 0;
    unsigned start = 0;
    for (unsigned len = 1; len <= kMaxBits; ++len) {
      code = (code + counts_[len - 1]) << 1;
      if (code + counts_[len] > (1u << len))
        fail("invalid RAR Huffman table (oversubscribed)");
      firstCode_[len] = code;
      firstSymbol_[len] = start;
      start += counts_[len];
    }

    auto nextSymbol = firstSymbol_;
    for (unsigned symbol = 0; symbol < lengths.size(); ++symbol) {
      const unsigned len = lengths[symbol];
      if (len == 0)
        continue;
      const unsigned index = nextSymbol[len]++;
      symbols_[index] = symbol;
      if (len <= kFastBits) {
        const unsigned prefix = firstCode_[len] + index - firstSymbol_[len];
        const unsigned suffixCount = 1u << (kFastBits - len);
        std::fill_n(fast_.begin() + prefix * suffixCount, suffixCount, Code{symbol, len});
      }
    }
  }

  unsigned decode(Bits& bits) const {
    const unsigned word = bits.peek(kMaxBits);
    const auto fast = fast_[word >> (kMaxBits - kFastBits)];
    if (fast.length != 0) {
      bits.discard(fast.length);
      return fast.symbol;
    }
    for (unsigned len = kFastBits + 1; len <= kMaxBits; ++len) {
      const unsigned prefix = word >> (kMaxBits - len);
      const unsigned index = prefix - firstCode_[len];
      if (index < counts_[len]) {
        bits.discard(len);
        return symbols_[firstSymbol_[len] + index];
      }
    }
    fail("truncated or invalid RAR Huffman code");
  }

private:
  static constexpr unsigned kFastBits = 8;
  static constexpr unsigned kMaxBits = 15;
  struct Code { unsigned symbol; unsigned length; };
  std::array<unsigned, kMaxBits + 1> counts_{}, firstCode_{}, firstSymbol_{};
  std::array<unsigned, 298> symbols_{};
  std::array<Code, 1u << kFastBits> fast_{};
};

struct Slot { unsigned base; unsigned extraBits; };

// Each slot covers a contiguous interval of 2^extraBits values.
template <std::size_t N>
constexpr auto slots(const std::array<unsigned, N>& widths) {
  std::array<Slot, N> result{};
  unsigned base = 0;
  for (unsigned i = 0; i < N; ++i) {
    result[i] = {base, widths[i]};
    base += 1u << widths[i];
  }
  return result;
}

constexpr auto kLengths = slots(std::array{0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                                           1u, 1u, 1u, 1u, 2u, 2u, 2u, 2u,
                                           3u, 3u, 3u, 3u, 4u, 4u, 4u, 4u,
                                           5u, 5u, 5u, 5u});
constexpr auto kShortDistances = slots(std::array{2u, 2u, 3u, 4u, 5u, 6u, 6u, 6u});
constexpr auto kDistances = [] {
  std::array<unsigned, 48> widths{};
  for (unsigned i = 4; i < widths.size(); ++i)
    widths[i] = std::min((i - 2) / 2, 16u);
  return slots(widths);
}();
static_assert(kDistances.back().base == 983040);

unsigned readSlot(Bits& bits, Slot slot) { return slot.base + bits.read(slot.extraBits); }

struct AudioChannel {
  unsigned sample(unsigned delta, int& channelDelta) {
    history = {lastDelta, lastDelta - history[0], history[1], history[2]};
    const std::array taps{history[0], history[1], history[2], history[3], channelDelta};
    int prediction = 8 * lastByte;
    for (unsigned i = 0; i < taps.size(); ++i)
      prediction += weights[i] * taps[i];
    const auto value = static_cast<std::uint8_t>((prediction >> 3) - static_cast<int>(delta));

    const int error = 8 * static_cast<std::int8_t>(delta);
    errors[0] += std::abs(error);
    for (unsigned i = 0; i < taps.size(); ++i) {
      errors[2 * i + 1] += std::abs(error - taps[i]);
      errors[2 * i + 2] += std::abs(error + taps[i]);
    }
    channelDelta = lastDelta = static_cast<std::int8_t>(value - lastByte);
    lastByte = value;

    if (++samples == 32) {
      const auto winner = std::ranges::min_element(errors) - errors.begin();
      if (winner != 0) {
        auto& weight = weights[(winner - 1) / 2];
        // The legacy decrement admits one step from -16 to -17.
        weight = std::clamp(weight + ((winner & 1) ? -1 : 1), -17, 16);
      }
      samples = 0;
      errors.fill(0);
    }
    return value;
  }

  std::array<int, 5> weights{};
  std::array<int, 4> history{};
  std::array<int, 11> errors{};
  int lastDelta = 0;
  int lastByte = 0;
  unsigned samples = 0;
};

class RarDecoder {
public:
  void decode(const Member& member, bool keepOutput, std::vector<std::byte>& output) {
    if (member.flags & 0x0004)
      fail("encrypted RAR entry: " + std::string(member.name));
    if (member.flags & 0x0003)
      fail("unsupported split RAR entry");
    if (member.method < kStored || member.method > 0x35)
      fail("unsupported RAR compression method");
    if ((member.method == kStored && (member.version < 15 || member.version >= 29)) ||
        (member.method != kStored && member.version != 20))
      fail("unsupported RAR unpack version");
    if (member.size > kMaxFileSize || member.size > kMaxSolidOutput - totalOutput_)
      fail("RAR output size limit exceeded");
    totalOutput_ += static_cast<std::size_t>(member.size);
    if (keepOutput)
      output.reserve(static_cast<std::size_t>(member.size));

    if (member.method == kStored) {
      if (member.packed.size() != member.size)
        fail("invalid RAR stored size");
      checkFileCrc(member, crc32(member.packed));
      if (keepOutput)
        output.assign(member.packed.begin(), member.packed.end());
      return;
    }
    if (member.windowCode() > 4)
      fail("unsupported RAR dictionary size");
    decodeCompressed(member, keepOutput ? &output : nullptr);
  }

private:
  static constexpr unsigned kMainCodes = 298;
  static constexpr unsigned kDistanceCodes = 48;
  static constexpr unsigned kRepeatCodes = 28;
  static constexpr unsigned kAudioCodes = 257;
  static constexpr unsigned kMaxChannels = 4;
  static constexpr unsigned kLzNewTables = 269;
  static constexpr unsigned kAudioNewTables = 256;

  void readTables(Bits& bits) {
    audio_ = bits.read(1) != 0;
    if (!bits.read(1))
      lengths_.fill(0);
    unsigned count = kMainCodes + kDistanceCodes + kRepeatCodes;
    if (audio_) {
      channels_ = bits.read(2) + 1;
      if (channel_ >= channels_)
        channel_ = 0;
      count = kAudioCodes * channels_;
    }

    std::array<unsigned, 19> levelLengths{};
    for (auto& length : levelLengths)
      length = bits.read(4);
    Huffman levelTable;
    levelTable.build(levelLengths);
    for (unsigned i = 0; i < count;) {
      const unsigned symbol = levelTable.decode(bits);
      if (symbol < 16) {
        lengths_[i] = (lengths_[i] + symbol) & 15;
        ++i;
        continue;
      }
      unsigned length = 0;
      unsigned run = 0;
      switch (symbol) {
        case 16:
          if (i == 0)
            fail("invalid RAR Huffman length repeat");
          length = lengths_[i - 1];
          run = bits.read(2) + 3;
          break;
        case 17: run = bits.read(3) + 3; break;
        case 18: run = bits.read(7) + 11; break;
      }
      if (run > count - i)
        fail("invalid RAR Huffman length run");
      std::fill_n(lengths_.begin() + i, run, length);
      i += run;
    }

    const std::span<const unsigned> lengths(lengths_);
    if (audio_) {
      for (unsigned i = 0; i < channels_; ++i)
        tables_[i].build(lengths.subspan(i * kAudioCodes, kAudioCodes));
    } else {
      tables_[0].build(lengths.first(kMainCodes));
      tables_[1].build(lengths.subspan(kMainCodes, kDistanceCodes));
      tables_[2].build(lengths.subspan(kMainCodes + kDistanceCodes, kRepeatCodes));
    }
    haveTables_ = true;
  }

  struct Match { unsigned length = 0; unsigned distance = 0; };

  Match readMatch(Bits& bits, unsigned symbol) const {
    if (symbol == 256)
      return lastMatch_;
    if (symbol <= 260) {
      const unsigned distance = recentDistances_[(nextDistance_ + 260 - symbol) & 3];
      const unsigned slot = tables_[2].decode(bits);
      return {2 + readSlot(bits, kLengths[slot]) + (distance >= 0x101) +
                  (distance >= 0x2000) + (distance >= 0x40000), distance};
    }
    if (symbol < kLzNewTables)
      return {2, 1 + readSlot(bits, kShortDistances[symbol - 261])};
    const unsigned length = 3 + readSlot(bits, kLengths[symbol - 270]);
    const unsigned slot = tables_[1].decode(bits);
    const unsigned distance = 1 + readSlot(bits, kDistances[slot]);
    return {length + (distance >= 0x2000) + (distance >= 0x40000), distance};
  }

  static void checkFileCrc(const Member& member, std::uint32_t crc) {
    if (crc != member.crc)
      fail("RAR file CRC mismatch: " + std::string(member.name));
  }

  void decodeCompressed(const Member& member, std::vector<std::byte>* output) {
    if (window_.empty())
      window_.resize(kWindowSize);
    Bits bits(member.packed);
    if (!haveTables_)
      readTables(bits);

    const unsigned windowLimit = (64u << 10) << member.windowCode();
    unsigned remaining = static_cast<unsigned>(member.size);
    Crc32 crc;
    const auto emit = [&](unsigned value) {
      const auto data = static_cast<std::byte>(value);
      window_[position_++ & kWindowMask] = data;
      crc.add(value);
      if (output)
        output->push_back(data);
    };

    while (remaining != 0) {
      const unsigned symbol = tables_[audio_ ? channel_ : 0].decode(bits);
      if (symbol == (audio_ ? kAudioNewTables : kLzNewTables)) {
        readTables(bits);
        continue;
      }
      if (audio_ || symbol < 256) {
        if (audio_) {
          emit(audioChannels_[channel_].sample(symbol, channelDelta_));
          channel_ = (channel_ + 1) % channels_;
        } else {
          emit(symbol);
        }
        --remaining;
        continue;
      }

      const auto match = readMatch(bits, symbol);
      if (match.distance == 0 || match.distance > position_ || match.distance > windowLimit)
        fail("invalid RAR match distance");
      if (match.length == 0 || match.length > remaining)
        fail("invalid RAR match length");
      recentDistances_[nextDistance_] = match.distance;
      nextDistance_ = (nextDistance_ + 1) & 3;
      lastMatch_ = match;
      remaining -= match.length;
      for (unsigned i = 0; i < match.length; ++i)
        emit(std::to_integer<unsigned>(window_[(position_ - match.distance) & kWindowMask]));
    }
    checkFileCrc(member, crc.value());

    // A table-change token can follow the last output byte. Its tables then
    // apply from bit zero of the next packed solid member.
    if (!output && bits.canReadTrailer() &&
        tables_[audio_ ? channel_ : 0].decode(bits) == (audio_ ? kAudioNewTables : kLzNewTables))
      readTables(bits);
  }

  std::vector<std::byte> window_;
  std::size_t totalOutput_ = 0;
  unsigned position_ = 0;
  std::array<unsigned, 4> recentDistances_{};
  unsigned nextDistance_ = 0;
  Match lastMatch_{};
  std::array<Huffman, kMaxChannels> tables_{};
  std::array<unsigned, kAudioCodes * kMaxChannels> lengths_{};
  std::array<AudioChannel, kMaxChannels> audioChannels_{};
  unsigned channels_ = 1;
  unsigned channel_ = 0;
  int channelDelta_ = 0;
  bool audio_ = false;
  bool haveTables_ = false;
};

struct Extracted {
  std::vector<std::byte> bytes;
  std::uint32_t crc;
};

Extracted extract(Bytes source, std::string_view entry, Rar20CrcObserver onPredecessor = nullptr) {
  const auto found = std::search(source.begin(), source.end(), kMarker.begin(), kMarker.end());
  if (found == source.end())
    fail("bad RAR marker (expected RAR 1.5–2.x signature)");
  auto archive = source.subspan(static_cast<std::size_t>(found - source.begin()) + kMarker.size());
  const auto main = readBlock(archive);
  if (main.type != kMainHeader || main.header.size() < 13 || (main.flags & kLongBlock))
    fail("invalid RAR archive header");
  if (main.flags & 0x0002) {
    checkHeaderCrc(main.header.first(13));
    checkCommentHeader(main.header.subspan(13));
  } else {
    checkHeaderCrc(main.header);
  }
  if (main.flags & 0x0080)
    fail("encrypted RAR archive headers");
  if (main.flags & 0x0001)
    fail("unsupported multi-volume RAR archive");

  const bool archiveSolid = (main.flags & kArchiveSolid) != 0;
  archive = archive.subspan(main.size());
  std::size_t chainStart = 0;
  std::size_t firstNeeded = 0;
  std::size_t selectedOffset = 0;
  std::optional<Member> selected;
  // Find the target and its most recent independent compressed predecessor.
  // Validate block framing through the end even if the match was earlier.
  for (std::size_t offset = 0; offset < archive.size();) {
    const auto block = readBlock(archive.subspan(offset));
    if (block.type == kFileHeader) {
      const auto member = readMember(block);
      if (!member.isDirectory()) {
        const bool solid = member.isSolid(archiveSolid);
        if (member.method != kStored && !solid)
          chainStart = offset;
        if (!selected && member.name == entry) {
          selected = member;
          selectedOffset = offset;
          firstNeeded = member.method != kStored && solid ? chainStart : offset;
        }
      }
    } else if (block.type == kCommentHeader) {
      checkCommentHeader(block.header);
    } else {
      checkHeaderCrc(block.header);
      if (block.type == kEndHeader) {
        if (block.flags & 0x0001)
          fail("unsupported multi-volume RAR archive");
        break;
      }
    }
    offset += block.size();
  }
  if (!selected)
    fail("RAR entry not found: " + std::string(entry));

  Extracted result{{}, selected->crc};
  RarDecoder decoder;
  for (std::size_t offset = firstNeeded; offset < selectedOffset;) {
    const auto block = readBlock(archive.subspan(offset));
    if (block.type == kFileHeader) {
      const auto member = readMember(block);
      if (!member.isDirectory()) {
        decoder.decode(member, false, result.bytes);
        if (onPredecessor)
          onPredecessor(member.name, member.crc);
      }
    }
    offset += block.size();
  }
  decoder.decode(*selected, true, result.bytes);
  return result;
}

// Write beside the destination so a failed write or rename preserves its previous contents.
void writeOutput(const fs::path& output, Bytes bytes) {
  struct Staging {
    fs::path directory;
    fs::path file;
    bool created = false;

    ~Staging() {
      if (created) {
        std::error_code ignored;
        fs::remove(file, ignored);
        fs::remove(directory, ignored);
      }
    }
  };

  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto directory = output.parent_path() / (".rar20-" + std::to_string(stamp));
  Staging staging{directory, directory / "member"};
  std::error_code ec;
  staging.created = fs::create_directory(staging.directory, ec);
  if (!staging.created)
    fail("cannot stage RAR output: " + output.string());

  std::ofstream out(staging.file, std::ios::binary);
  if (!out)
    fail("cannot open staged RAR output: " + output.string());
  if (!bytes.empty())
    out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  out.close();
  if (!out)
    fail("cannot write RAR output: " + output.string());
  fs::rename(staging.file, output, ec);
  if (ec)
    fail("cannot replace RAR output: " + output.string() + ": " + ec.message());
}

}  // namespace

std::expected<std::vector<std::byte>, std::string> extractRar20(Bytes archive, std::string_view entry) {
  try {
    return extract(archive, entry).bytes;
  } catch (const DecodeError& error) {
    return std::unexpected(error.message);
  } catch (const std::bad_alloc&) {
    return std::unexpected("insufficient memory for RAR extraction");
  }
}

std::expected<std::uint32_t, std::string> extractRar20File(
    const std::filesystem::path& archive, std::string_view entry,
    const std::filesystem::path& output, Rar20CrcObserver onPredecessor) {
  try {
    std::ifstream in(archive, std::ios::binary | std::ios::ate);
    if (!in)
      fail("cannot open RAR input: " + archive.string());
    const auto size = in.tellg();
    if (size < 0 || size > static_cast<std::streamoff>(kMaxFileSize))
      fail("RAR input size limit exceeded");
    std::vector<std::byte> packed(static_cast<std::size_t>(size));
    in.seekg(0);
    if (!packed.empty() && !in.read(reinterpret_cast<char*>(packed.data()), size))
      fail("truncated RAR input: " + archive.string());
    in.close();

    auto decoded = extract(packed, entry, onPredecessor);
    writeOutput(output, decoded.bytes);
    return decoded.crc;
  } catch (const DecodeError& error) {
    return std::unexpected(error.message);
  } catch (const std::bad_alloc&) {
    return std::unexpected("insufficient memory for RAR extraction");
  }
}

}  // namespace webo

extern "C" int webolution_extract_rar20(const char* archive, const char* entry, const char* output) {
  if (!archive || !entry || !output) {
    std::fputs("RAR extraction requires input path, entry name and output path\n", stderr);
    return 1;
  }
  try {
    const auto result = webo::extractRar20File(archive, entry, output);
    if (result)
      return 0;
    std::fprintf(stderr, "%s\n", result.error().c_str());
  } catch (const std::exception& error) {
    std::fprintf(stderr, "RAR extraction: %s\n", error.what());
  }
  return 1;
}
