// Synthetic coverage for branches absent from the 105 installed fixtures.
// Layouts follow the original readers cited beside each record builder.
#include "lemmings/level_gadget_records.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {
using Bytes = std::vector<std::uint8_t>;
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void number(Bytes& bytes, std::uint64_t value, unsigned width = 4) {
    for (unsigned i = 0; i < width; ++i) bytes.push_back(
        static_cast<std::uint8_t>(value >> (8 * i)));
}
void text(Bytes& bytes, std::string_view value) {
    number(bytes, value.size());
    bytes.insert(bytes.end(), value.begin(), value.end());
}
void sequence(Bytes& bytes, unsigned count, std::uint64_t first, unsigned width = 8) {
    for (unsigned i = 0; i < count; ++i) number(bytes, first + i, width);
}
void replace_word(Bytes& bytes, std::size_t offset, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) bytes.at(offset + i) =
        static_cast<std::uint8_t>(value >> (8 * i));
}
template<class F>
void rejects(F read, const Bytes& bytes, const char* message) {
    lemmings::LevelByteStream stream{bytes};
    bool rejected = false;
    try { read(stream); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, message);
    check(stream.cursor <= bytes.size(), "rejected input advanced past its buffer");
}
void model_one(Bytes& bytes, std::string_view name) { // 0x43CEDF -> 0x468BF2
    number(bytes, 1);
    number(bytes, 0x12345678);
    number(bytes, 0x87654321);
    number(bytes, 0);
    number(bytes, 0xFFFFFFFF);
    text(bytes, name);
}
void model_base(Bytes& bytes) { // 0x432283 -> 0x423291; no optional model
    sequence(bytes, 5, 0x7FF0000000000001ULL); // Preserve signaling NaN payloads.
    number(bytes, 0x80, 1);
    number(bytes, 0, 1);
    number(bytes, 0x8000, 2);
    number(bytes, 0xFF, 1);
    number(bytes, 0, 1);
    number(bytes, 0x7F, 1);
}
void spatial(Bytes& bytes, bool populated) { // 0x43130E
    model_base(bytes);
    sequence(bytes, 6, 0x3FF0000000000000ULL);
    number(bytes, populated ? 1 : 0);
    if (populated) sequence(bytes, 3, 0x1111000000000000ULL);
    number(bytes, populated ? 2 : 0);
    if (populated) sequence(bytes, 6, 0x2222000000000000ULL);
}
void tail(Bytes& bytes, bool populated) {
    sequence(bytes, 11, 0x100, 4);
    number(bytes, populated ? 2 : 0);
    if (populated) { number(bytes, 0x80000000); number(bytes, 0xFFFFFFFF); }
    for (auto flag : {0, 1, 0x80, 0, 0xFF}) number(bytes, flag, 1);
    number(bytes, 0xFEDC, 2);
    number(bytes, 0x10203040);
    number(bytes, populated ? 2 : 0); // 0x44D429 -> three u32 per record.
    if (populated) sequence(bytes, 6, 0xA0000000, 4);
}
Bytes empty_gadgets() {
    Bytes bytes(52, 0); // Twelve list counts and intervening +5568 value.
    tail(bytes, false);
    check(bytes.size() == 115, "empty gadget layout must occupy 115 bytes");
    return bytes;
}
Bytes missing_branches() {
    Bytes bytes;
    number(bytes, 0); // basic
    number(bytes, 1); // linked 0x42DA25
    const auto linked_start = bytes.size();
    model_base(bytes);
    sequence(bytes, 2, 0x8000000000000000ULL);
    number(bytes, 0xF1234567);
    text(bytes, "left");
    text(bytes, "right");
    sequence(bytes, 9, 0x3000000000000000ULL);
    sequence(bytes, 3, 0x4000000000000000ULL);
    number(bytes, 0x89ABCDEF);
    check(bytes.size() - linked_start == 184, "linked serialized stride");
    number(bytes, 0); // 425e50
    number(bytes, 0x5568ABCD);
    number(bytes, 0); // 426c6b
    number(bytes, 1); // spatial 0x4489F0
    spatial(bytes, true);
    number(bytes, 0); // 424187
    number(bytes, 0); // 42acac
    number(bytes, 0); // 4347ec
    number(bytes, 0); // basic_4342bf
    number(bytes, 1); // spatial_41e038
    spatial(bytes, false);
    number(bytes, 0); // tagged
    number(bytes, 1); // named FALL -> 0x427CCD
    text(bytes, "FALL");
    text(bytes, "subclass");
    spatial(bytes, false);
    number(bytes, 0xFF, 1);
    number(bytes, 0x7FF0000000001234ULL, 8);
    number(bytes, 0xCAFEBABE);
    number(bytes, 0x80, 1);
    number(bytes, 0, 1);
    for (auto name : {"north", "south", "east", "west"}) text(bytes, name);
    number(bytes, 0, 1);
    number(bytes, 3, 1);
    text(bytes, "reference-A");
    text(bytes, "reference-B");
    number(bytes, 0xFFF0000000000000ULL, 8);
    number(bytes, 0x8000000000000000ULL, 8);
    text(bytes, "fall-name-A");
    text(bytes, "fall-name-B");
    tail(bytes, true);
    return bytes;
}
void type_two_prefix(Bytes& bytes, std::uint32_t children) { // 0x466410
    number(bytes, 2);
    sequence(bytes, 7, 0x2222000000000000ULL);
    number(bytes, 0x230);
    number(bytes, 0); // records_1c8
    number(bytes, 0xFFFFFFFF); // Allocation count consumes no record payload.
    number(bytes, 0xABCDEF01);
    text(bytes, "");
    text(bytes, "");
    number(bytes, children);
}
Bytes class_three_model() { // 0x450F57 + 0x4749BF + 0x47AFB1
    Bytes bytes;
    number(bytes, 0);
    text(bytes, "model");
    sequence(bytes, 3, 0x1100000000000000ULL);
    text(bytes, "Object 09");
    number(bytes, 3);
    sequence(bytes, 10, 0x3300000000000000ULL);
    sequence(bytes, 12, 0x4400000000000000ULL);
    text(bytes, "root-three");
    sequence(bytes, 2, 0x5500000000000000ULL);
    number(bytes, 0); // child count
    number(bytes, 0x80, 1); // first flag
    number(bytes, 0, 1); // no auxiliary list
    number(bytes, 0xFF, 1); // final-name presence
    text(bytes, "last-name");
    return bytes;
}
}

int main() {
    const auto empty = empty_gadgets();
    lemmings::LevelByteStream empty_stream{empty};
    const auto empty_result = lemmings::read_level_gadget_records(empty_stream);
    check(empty_stream.cursor == 115 && empty_result.linked_entities.empty() &&
          empty_result.named_entities.empty() && empty_result.final_records.empty(), "empty lists");

    auto bytes = missing_branches();
    const auto end = bytes.size();
    bytes.insert(bytes.end(), {0xDE, 0xAD, 0xBE, 0xEF});
    lemmings::LevelByteStream stream{bytes};
    const auto result = lemmings::read_level_gadget_records(stream);
    check(stream.cursor == end, "gadget reader consumed trailing sentinel");
    check(result.linked_entities.size() == 1, "linked list count");
    const auto& linked = result.linked_entities.front();
    check(linked.id == -32768 && linked.flag_38 && !linked.flag_28 &&
          linked.flag_40 && linked.flag_41 && !linked.model, "linked base flags/ID");
    check(linked.double_bits.front() == 0x7FF0000000000001ULL &&
          linked.double_bits_60_68[1] == 0x8000000000000001ULL, "linked double payload");
    check(linked.value_50 == 0xF1234567 && linked.names[0] == "left" &&
          linked.names[1] == "right" && linked.matrix_118[8] == 0x3000000000000008ULL &&
          linked.triple_b8[2] == 0x4000000000000002ULL &&
          linked.registry_index == 0x89ABCDEF, "linked trailing fields");
    check(result.value_5568 == 0x5568ABCD && result.spatial_entities.size() == 1 &&
          result.spatial_41e038.size() == 1, "spatial list dispatch");
    const auto& spatial_record = result.spatial_entities.front();
    check(spatial_record.triple_arrays[0].size() == 1 &&
          spatial_record.triple_arrays[1].size() == 2 &&
          spatial_record.triple_arrays[1][1][2] == 0x2222000000000005ULL &&
          result.spatial_41e038.front().triple_arrays[0].empty(), "spatial arrays");
    check(result.named_entities.size() == 1, "FALL list count");
    const auto& falling = result.named_entities.front();
    check(falling.type == "FALL" && falling.subclass_name == "subclass" &&
          falling.flag_98 && falling.flag_1d1 && !falling.flag_1d2 &&
          !falling.flag_1d0 && falling.flag_1d3, "FALL shared flags");
    check(falling.double_a8 == 0x7FF0000000001234ULL && falling.value_c0 == 0xCAFEBABE &&
          falling.names[3] == "west" && falling.falling_references[0] == "reference-A" &&
          falling.falling_references[1] == "reference-B" &&
          falling.subclass_doubles[0] == 0xFFF0000000000000ULL &&
          falling.subclass_doubles[1] == 0x8000000000000000ULL &&
          falling.falling_names[0] == "fall-name-A" &&
          falling.falling_names[1] == "fall-name-B", "FALL field order");
    check(result.values_27d0.back() == 0x10A && result.values_1fb0.size() == 2 &&
          result.values_1fb0.back() == 0xFFFFFFFF &&
          result.flags_50cc == std::array<bool, 5>{false, true, true, false, true} &&
          result.short_50a8 == 0xFEDC && result.value_50a0 == 0x10203040 &&
          result.final_records.size() == 2 &&
          result.final_records[1] == std::array<std::uint32_t, 3>{0xA0000003, 0xA0000004, 0xA0000005},
          "final record boundaries");
    bytes.resize(end);
    for (std::size_t cut = 0; cut < bytes.size(); ++cut) {
        rejects(lemmings::read_level_gadget_records, Bytes(bytes.begin(), bytes.begin() + cut),
                "truncated gadget stream accepted");
    }
    for (const auto offset : {0U, 4U, 8U, 16U, 20U, 24U, 28U, 32U, 36U, 40U, 44U, 48U, 96U, 111U}) {
        auto invalid = empty;
        replace_word(invalid, offset, 0xFFFFFFFF);
        rejects(lemmings::read_level_gadget_records, invalid, "impossible list count accepted");
    }
    auto bad_tag = empty;
    replace_word(bad_tag, 44, 1);
    bad_tag.insert(bad_tag.begin() + 48, {5, 0, 0, 0});
    rejects(lemmings::read_level_gadget_records, bad_tag, "unsupported tagged entity accepted");
    auto bad_name = bytes;
    const auto found = std::search(bad_name.begin(), bad_name.end(), "FALL", "FALL" + 4);
    check(found != bad_name.end(), "missing synthetic FALL marker");
    std::fill_n(found, 4, 'X');
    rejects(lemmings::read_level_gadget_records, bad_name, "unsupported named entity accepted");

    Bytes resource;
    model_one(resource, std::string_view("resource\0ignored", 16));
    lemmings::LevelByteStream model_stream{resource};
    const auto model = lemmings::read_level_model(model_stream);
    check(model_stream.cursor == resource.size() && model.type == 1 && model.name == "resource" &&
          model.resource_values == std::array<std::uint32_t, 4>{0x12345678, 0x87654321, 0, 0xFFFFFFFF},
          "resource model field order/embedded null");
    for (std::size_t cut = 0; cut < resource.size(); ++cut) {
        rejects(lemmings::read_level_model, Bytes(resource.begin(), resource.begin() + cut),
                "truncated model accepted");
    }
    rejects(lemmings::read_level_model, Bytes{3, 0, 0, 0}, "unsupported model tag accepted");

    const auto geometry = class_three_model();
    lemmings::LevelByteStream geometry_stream{geometry};
    const auto geometry_model = lemmings::read_level_model(geometry_stream);
    check(geometry_stream.cursor == geometry.size() && geometry_model.name == "model" &&
          geometry_model.object && geometry_model.object->marker_matches &&
          geometry_model.object->first_flag && !geometry_model.object->has_auxiliary_list &&
          geometry_model.has_final_name && geometry_model.final_name == "last-name", "class3 model envelope");
    const auto& root = geometry_model.object->root;
    check(root.class_tag == 3 && root.class_three_double_bits[9] == 0x3300000000000009ULL &&
          root.following_double_bits[11] == 0x440000000000000BULL &&
          root.following_name == "root-three" && root.final_double_bits[1] == 0x5500000000000001ULL &&
          root.nested_children.empty(), "class3 extension/base boundary");
    auto unknown_object = geometry;
    replace_word(unknown_object, 50, 4); // After model name/vector and Object 09 marker.
    rejects(lemmings::read_level_model, unknown_object, "unsupported object class accepted");

    Bytes nested;
    for (unsigned i = 0; i < 256; ++i) type_two_prefix(nested, 1);
    model_one(nested, "leaf");
    lemmings::LevelByteStream nested_stream{nested};
    const auto tree = lemmings::read_level_model(nested_stream);
    check(nested_stream.cursor == nested.size(), "model depth256 consumption");
    const auto* node = &tree;
    for (unsigned i = 0; i < 256; ++i) {
        check(node->type == 2 && node->children.size() == 1 &&
              node->allocation_count == 0xFFFFFFFF && node->discarded_value == 0xABCDEF01,
              "nested model non-payload allocation count");
        node = &node->children.front();
    }
    check(node->type == 1 && node->name == "leaf", "nested model leaf");
    Bytes too_deep;
    type_two_prefix(too_deep, 1);
    too_deep.insert(too_deep.end(), nested.begin(), nested.end());
    rejects(lemmings::read_level_model, too_deep, "model depth257 accepted");
    Bytes bad_model_count;
    type_two_prefix(bad_model_count, 0);
    replace_word(bad_model_count, 64, 0xFFFFFFFF); // record count at +64
    rejects(lemmings::read_level_model, bad_model_count, "impossible model record count accepted");
    bad_model_count.clear();
    type_two_prefix(bad_model_count, 0xFFFFFFFF);
    rejects(lemmings::read_level_model, bad_model_count, "impossible child-model count accepted");
    std::cout << "gadget/model synthetic missing-branch and malformed-input tests passed\n";
}
