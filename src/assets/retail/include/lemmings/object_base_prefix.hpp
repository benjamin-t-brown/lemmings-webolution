#pragma once

#include "lemmings/level_stream.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace lemmings {

// Portable locations in the source stream, not fields of an original object.
// Ends are exclusive; holder_end includes flags and any auxiliary payload.
struct ObjectSourceExtent {
    std::size_t marker_begin = 0;
    std::size_t typed_root_begin = 0;
    std::size_t tree_end = 0;
    std::size_t holder_end = 0;
};

// Initial file-read slice of 0x00437D48 -> 0x004749BF -> class tag zero.
struct ObjectBasePrefix {
    ObjectSourceExtent source_extent;
    std::optional<ObjectSourceExtent> second_object_source_extent;
    bool marker_matches = false;
    std::uint32_t class_tag = 0;
    std::array<std::uint64_t, 12> double_bits{};
    std::string name;
    std::array<std::uint64_t, 2> final_double_bits{};
    std::uint32_t child_count = 0;
    bool first_flag = false;
    bool has_auxiliary_list = false;
    std::uint32_t auxiliary_count = 0;
    struct AuxiliaryRecord {
        struct Spin {
            std::uint32_t node_index;
            std::uint32_t axis_selector;
            std::uint64_t turns_per_second_bits;
        };
        std::uint64_t leading_double_bits = 0;
        std::uint32_t first_count = 0;
        std::uint32_t second_count = 0;
        std::uint32_t third_count = 0;
        std::string name;
        std::uint32_t trailing_integer_count = 0;
        std::vector<std::uint8_t> serialized_bytes;
        std::vector<Spin> spins;
    };
    std::vector<AuxiliaryRecord> auxiliary_records;
    bool has_second_object = false;
    bool second_object_marker_matches = false;
    std::uint32_t second_object_class_tag = 0;
    std::array<std::uint64_t, 12> second_object_double_bits{};
    std::string second_object_name;
    std::array<std::uint64_t, 2> second_object_final_double_bits{};
    std::uint32_t second_object_child_count = 0;
    struct Child {
        std::uint32_t class_tag = 0;
        struct ClassTwoData {
            std::array<std::uint64_t, 3> leading_double_bits{};
            bool first_flag = false;
            bool extended_resource = false;
            std::string resource_name;
            std::array<std::uint32_t, 4> resource_values{};
            std::array<std::uint64_t, 4> resource_double_bits{};
            std::uint32_t final_value = 0;
        };
        std::optional<ClassTwoData> class_two;
        std::array<std::uint64_t, 10> class_three_double_bits{};
        std::vector<std::array<std::uint64_t, 3>> first_triples;
        std::vector<std::array<std::uint64_t, 3>> second_triples;
        struct NamedReference {
            std::string name;
            std::uint32_t value = 0;
        };
        std::vector<NamedReference> named_references;
        std::uint32_t polygon_count = 0;
        struct PolygonVertex {
            std::uint32_t value = 0;
            std::array<std::uint64_t, 6> double_bits{};
        };
        struct PolygonRecord {
            std::uint32_t leading_value = 0;
            std::array<std::uint64_t, 4> plane_double_bits{}; // Types four and six.
            std::vector<PolygonVertex> vertices;
        };
        struct Polygon {
            std::uint32_t allocated_bytes = 0;
            bool marker_matches = false;
            std::uint32_t type = 0;
            std::uint32_t record_count = 0;
            std::uint32_t extra_value = 0;
            std::vector<PolygonRecord> records;
        };
        std::vector<Polygon> polygons;
        std::array<std::uint64_t, 12> following_double_bits{};
        std::string following_name;
        std::array<std::uint64_t, 2> final_double_bits{};
        std::uint32_t nested_child_count = 0;
        std::vector<Child> nested_children;
    };
    std::vector<Child> children;
    std::vector<Child> second_object_children;
    bool second_object_first_flag = false;
    bool second_object_has_auxiliary_list = false;
    std::uint32_t second_object_auxiliary_count = 0;
    std::vector<AuxiliaryRecord> second_object_auxiliary_records;
};

ObjectBasePrefix read_object_base_prefix(LevelByteStream& stream);

// Shared 0x004749BF object reader, without the outer paired-object wrapper.
struct SerializedObject {
    ObjectSourceExtent source_extent;
    bool marker_matches = false;
    ObjectBasePrefix::Child root;
    bool first_flag = false;
    bool has_auxiliary_list = false;
    std::vector<ObjectBasePrefix::AuxiliaryRecord> auxiliary_records;
};

SerializedObject read_serialized_object(LevelByteStream& stream);

} // namespace lemmings
