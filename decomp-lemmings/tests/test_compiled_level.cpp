#include "lemmings/compiled_level.hpp"
#include "lemmings/world_state.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename Action>
void rejects(Action action, const char* expected) {
    try { action(); }
    catch (const std::invalid_argument& error) {
        check(std::string(error.what()).find(expected) != std::string::npos,
              "unexpected diagnostic");
        return;
    }
    throw std::runtime_error("invalid input was accepted");
}

template <typename Action>
void rejects_exactly(Action action, const std::string& expected) {
    try { action(); }
    catch (const std::invalid_argument& error) {
        check(error.what() == expected, "unexpected exact diagnostic");
        return;
    }
    throw std::runtime_error("invalid input was accepted");
}

// Independent serialized fixture, not a round trip through production readers.
// The literal boundaries are byte sums documented in compiled_level_composition.md.
constexpr std::array<std::size_t, 5> section_starts{0, 74, 6552, 6850, 6870};
constexpr std::size_t level_size = 6985;

struct LevelBytes {
    std::vector<std::uint8_t> bytes;
    void u8(std::uint8_t value) { bytes.push_back(value); }
    void little_endian(std::uint64_t value, unsigned width) {
        for (unsigned i = 0; i < width; ++i)
            u8(static_cast<std::uint8_t>(value >> (8 * i)));
    }
    void u16(std::uint16_t value) { little_endian(value, 2); }
    void u32(std::uint32_t value) { little_endian(value, 4); }
    void u64(std::uint64_t bits) { little_endian(bits, 8); }
    void text(std::string_view value) {
        u32(static_cast<std::uint32_t>(value.size()));
        bytes.insert(bytes.end(), value.begin(), value.end());
    }
};

std::vector<std::uint8_t> synthetic_level() {
    LevelBytes out;
    out.text("Level 23");
    for (const auto value : {3u, 0x11223344u, 0x55667788u, 0x99AABBCCu,
                             0xDDEEFF00u, 0x10203040u}) out.u32(value);
    for (const auto flag : {0, 1, 0x80, 0}) out.u8(flag);
    for (int i = 0; i < 3; ++i) out.u32(0); // First three name tables.
    out.u32(0); // No prefix object.
    out.u32(0); // Fourth name table.
    out.u32(0); // Fifth name table.
    out.text("prefix");
    check(out.bytes.size() == section_starts[1], "fixture prefix boundary");

    out.u32(0x21324354);
    out.u64(0x8000000000000000ULL); // Negative zero, never converted to double.
    out.u32(0); // Triple blocks.
    out.u32(0); // Double blocks.
    out.u32(0x31425364);
    // Required class-one record: empty arrays, polygons and nested children.
    for (int i = 0; i < 4; ++i) out.u32(0);
    for (int i = 0; i < 12; ++i) out.u64(0);
    out.text("world");
    out.u64(0x3FF8000000000000ULL);
    out.u64(0);
    out.u32(0);
    out.u32(96);
    out.u64(0x7FF80000000023ABULL); // NaN with a literal payload.
    for (int i = 0; i < 4; ++i) out.u32(0); // Integer, triple, integer, byte arrays.
    for (int i = 0; i < 6144; ++i) out.u8(0x6D);
    out.text("in");
    out.text("out");
    for (int i = 0; i < 32; ++i) out.u32(0); // Tail lists.
    check(out.bytes.size() == section_starts[2], "fixture world boundary");

    out.text("Object 09");
    out.u32(0); // First base object: class zero, no children, empty auxiliary list.
    out.u64(0x3FF4000000000000ULL);
    for (int i = 1; i < 12; ++i) out.u64(0);
    out.text("first");
    out.u64(0);
    out.u64(0x4004000000000000ULL);
    out.u32(0);
    out.u8(1);
    out.u8(1);
    out.u32(0);
    out.u8(1); // Include the second object of the pair.
    out.text("Object 09");
    out.u32(0);
    out.u64(0xC004000000000000ULL);
    for (int i = 1; i < 12; ++i) out.u64(0);
    out.text("second");
    out.u64(0);
    out.u64(0x4011000000000000ULL);
    out.u32(0);
    out.u8(0);
    out.u8(1);
    out.u32(0);
    check(out.bytes.size() == section_starts[3], "fixture object boundary");

    out.u32(23); // Registry size is retained metadata, not an allocation here.
    for (int i = 0; i < 4; ++i) out.u32(0); // Groups, name pairs, both actor lists.
    check(out.bytes.size() == section_starts[4], "fixture entity boundary");

    for (int i = 0; i < 3; ++i) out.u32(0); // First three gadget lists.
    out.u32(0xA1B2C3D4);
    for (int i = 0; i < 9; ++i) out.u32(0); // Remaining lists, including tagged/named.
    out.u32(0x10293847);
    for (int i = 1; i < 10; ++i) out.u32(0);
    out.u32(0x56473829);
    out.u32(0); // Integer array.
    for (const auto flag : {1, 0, 0x80, 0, 1}) out.u8(flag);
    out.u16(0xB17E);
    out.u32(0xD4C3B2A1);
    out.u32(0); // Final records.
    check(out.bytes.size() == level_size, "fixture final boundary");
    return out.bytes;
}

void check_synthetic_values(const lemmings::CompiledLevel& level) {
    const auto& prefix = level.prefix;
    check(prefix.header.fields == std::array<std::uint32_t, 6>{
        3, 0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x10203040},
        "prefix fields mismatch");
    check(prefix.header.first_field_times_58 == 174 &&
          prefix.header.flags == std::array<bool, 4>{false, true, true, false},
          "prefix arithmetic/flags mismatch");
    check(prefix.following_short_string == "prefix", "prefix string mismatch");
    check(prefix.first_names.empty() && prefix.second_names.empty() &&
          prefix.third_names.empty() && prefix.fourth_names.empty() &&
          prefix.fifth_names.empty() && prefix.following_object_count == 0 &&
          !prefix.first_object_head, "prefix empty collections mismatch");

    const auto& world = level.world;
    check(world.leading_value == 0x21324354 &&
          world.following_class_leading_value == 0x31425364 &&
          world.following_value == 96, "world fields mismatch");
    check(world.following_double_bits == 0x8000000000000000ULL &&
          world.following_double_bits_2 == 0x7FF80000000023ABULL &&
          world.following_class_one.final_doubles[0] == 0x3FF8000000000000ULL,
          "world binary64 bits mismatch");
    check(world.following_class_one.following_name == "world" &&
          world.first_name == "in" && world.second_name == "out" &&
          world.fixed_bytes.front() == 0x6D && world.fixed_bytes.back() == 0x6D,
          "world names/fixed bytes mismatch");
    const auto& class_one = world.following_class_one;
    check(class_one.first_triples.empty() && class_one.second_triples.empty() &&
          class_one.named_references.empty() && class_one.following_polygon_count == 0 &&
          class_one.polygons.empty() && class_one.following_nested_object_count == 0 &&
          class_one.nested_class_one.empty(), "world class-one empty collections mismatch");
    check(world.first_blocks.empty() && world.second_blocks.empty() &&
          world.first_integer_array.empty() && world.integer_triples.empty() &&
          world.second_integer_array.empty() && world.byte_array.empty(),
          "world empty arrays mismatch");
    for (const auto& list : world.tail_lists)
        check(list.empty(), "world tail list not empty");

    const auto& objects = level.objects;
    check(objects.marker_matches && objects.class_tag == 0 && objects.name == "first" &&
          objects.double_bits[0] == 0x3FF4000000000000ULL &&
          objects.final_double_bits[1] == 0x4004000000000000ULL &&
          objects.first_flag && objects.has_auxiliary_list,
          "first paired object mismatch");
    check(objects.has_second_object && objects.second_object_marker_matches &&
          objects.second_object_class_tag == 0 && objects.second_object_name == "second" &&
          objects.second_object_double_bits[0] == 0xC004000000000000ULL &&
          objects.second_object_final_double_bits[1] == 0x4011000000000000ULL &&
          !objects.second_object_first_flag && objects.second_object_has_auxiliary_list,
          "second paired object mismatch");
    check(objects.child_count == 0 && objects.children.empty() &&
          objects.auxiliary_count == 0 && objects.auxiliary_records.empty() &&
          objects.second_object_child_count == 0 && objects.second_object_children.empty() &&
          objects.second_object_auxiliary_count == 0 &&
          objects.second_object_auxiliary_records.empty(), "paired empty collections mismatch");

    const auto& entities = level.entities;
    check(entities.registry_size == 23, "entity registry sentinel mismatch");
    check(entities.groups.empty() && entities.name_pairs.empty() && entities.actors.empty() &&
          entities.subclass_actors.empty(), "entity tables not empty");

    const auto& gadgets = level.gadgets;
    check(gadgets.value_5568 == 0xA1B2C3D4 && gadgets.values_27d0[0] == 0x10293847 &&
          gadgets.values_27d0[10] == 0x56473829 && gadgets.short_50a8 == 0xB17E &&
          gadgets.value_50a0 == 0xD4C3B2A1 &&
          gadgets.flags_50cc == std::array<bool, 5>{true, false, true, false, true},
          "gadget sentinel fields mismatch");
    check(gadgets.basic_entities.empty() && gadgets.linked_entities.empty() &&
          gadgets.entities_425e50.empty() && gadgets.entities_426c6b.empty() &&
          gadgets.spatial_entities.empty() && gadgets.entities_424187.empty() &&
          gadgets.entities_42acac.empty() && gadgets.entities_4347ec.empty() &&
          gadgets.basic_4342bf.empty() && gadgets.spatial_41e038.empty() &&
          gadgets.tagged_entities.empty() && gadgets.named_entities.empty() &&
          gadgets.values_1fb0.empty() && gadgets.final_records.empty(),
          "gadget collections not empty");
    check(level.consumed_bytes == 6985, "synthetic consumed byte boundary mismatch");
}

void test_synthetic_composition() {
    auto bytes = synthetic_level();
    const auto original = bytes;
    const auto strict = lemmings::read_compiled_level(bytes, true);
    check(strict.prefix.header.marker_matches, "synthetic level marker mismatch");
    check_synthetic_values(strict);
    check(bytes == original, "strict parse changed input bytes");

    bytes.insert(bytes.end(), {0xAB, 0xCD, 0xEF});
    const auto extended = bytes;
    rejects_exactly([&] { lemmings::read_compiled_level(bytes); },
                    "trailing level data at byte 6985 (3 bytes remain)");
    check(bytes == extended, "strict rejection changed input bytes");
    const auto lenient = lemmings::read_compiled_level(bytes, false);
    check_synthetic_values(lenient);
    check(lenient.prefix.header.marker_matches && bytes.size() - lenient.consumed_bytes == 3,
          "lenient parse consumed trailing bytes");
    check(bytes == extended, "lenient parse changed input bytes");

    bytes = original;
    bytes[11] = '4'; // Same-length "Level 24": mismatch does not stop the reader.
    const auto mismatching = bytes;
    const auto level = lemmings::read_compiled_level(bytes);
    check(!level.prefix.header.marker_matches, "mismatching marker was accepted as matching");
    check_synthetic_values(level);
    check(bytes == mismatching, "marker-mismatch parse changed input bytes");

    constexpr std::array<const char*, 5> diagnostics{
        "prefix at byte 0: invalid level string read",
        "world geometry at byte 74: truncated world geometry field",
        "paired objects at byte 6552: invalid level string read",
        "entity tables at byte 6850: truncated entity field",
        "gadget records at byte 6870: truncated gadget field"};
    bytes = original;
    for (std::size_t i = 0; i < section_starts.size(); ++i) {
        rejects_exactly([&] {
            lemmings::read_compiled_level(std::span(bytes).first(section_starts[i]));
        }, diagnostics[i]);
    }
    check(bytes == original, "section truncation changed input bytes");
}
}

int main(int argc, char** argv) {
    rejects([] { lemmings::read_compiled_level({}); }, "prefix at byte 0");
    rejects([] { lemmings::load_compiled_level({}, "MISSING.LVL"); }, "not found");
    test_synthetic_composition();
    if (argc == 1) return 0;
    check(argc <= 3, "usage: test_compiled_level [base.box [patch.box]]");
    std::vector<lemmings::BoxArchiveIndex> archives;
    if (argc == 3) archives.push_back(lemmings::read_box_archive_index(argv[2]));
    archives.push_back(lemmings::read_box_archive_index(argv[1]));

    std::size_t checked = 0;
    for (const auto& archive : archives) {
        for (std::size_t i = 0; i < archive.entries.size(); ++i) {
            const auto& entry = archive.entries[i];
            if (!entry.name.ends_with(".LVL")) continue;
            const auto bytes = archive.read_entry(i);
            const auto level = lemmings::read_compiled_level(bytes);
            check(level.consumed_bytes == bytes.size(), "incomplete level");
            check(level.prefix.header.marker_matches, "unexpected level marker");
            auto collision=lemmings::collision_grid_from_level(level.world);
            auto grid=collision.view();
            check(grid.height==static_cast<std::int32_t>(level.world.following_value),
                  "collision height mapping mismatch");
            check(collision.cells.size()==level.world.second_integer_array.size(),
                  "collision array mapping mismatch");
            for(std::size_t cell=0;cell<collision.cells.size();++cell)
                check(std::bit_cast<std::uint32_t>(collision.cells[cell])==
                    level.world.second_integer_array[cell],"collision value mismatch");
            if (checked++ == 0) {
                auto extended = bytes;
                extended.push_back(0xAB);
                rejects([&] { lemmings::read_compiled_level(extended); }, "trailing level data");
                check(lemmings::read_compiled_level(extended, false).consumed_bytes == bytes.size(),
                      "lenient parse consumed trailing bytes");
                rejects([&] {
                    lemmings::read_compiled_level(std::span(bytes).first(bytes.size() - 1));
                }, "gadget records");
            }
            // Verify archive selection through the composition API, including
            // overridden base entries. Payload readers are checked separately.
            const auto location = lemmings::find_box_entry(archives, entry.name);
            check(location.has_value(), "indexed level not found");
            const auto loaded = lemmings::load_compiled_level(archives, entry.name);
            check(loaded.archive_path == archives[location->archive].path,
                  "archive precedence mismatch");
            check(loaded.entry_name == entry.name, "entry name mismatch");
            check(loaded.entry_size == loaded.level.consumed_bytes, "loaded level incomplete");
        }
    }
    check(checked != 0, "archives contain no levels");
    std::cout << "Parsed " << checked << " installed level entries through the composed API\n";
}
