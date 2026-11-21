#include "assets/Rar20.h"

#include <zlib.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

namespace {

using Data = std::vector<std::byte>;
using Bytes = std::span<const std::byte>;

void require(bool condition, std::string_view reason) {
  if (!condition) {
    std::cerr << "RAR 2.0: " << reason << '\n';
    std::exit(1);
  }
}

std::uint32_t crc(Bytes bytes) {
  return ::crc32_z(0, reinterpret_cast<const Bytef*>(bytes.data()), bytes.size());
}

void setLE(Data& bytes, unsigned offset, std::uint32_t value, unsigned count) {
  for (unsigned i = 0; i < count; ++i)
    bytes[offset + i] = std::byte((value >> (8 * i)) & 0xff);
}

void headerCrc(Data& header) { setLE(header, 0, crc(Bytes(header).subspan(2)), 2); }

Data header(unsigned size, unsigned type, unsigned flags = 0) {
  Data out(size);
  out[2] = std::byte(type);
  setLE(out, 3, flags, 2);
  setLE(out, 5, size, 2);
  return out;
}

void append(Data& out, Bytes input) { out.insert(out.end(), input.begin(), input.end()); }

Data archivePrefix(unsigned flags = 0) {
  // Arbitrary stub bytes exercise the SFX marker search.
  Data out{std::byte{1}, std::byte{2}, std::byte{0x52}, std::byte{0x61}, std::byte{0x72},
           std::byte{0x21}, std::byte{0x1a}, std::byte{0x07}, std::byte{0}};
  auto main = header(13, 0x73, flags);
  headerCrc(main);
  append(out, main);
  return out;
}

void addMember(Data& archive, std::string_view name, unsigned version, unsigned method,
               unsigned flags, Bytes packed, Bytes unpacked) {
  auto file = header(32 + name.size(), 0x74, 0x8000 | flags);
  setLE(file, 7, packed.size(), 4);
  setLE(file, 11, unpacked.size(), 4);
  setLE(file, 16, crc(unpacked), 4);
  file[24] = std::byte(version);
  file[25] = std::byte(method);
  setLE(file, 26, name.size(), 2);
  for (unsigned i = 0; i < name.size(); ++i)
    file[32 + i] = std::byte(name[i]);
  headerCrc(file);
  append(archive, file);
  append(archive, packed);
}

void expectError(Bytes archive, std::string_view entry, std::string_view reason) {
  const auto result = webo::extractRar20(archive, entry);
  require(!result, "damaged or unsupported input succeeded");
  require(result.error().find(reason) != std::string::npos, result.error());
}

void checkStoredAndErrors() {
  const Data data{std::byte{1}, std::byte{42}, std::byte{255}};
  auto archive = archivePrefix();
  addMember(archive, "data", 15, 0x30, 0, data, data);
  const auto payloadEnd = archive.size();
  auto end = header(7, 0x7b);
  headerCrc(end);
  append(archive, end);

  require(webo::extractRar20(archive, "data") == data, "stored member data");
  expectError(Bytes(archive).first(5), "data", "bad RAR marker");
  expectError(Bytes(archive).first(payloadEnd - 1), "data", "truncated");
  expectError(archive, "missing", "entry not found");
  archive[payloadEnd - 1] ^= std::byte{1};
  expectError(archive, "data", "file CRC mismatch");
  archive[payloadEnd - 1] ^= std::byte{1};
  archive[payloadEnd] ^= std::byte{1};
  expectError(archive, "data", "header CRC mismatch");

  auto unsupported = archivePrefix();
  addMember(unsupported, "encrypted", 20, 0x30, 4, data, data);
  addMember(unsupported, "method", 20, 0x36, 0, data, data);
  addMember(unsupported, "version", 29, 0x35, 0, data, data);
  expectError(unsupported, "encrypted", "encrypted");
  expectError(unsupported, "method", "unsupported RAR compression method");
  expectError(unsupported, "version", "unsupported RAR unpack version");
}

struct BitWriter {
  void put(unsigned value, unsigned width) {
    while (width > 0) {
      if (used == 0)
        bytes.push_back(std::byte{0});
      const unsigned bit = (value >> --width) & 1;
      bytes.back() |= std::byte(bit << (7 - used));
      used = (used + 1) % 8;
    }
  }
  Data bytes;
  unsigned used = 0;
};

void writeAudioTables(BitWriter& bits, bool keepLengths) {
  bits.put(1, 1);  // audio mode
  bits.put(keepLengths, 1);
  bits.put(1, 2);  // two channels
  for (unsigned i = 0; i < 19; ++i)
    bits.put(i == 0 ? 1 : (i == 2 || i == 14) ? 2 : 0, 4);
  for (unsigned channel = 0; channel < 2; ++channel) {
    // Delta symbols 0, 1, 255 and table marker 256 take two bits each.
    // In the next table, replace delta 0 with delta 2, making delta 1 code 00.
    for (unsigned symbol = 0; symbol <= 256; ++symbol) {
      if (keepLengths && symbol == 0)
        bits.put(3, 2);  // Add 14 to the old length of 2 modulo 16.
      else if (keepLengths ? symbol == 2 : (symbol < 2 || symbol > 254))
        bits.put(2, 2);  // Add 2 to the old length of 0.
      else
        bits.put(0, 1);  // Keep the old length.
    }
  }
}

void checkAudioContinuation() {
  auto archive = archivePrefix(8);
  BitWriter first;
  writeAudioTables(first, false);
  Data samples;
  for (unsigned i = 1; i <= 32; ++i) {
    first.put(1, 2);  // channel 0, delta 1
    first.put(2, 2);  // channel 1, delta 255
    samples.push_back(std::byte(256 - i));
    samples.push_back(std::byte(i));
  }
  first.put(1, 2);
  // Channel 0's own previous delta wins the adaptation after 32 samples.
  samples.push_back(std::byte{222});
  first.put(3, 2);  // channel 1's table-change symbol after the last sample
  writeAudioTables(first, true);
  addMember(archive, "first", 20, 0x33, 0, first.bytes, samples);

  BitWriter second;
  second.put(2, 2);  // channel 1 recovers its cursor and previous sample
  second.put(0, 2);  // new code for delta 1, with channel 0's adapted predictor
  const Data tail{std::byte{33}, std::byte{220}};
  addMember(archive, "second", 20, 0x35, 0x10, second.bytes, tail);
  require(webo::extractRar20(archive, "first") == samples, "audio samples and adaptation");
  const auto decoded = webo::extractRar20(archive, "second");
  if (!decoded)
    require(false, decoded.error());
  require(*decoded == tail, "audio state and table change across a solid boundary");
}

Data read(const std::filesystem::path& file) {
  std::ifstream in(file, std::ios::binary | std::ios::ate);
  require(bool(in), "open " + file.string());
  const auto size = in.tellg();
  require(size > 0 && size <= 64 * 1024 * 1024, "file size for " + file.string());
  Data bytes(static_cast<std::size_t>(size));
  in.seekg(0);
  require(bool(in.read(reinterpret_cast<char*>(bytes.data()), size)), "read " + file.string());
  return bytes;
}

}  // namespace

int main(int argc, char** argv) {
  require(argc == 3, "expected disc archive and test output paths");
  checkStoredAndErrors();
  checkAudioContinuation();
  std::cout << "In-memory stored, audio continuation and error cases passed\n";
  if (!std::filesystem::exists(argv[1])) {
    std::cout << "SKIP disc fixture (missing " << argv[1] << ")\n";
    return 0;
  }

  const auto archive = read(argv[1]);
  {
    const auto exe = webo::extractRar20(archive, "Lemmings Revolution.exe");
    if (!exe)
      require(false, exe.error());
    require(exe->size() == 852042 && crc(*exe) == 0x364b1d9a, "EXE decoded CRC32");
    std::cout << "Lemmings Revolution.exe CRC32 364b1d9a (discarded in memory)\n";
  }
  const auto written = webo::extractRar20File(argv[1], "patch.box", argv[2]);
  if (!written)
    require(false, written.error());
  require(written == 0xef0b386cu, "path-based extraction of patch.box");
  const auto patch = read(argv[2]);
  require(patch.size() == 8533284 && crc(patch) == 0xef0b386c, "patch.box decoded CRC32");
  std::cout << "patch.box CRC32 ef0b386c\n";
}
