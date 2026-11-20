// Report installed MM_W_012 entities_42acac fluid gates (vtable 0x004B1020,
// constructor 0x00428680, reader 0x0042ACAC) via lemmings::read_compiled_level.
// Include child models (+0x158 base, +0x15C, optional +0x160/+0x164),
// wave vectors (+0x1B8..+0x278, +0x290), secondary resource name
// (+0x190 -> list +0x194), and words +0x16C/+0x140.
//
// Require 19 readable gates and matching stored heights for the targets of lever (183,60).
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "lemmings/compiled_level.hpp"

#include <array>
#include <bit>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

double dbl(std::uint64_t bits) { return std::bit_cast<double>(bits); }

std::string vec3(const std::array<std::uint64_t, 3>& v) {
  return std::format("({:g},{:g},{:g})", dbl(v[0]), dbl(v[1]), dbl(v[2]));
}

std::string dumpChild(const lemmings::ObjectBasePrefix::Child& c, int depth) {
  std::ostringstream o;
  const std::string pad(static_cast<std::size_t>(depth) * 2 + 8, ' ');
  o << std::format("{}child class_tag={} polygons={} first_triples={} second_triples={} refs={} nested={}\n", pad, c.class_tag,
                   c.polygons.size(), c.first_triples.size(), c.second_triples.size(), c.named_references.size(),
                   c.nested_children.size());
  if (c.class_two)
    o << std::format("{}  class2 resource='{}' extended={} values=[{},{},{},{}] final={}\n", pad, c.class_two->resource_name,
                     c.class_two->extended_resource, c.class_two->resource_values[0], c.class_two->resource_values[1],
                     c.class_two->resource_values[2], c.class_two->resource_values[3], c.class_two->final_value);
  if (!c.following_name.empty()) o << std::format("{}  following_name='{}'\n", pad, c.following_name);
  for (const auto& r : c.named_references) o << std::format("{}  ref '{}' = {}\n", pad, r.name, r.value);
  for (std::size_t i = 0; i < c.polygons.size(); ++i) {
    const auto& p = c.polygons[i];
    std::size_t verts = 0;
    for (const auto& r : p.records) verts += r.vertices.size();
    o << std::format("{}  poly[{}] type={} records={} vertices={} extra={}\n", pad, i, p.type, p.record_count, verts,
                     p.extra_value);
  }
  for (const auto& n : c.nested_children) o << dumpChild(n, depth + 1);
  return o.str();
}

// Report class-1 geometry, including +0x118 vertices, +0x128 normals, the
// 12-double node header and all PolyList records. Kinds 4/6 share one RGBA
// with index/UV corners (0x00479F3A). Kinds 5/7 store index/UV/RGBA per corner.
std::string dumpMesh(const char* label, const lemmings::ObjectBasePrefix::Child& c) {
  std::ostringstream o;
  o << std::format("    {} mesh '{}' class_tag={}\n", label, c.following_name, c.class_tag);
  o << "      head:";
  for (const auto bits : c.following_double_bits) o << std::format(" {:g}", dbl(bits));
  o << std::format(" | final {:g} {:g}\n", dbl(c.final_double_bits[0]), dbl(c.final_double_bits[1]));
  for (std::size_t i = 0; i < c.first_triples.size(); ++i)
    o << std::format("      vertex[{}]={}\n", i, vec3(c.first_triples[i]));
  for (std::size_t i = 0; i < c.second_triples.size(); ++i)
    o << std::format("      normal[{}]={}\n", i, vec3(c.second_triples[i]));
  for (const auto& ref : c.named_references) o << std::format("      material '{}' blend={}\n", ref.name, ref.value);
  for (std::size_t i = 0; i < c.polygons.size(); ++i) {
    const auto& p = c.polygons[i];
    const bool sharedColour = p.type == 4 || p.type == 6;
    for (std::size_t r = 0; r < p.records.size(); ++r) {
      const auto& rec = p.records[r];
      o << std::format("      block[{}] kind={} slot={} record[{}] normal={}", i, p.type, p.extra_value, r, rec.leading_value);
      if (sharedColour)
        o << std::format(" rgba=({:g},{:g},{:g},{:g})", dbl(rec.plane_double_bits[0]), dbl(rec.plane_double_bits[1]),
                         dbl(rec.plane_double_bits[2]), dbl(rec.plane_double_bits[3]));
      o << "\n";
      for (std::size_t k = 0; k < rec.vertices.size(); ++k) {
        const auto& v = rec.vertices[k];
        o << std::format("        corner[{}] vertex={} uv=({:g},{:g})", k, v.value, dbl(v.double_bits[0]), dbl(v.double_bits[1]));
        if (!sharedColour)
          o << std::format(" rgba=({:g},{:g},{:g},{:g})", dbl(v.double_bits[2]), dbl(v.double_bits[3]), dbl(v.double_bits[4]),
                           dbl(v.double_bits[5]));
        o << "\n";
      }
    }
  }
  return o.str();
}

std::string dumpModel(const char* label, const lemmings::LevelModel& m) {
  std::ostringstream o;
  o << std::format("    {}: type={} name='{}' final_name='{}' res=[{},{},{},{}] doubles={} children={}\n", label, m.type, m.name,
                   m.final_name, m.resource_values[0], m.resource_values[1], m.resource_values[2], m.resource_values[3],
                   vec3(m.double_bits), m.children.size());
  if (m.object) {
    o << std::format("      object marker={} first_flag={} aux={} root:\n", m.object->marker_matches, m.object->first_flag,
                     m.object->auxiliary_records.size());
    for (const auto& a : m.object->auxiliary_records)
      o << std::format("      aux name='{}' counts={},{},{} lead={:g}\n", a.name, a.first_count, a.second_count, a.third_count,
                       dbl(a.leading_double_bits));
    o << dumpChild(m.object->root, 0);
  }
  for (const auto& c : m.children) o << dumpModel("child-model", c);
  return o.str();
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "fluid surface measurement: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    return 0;
  }
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    std::cerr << "open: " << lib.error() << "\n";
    return 1;
  }
  int failures = 0;
  const auto fail = [&](const std::string& why) {
    std::cerr << "FAIL: " << why << "\n";
    ++failures;
  };

  std::string member;
  for (const auto& name : webo::levelFileNames(*lib)) {
    std::string upper;
    for (char ch : name) upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    if (upper == "COMPILED\\MM_W_012.LVL") member = name;
  }
  if (member.empty()) {
    fail("MM_W_012 member missing");
    return 1;
  }
  auto blob = lib->read(member);
  if (!blob) {
    fail("MM_W_012 member unreadable: " + blob.error());
    return 1;
  }
  const std::vector<std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()),
                                      reinterpret_cast<const std::uint8_t*>(blob->data()) + blob->size());
  const auto level = lemmings::read_compiled_level(std::span<const std::uint8_t>(raw.data(), raw.size()), false);
  const auto& gates = level.gadgets.entities_42acac;

  std::ostringstream out;
  out << std::format("MM_W_012 entities_42acac gates: {} (member bytes {})\n", gates.size(), raw.size());
  std::set<std::string> resources, baseModels, model15c;
  for (std::size_t i = 0; i < gates.size(); ++i) {
    const auto& g = gates[i];
    out << std::format("gate[{}] id={} flag_38={} flag_28={} flag_40={} flag_41={}\n", i, g.id, g.flag_38, g.flag_28, g.flag_40,
                       g.flag_41);
    out << std::format("    base doubles +8,+10,+18,+20,+30 = {:g},{:g},{:g},{:g},{:g}\n", dbl(g.double_bits[0]),
                       dbl(g.double_bits[1]), dbl(g.double_bits[2]), dbl(g.double_bits[3]), dbl(g.double_bits[4]));
    out << std::format("    min(+48/+50/+58)=({:g},{:g},{:g}) max(+60/+68/+70)=({:g},{:g},{:g})\n", dbl(g.double_bits_48_60[0]),
                       dbl(g.double_bits_48_60[1]), dbl(g.double_bits_48_60[2]), dbl(g.double_bits_48_60[3]),
                       dbl(g.double_bits_48_60[4]), dbl(g.double_bits_48_60[5]));
    out << std::format("    triple arrays +78: {} entries, +88: {} entries\n", g.triple_arrays[0].size(), g.triple_arrays[1].size());
    if (g.model) out << dumpModel("base model (ModelEntity::model, +0x44 owner)", *g.model);
    else out << "    base model: none\n";
    out << dumpModel("+0x15C model", g.model_15c);
    if (g.model_160) out << dumpModel("+0x160 model", *g.model_160);
    else out << "    +0x160: absent\n";
    if (g.model_164) out << dumpModel("+0x164 model", *g.model_164);
    else out << "    +0x164: absent\n";
    out << std::format("    [+0x18C]={} kind[+0x128]={} [+0x168]={} [+0x16C]={} [+0x140]={}\n", g.values_18c_128_168[0],
                       g.values_18c_128_168[1], g.values_18c_128_168[2], g.value_16c, g.value_140);
    out << std::format("    +0x170 vec={} high[+0x98]={:g} stream[+0x1A8]={:g} low[+0xA0]={:g}\n", vec3(g.triple_170),
                       dbl(g.doubles_98_1a8_a0[0]), dbl(g.doubles_98_1a8_a0[1]), dbl(g.doubles_98_1a8_a0[2]));
    out << std::format("    name_a8(+0xA8)='{}' resource_name(+0x190 source)='{}'\n", g.name_a8, g.resource_name);
    for (std::size_t k = 0; k < 4; ++k)
      out << std::format("    pos_off[{}] +0x{:X}={}  amp[{}] +0x{:X}={}\n", k, 0x1B8 + 0x18 * k, vec3(g.triples[2 * k]), k,
                         0x218 + 0x18 * k, vec3(g.triples[2 * k + 1]));
    out << std::format("    +0x278={} +0x290={}\n", vec3(g.triples[8]), vec3(g.triples[9]));
    resources.insert(g.resource_name);
    baseModels.insert(g.model ? g.model->name : "<none>");
    model15c.insert(g.model_15c.name);
  }
  out << "\ndistinct resource_name: ";
  for (const auto& s : resources) out << "'" << s << "' ";
  out << "\ndistinct base model names: ";
  for (const auto& s : baseModels) out << "'" << s << "' ";
  out << "\ndistinct +0x15C model names: ";
  for (const auto& s : model15c) out << "'" << s << "' ";
  out << "\n";

  if (gates.size() != 19) fail(std::format("expected 19 gates, got {}", gates.size()));
  for (const std::size_t i : {std::size_t{11}, std::size_t{12}}) {
    if (i >= gates.size()) continue;
    const auto& g = gates[i];
    if (dbl(g.doubles_98_1a8_a0[0]) != 24.5 || dbl(g.doubles_98_1a8_a0[2]) != 0.0 || g.values_18c_128_168[1] != 0)
      fail(std::format("gate[{}] stored fields differ from the accepted lever-links measurement", i));
  }

  // Stored mesh geometry of the lever (183,60) receivers. Informational only: 0x0042B24D rewrites
  // vertices 2..5 from pos_off[k] every update, so a stored mismatch is reported, never failed.
  std::ostringstream meshes;
  meshes << "MM_W_012 lever (183,60) gate meshes (entities_42acac[11], [12])\n";
  for (const std::size_t i : {std::size_t{11}, std::size_t{12}}) {
    if (i >= gates.size()) continue;
    const auto& g = gates[i];
    meshes << std::format("gate[{}] id={}\n", i, g.id);
    if (g.model && g.model->object) {
      const auto& root = g.model->object->root;
      meshes << dumpMesh("base +0x158", root);
      const std::size_t slotOf[4] = {2, 3, 5, 4};  // pos_off[k] -> vertex slot (0x0042B24D)
      for (std::size_t k = 0; k < 4; ++k)
        if (slotOf[k] < root.first_triples.size())
          meshes << std::format("      stored vertex[{}] {} pos_off[{}]\n", slotOf[k],
                                root.first_triples[slotOf[k]] == g.triples[2 * k] ? "==" : "!=", k);
    }
    if (g.model_15c.object) meshes << dumpMesh("+0x15C", g.model_15c.object->root);
  }

  std::error_code ec;
  fs::create_directories("fluid-surface-research-001", ec);
  std::ofstream("fluid-surface-research-001/gate-records-MM_W_012.txt") << out.str();
  std::ofstream("fluid-surface-research-001/gate-meshes-MM_W_012.txt") << meshes.str();
  std::cout << std::format("fluid surface measurement: {} gates dumped, {} failures\n", gates.size(), failures);
  return failures == 0 ? 0 : 1;
}
