#pragma once

#include "lemmings/compiled_level_header.hpp"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace lemmings {

// File-read slice of the name-table loaders called by 0x004432C1.
// Their in-memory deduplication, allocation, and resource lookup remain
// outside this parser.
std::vector<std::string> read_level_name_table(LevelByteStream& stream,
                                                std::int32_t capacity = 255);

struct CompiledLevelPrefix {
    CompiledLevelHeader header;
    std::vector<std::string> first_names;
    std::vector<std::string> second_names;
    std::vector<std::string> third_names;
    std::vector<std::string> fourth_names;
    std::vector<std::string> fifth_names;
    std::string following_short_string;
    std::uint32_t following_object_count = 0;
    struct FirstObjectHead {
        std::string outer_name;
        std::uint32_t leading_value = 0;
        std::string inner_name;
        std::array<std::uint64_t, 3> serialized_double_bits{};
        bool marker_matches = false;
        std::uint32_t class_tag = 0;
        struct ClassOneHead {
            std::vector<std::array<std::uint64_t, 3>> first_triples;
            std::vector<std::array<std::uint64_t, 3>> second_triples;
            struct NamedReference {
                std::string name;
                std::uint32_t value = 0;
            };
            std::vector<NamedReference> named_references;
            std::uint32_t following_polygon_count = 0;
            struct PolygonFourRecord {
                std::uint32_t leading_value = 0;
                std::array<std::uint64_t, 3> triple{};
                std::uint64_t scalar = 0;
                struct Component {
                    std::uint32_t value = 0;
                    std::array<std::uint64_t, 2> pair{};
                };
                std::array<Component, 3> components{};
            };
            struct Polygon {
                std::uint32_t allocated_bytes = 0;
                bool marker_matches = false;
                std::uint32_t type = 0;
                std::uint32_t record_count = 0;
                std::uint32_t extra_value = 0;
                std::vector<PolygonFourRecord> type_four_records;
            };
            std::vector<Polygon> polygons;
            std::array<std::uint64_t, 12> following_doubles{};
            std::string following_name;
            std::array<std::uint64_t, 2> final_doubles{};
            std::uint32_t following_nested_object_count = 0;
            std::vector<ClassOneHead> nested_class_one;
        };
        std::optional<ClassOneHead> class_one_head;
        bool first_object_flag = false;
        bool has_auxiliary_list = false;
        std::uint32_t auxiliary_list_count = 0;
        bool has_final_name = false;
        std::string final_name;
    };
    std::optional<FirstObjectHead> first_object_head;
};

using ClassOneRecord = CompiledLevelPrefix::FirstObjectHead::ClassOneHead;

ClassOneRecord read_class_one_record(LevelByteStream& stream);

CompiledLevelPrefix read_compiled_level_prefix(LevelByteStream& stream);

} // namespace lemmings
