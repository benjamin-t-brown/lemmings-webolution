// native-023 step 1: read-only measurement of the serialized teleport link
// fields on every effective installed level. Source-built; the original
// executable is never run. Reports (does not assert) what the structured
// reader holds so the pairing rule can be judged from data.
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "lemmings/compiled_level.hpp"

#include <algorithm>
#include <bit>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool hasTeleportName(const std::string& s) {
  std::string l;
  for (char c : s) l.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  return l.find("teleport") != std::string::npos;
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "teleport link measurement: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    return 0;
  }
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    std::cerr << "open: " << lib.error() << "\n";
    return 1;
  }
  std::ostringstream out;
  int failures = 0;
  std::size_t levels = 0, teleLevels = 0, differ = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob) continue;
    ++levels;
    auto objs = webo::findLevelObjects(*blob);
    std::vector<const webo::PlacedModel*> loc;
    if (objs)
      for (const auto& p : objs->placed)
        if (p.kind == webo::PlacedKind::Teleport) loc.push_back(&p);

    const std::span<const std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()),
                                            blob->size());
    lemmings::CompiledLevel level;
    try {
      level = lemmings::read_compiled_level(raw, false);
    } catch (const std::invalid_argument& e) {
      out << std::format("{} READ-REJECTED: {}\n", name, e.what());
      ++failures;
      continue;
    }
    struct Rec {
      std::size_t index;
      int id;
      std::uint32_t v98;
      std::string m0, m1;
      std::vector<double> d;
    };
    std::vector<Rec> recs;
    const auto& v = level.gadgets.entities_4347ec;
    for (std::size_t i = 0; i < v.size(); ++i) {
      const auto& e = v[i];
      bool tele = hasTeleportName(e.models_1c8_1cc[0].name) || hasTeleportName(e.models_1c8_1cc[1].name);
      for (const auto& n : e.names) tele = tele || hasTeleportName(n);
      if (!tele) continue;
      Rec r{i, e.id, e.value_98, e.models_1c8_1cc[0].name, e.models_1c8_1cc[1].name, {}};
      for (auto b : e.double_bits) r.d.push_back(std::bit_cast<double>(b));
      recs.push_back(r);
    }
    // Also any 426c6b record naming teleport (control).
    std::size_t other = 0;
    for (const auto& e : level.gadgets.entities_426c6b)
      if (e.model && hasTeleportName(e.model->name)) ++other;
    if (loc.empty() && recs.empty() && other == 0) continue;
    ++teleLevels;
    out << std::format("LEVEL {}  locatorTeleports={} structured4347ec(total={},tele={}) 426c6b-tele={}\n",
                       name, loc.size(), v.size(), recs.size(), other);
    for (std::size_t i = 0; i < loc.size(); ++i)
      out << std::format("  loc[{}] {} pos=({:.3f},{:.3f},{:.3f})\n", i, loc[i]->name,
                         loc[i]->pose.position.x, loc[i]->pose.position.y, loc[i]->pose.position.z);
    for (const auto& r : recs) {
      out << std::format("  rec[{}] id={} v98={} m0='{}' m1='{}' base=", r.index, r.id, r.v98, r.m0, r.m1);
      for (double d : r.d) out << std::format("{:.3f} ", d);
      out << "\n";
    }
    // 0x00423325 stores every actor into one list by base id, whatever its class,
    // so also report ids shared across all gadget collections (a collision would
    // change what the pad lookup returns).
    {
      std::map<int, int> all;
      const auto addAll = [&](const auto& v) {
        for (const auto& e : v) ++all[e.id];
      };
      const auto& g = level.gadgets;
      addAll(g.basic_entities); addAll(g.linked_entities); addAll(g.entities_425e50);
      addAll(g.entities_426c6b); addAll(g.spatial_entities); addAll(g.entities_424187);
      addAll(g.entities_42acac); addAll(g.entities_4347ec); addAll(g.basic_4342bf);
      addAll(g.spatial_41e038); addAll(g.tagged_entities); addAll(g.named_entities);
      std::size_t dupAll = 0, hitsOnLinks = 0;
      for (const auto& [id, count] : all)
        if (count > 1) ++dupAll;
      for (const auto& r : recs) {
        const auto it = all.find(static_cast<int>(r.v98));
        if (it != all.end() && it->second > 1) ++hitsOnLinks;
      }
      out << std::format("  ALL-ACTOR-IDS actors={} distinctIds={} idsSharedByMoreThanOneActor={} padLinkTargetsSharedById={}\n",
                         [&] { std::size_t n = 0; for (auto& [i, c] : all) n += c; return n; }(),
                         all.size(), dupAll, hitsOnLinks);
    }
    // id rule candidate: partner of rec k = rec whose id == v98.
    // Compare against file-order pairing i <-> i+n/2 on the same sequence.
    const std::size_t n = recs.size();
    std::map<int, std::size_t> byId;
    bool dupId = false;
    for (std::size_t i = 0; i < n; ++i)
      if (!byId.emplace(recs[i].id, i).second) dupId = true;
    std::string idPairs, filePairs;
    bool unpaired = false, mismatch = false;
    for (std::size_t i = 0; i < n; ++i) {
      auto it = byId.find(static_cast<int>(recs[i].v98));
      long idp = it == byId.end() ? -1 : static_cast<long>(it->second);
      if (idp < 0) unpaired = true;
      long fp = (n >= 2 && n % 2 == 0) ? static_cast<long>((i + n / 2) % n) : -1;
      if (idp != fp) mismatch = true;
      idPairs += std::format("{}->{} ", i, idp);
      filePairs += std::format("{}->{} ", i, fp);
    }
    if (mismatch) ++differ;
    out << std::format("  RESULT n={} locatorN={} countsEqual={} dupIds={} unpaired={} idRule[{}] fileOrder[{}] DIFFER={}\n",
                       n, loc.size(), n == loc.size(), dupId, unpaired, idPairs, filePairs, mismatch);
  }
  out << std::format("levels={} teleportLevels={} levelsWhereIdRuleDiffersFromFileOrder={} readFailures={}\n",
                     levels, teleLevels, differ, failures);
  std::cout << out.str();
  std::error_code ec;
  fs::create_directories("native-023", ec);
  std::ofstream("native-023/step1-measure.txt") << out.str();
  return 0;
}
