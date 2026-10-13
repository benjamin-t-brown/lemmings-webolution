// Tests for the RIFF/WAVE reader.
//
// The format is standard, so the interesting cases are the ones where the
// retail data is not uniform: seventeen sample rates from 3179 to 44100, both
// 8- and 16-bit, mono and stereo, and three files with chunks after `data`.
// The synthetic tests below cover the parts that are easy to get subtly wrong
// -- 8-bit samples being unsigned, and odd-sized chunks being padded -- and
// the retail pass requires all 73 to decode.

#include "assets/Box.h"
#include "assets/Wav.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

void putTag(std::vector<std::byte>& v, const char* tag) {
  for (int i = 0; i < 4; ++i)
    v.push_back(static_cast<std::byte>(tag[i]));
}
void put32(std::vector<std::byte>& v, std::uint32_t x) {
  for (int i = 0; i < 4; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}
void put16(std::vector<std::byte>& v, std::uint16_t x) {
  for (int i = 0; i < 2; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}

// A RIFF file with the given fmt and data, optionally with an odd-sized chunk
// before the data so the padding rule is exercised.
std::vector<std::byte> makeWav(std::uint16_t format, std::uint16_t channels,
                               std::uint32_t rate, std::uint16_t bits,
                               const std::vector<std::byte>& pcm, bool oddChunkFirst = false,
                               bool trailingChunk = false) {
  std::vector<std::byte> body;

  if (oddChunkFirst) {
    putTag(body, "junk");
    put32(body, 3);
    body.push_back(std::byte{1});
    body.push_back(std::byte{2});
    body.push_back(std::byte{3});
    body.push_back(std::byte{0});  // the pad byte RIFF requires
  }

  putTag(body, "fmt ");
  put32(body, 16);
  put16(body, format);
  put16(body, channels);
  put32(body, rate);
  put32(body, rate * channels * bits / 8);  // byte rate
  put16(body, static_cast<std::uint16_t>(channels * bits / 8));
  put16(body, bits);

  putTag(body, "data");
  put32(body, static_cast<std::uint32_t>(pcm.size()));
  body.insert(body.end(), pcm.begin(), pcm.end());
  if (pcm.size() & 1)
    body.push_back(std::byte{0});

  if (trailingChunk) {
    putTag(body, "smpl");
    put32(body, 4);
    for (int i = 0; i < 4; ++i)
      body.push_back(std::byte{0xAB});
  }

  std::vector<std::byte> file;
  putTag(file, "RIFF");
  put32(file, static_cast<std::uint32_t>(body.size() + 4));
  putTag(file, "WAVE");
  file.insert(file.end(), body.begin(), body.end());
  return file;
}

void testSynthetic() {
  std::cout << "synthetic files\n";

  // 16-bit mono, read straight through.
  std::vector<std::byte> pcm16;
  for (std::int16_t s : {std::int16_t{0}, std::int16_t{1000}, std::int16_t{-1000},
                         std::int16_t{32767}}) {
    const auto u = static_cast<std::uint16_t>(s);
    pcm16.push_back(static_cast<std::byte>(u & 0xFF));
    pcm16.push_back(static_cast<std::byte>(u >> 8));
  }
  auto a = webo::decodeWav(makeWav(1, 1, 22050, 16, pcm16));
  check(a.has_value(), "16-bit mono decodes");
  if (a) {
    check(a->channels == 1 && a->sampleRate == 22050, "format is reported");
    check(a->samples.size() == 4, "four samples");
    check(a->samples[1] == 1000 && a->samples[2] == -1000, "signed values survive");
    check(a->frameCount() == 4, "frame count for mono");
  }

  // 8-bit is UNSIGNED and centred on 128. Silence is 128, not 0; reading it
  // as signed would make silence read as a hard -1.0 and every file buzz.
  const std::vector<std::byte> pcm8 = {std::byte{128}, std::byte{255}, std::byte{0},
                                       std::byte{192}};
  auto b = webo::decodeWav(makeWav(1, 1, 11025, 8, pcm8));
  check(b.has_value(), "8-bit mono decodes");
  if (b) {
    check(b->samples.size() == 4, "four samples");
    check(b->samples[0] == 0, "128 is silence, not full scale");
    check(b->samples[1] > 32000, "255 is near positive full scale");
    check(b->samples[2] < -32000, "0 is near negative full scale");
    check(b->samples[3] > 0 && b->samples[3] < 32000, "192 is partway up");
  }

  // Stereo frames.
  auto c = webo::decodeWav(makeWav(1, 2, 44100, 8, pcm8));
  check(c.has_value(), "stereo decodes");
  if (c) {
    check(c->channels == 2, "two channels");
    check(c->frameCount() == 2, "four samples are two stereo frames");
  }

  // A chunk before fmt, odd-sized, so the pad byte must be skipped. Getting
  // this wrong desynchronises every chunk after it.
  auto d = webo::decodeWav(makeWav(1, 1, 22050, 16, pcm16, /*oddChunkFirst=*/true));
  check(d.has_value(), "an odd-sized leading chunk is skipped with its pad");
  if (d)
    check(d->samples.size() == 4 && d->samples[1] == 1000, "and the data still reads");

  // A chunk after data, as three retail files have.
  auto e = webo::decodeWav(makeWav(1, 1, 22050, 16, pcm16, false, /*trailingChunk=*/true));
  check(e.has_value(), "a trailing chunk is ignored");
  if (e)
    check(e->samples.size() == 4, "and does not corrupt the data");

  // A partial final stereo frame must be dropped, not left to swap the
  // channels for the rest of playback.
  const std::vector<std::byte> odd8 = {std::byte{128}, std::byte{200}, std::byte{64}};
  auto f = webo::decodeWav(makeWav(1, 2, 22050, 8, odd8));
  check(f.has_value(), "a partial final frame decodes");
  if (f)
    check(f->samples.size() == 2 && f->frameCount() == 1, "with the partial frame dropped");

  // Rejections.
  check(!webo::decodeWav(std::vector<std::byte>{}).has_value(), "empty input is rejected");
  check(!webo::decodeWav(makeWav(2, 1, 22050, 16, pcm16)).has_value(), "non-PCM is rejected");
  check(!webo::decodeWav(makeWav(1, 1, 22050, 24, pcm16)).has_value(), "24-bit is rejected");
  check(!webo::decodeWav(makeWav(1, 5, 22050, 16, pcm16)).has_value(),
        "five channels are rejected");
  check(!webo::decodeWav(makeWav(1, 1, 0, 16, pcm16)).has_value(), "a zero rate is rejected");

  auto notRiff = makeWav(1, 1, 22050, 16, pcm16);
  notRiff[0] = std::byte{'X'};
  check(!webo::decodeWav(notRiff).has_value(), "a bad magic is rejected");
}

void testResample() {
  std::cout << "resampling\n";

  webo::Sound mono;
  mono.sampleRate = 1000;
  mono.channels = 1;
  mono.samples = {0, 1000, 2000, 3000};

  // Doubling the rate doubles the frames and keeps the endpoints.
  const auto up = webo::resampleTo(mono, 2000, 1);
  check(up.sampleRate == 2000 && up.channels == 1, "rate and channels are set");
  check(up.frameCount() == 8, std::format("eight frames (got {})", up.frameCount()));
  check(up.samples.front() == 0, "the first sample is preserved");
  // Interpolated, so the midpoint between 0 and 1000 lands between them.
  check(up.samples[1] > 0 && up.samples[1] < 1000, "interpolated, not duplicated");

  // Halving the rate halves the frames.
  const auto down = webo::resampleTo(mono, 500, 1);
  check(down.frameCount() == 2, std::format("two frames (got {})", down.frameCount()));

  // The same rate is a pass-through, which is the common case for the 38
  // retail sounds already at the output rate.
  const auto same = webo::resampleTo(mono, 1000, 1);
  check(same.samples == mono.samples, "an unchanged rate leaves the samples alone");

  // Mono fans out to both channels rather than leaving one silent.
  const auto stereo = webo::resampleTo(mono, 1000, 2);
  check(stereo.channels == 2 && stereo.frameCount() == 4, "mono to stereo keeps the frames");
  check(stereo.samples[0] == stereo.samples[1], "both channels carry the mono signal");

  // Stereo down to mono averages. A hard-panned sound must not disappear,
  // which is what dropping a channel would do.
  webo::Sound panned;
  panned.sampleRate = 1000;
  panned.channels = 2;
  panned.samples = {0, 8000, 0, 8000};  // silent left, loud right
  const auto folded = webo::resampleTo(panned, 1000, 1);
  check(folded.channels == 1 && folded.frameCount() == 2, "stereo to mono keeps the frames");
  check(folded.samples[0] == 4000, "and averages rather than dropping a channel");

  // Degenerate input must not crash or divide by zero.
  check(webo::resampleTo(webo::Sound{}, 22050, 2).empty(), "an empty sound stays empty");
  check(webo::resampleTo(mono, 0, 1).empty(), "a zero rate yields nothing");
  check(webo::resampleTo(mono, 22050, 0).empty(), "zero channels yields nothing");
}

void testAssetNames() {
  std::cout << "cue names\n";
  check(webo::soundAssetName("Letsgo") == "SOUNDS\\LETSGO.WAV", "a cue maps to its entry");
  check(webo::soundAssetName("Yippee") == "SOUNDS\\YIPPEE.WAV", "and another");
}

void testRetail(const fs::path& dir) {
  std::cout << "retail sounds\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  const auto names = webo::soundNames(*lib);
  check(names.size() == 73, std::format("73 sounds (got {})", names.size()));

  std::size_t decoded = 0;
  std::set<int> rates;
  std::set<int> channelCounts;
  double totalSeconds = 0;
  for (const auto& n : names) {
    auto sound = webo::loadSound(*lib, n);
    if (!sound) {
      std::cerr << std::format("  FAIL  {}: {}\n", n, sound.error());
      ++g_failures;
      continue;
    }
    ++decoded;
    rates.insert(sound->sampleRate);
    channelCounts.insert(sound->channels);
    totalSeconds += sound->seconds();
    if (sound->empty())
      std::cerr << std::format("  FAIL  {} decoded to nothing\n", n);
  }

  std::cout << std::format("  {} decoded, {} distinct rates {}..{}, {:.1f}s total\n", decoded,
                           rates.size(), rates.empty() ? 0 : *rates.begin(),
                           rates.empty() ? 0 : *rates.rbegin(), totalSeconds);
  check(decoded == names.size(), "every retail sound decodes");
  check(rates.size() > 10, "the rates really do vary, so a mixer must resample");
  check(channelCounts.count(1) == 1 && channelCounts.count(2) == 1,
        "both mono and stereo are present");

  // The cues a level names must exist, since the simulation will ask for them
  // by name.
  for (const char* cue : {"Letsgo", "Yippee", "Chink", "Explode", "Ohno", "Splat"})
    check(lib->contains(webo::soundAssetName(cue)), std::format("cue '{}' exists", cue));
}

}  // namespace

int main() {
  testSynthetic();
  testResample();
  testResample();
  testAssetNames();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box"))
      testRetail(dir);
    else
      std::cout << "retail sounds: skipped, no lemmings.box\n";
  } else {
    std::cout << "retail sounds: skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
