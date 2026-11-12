#include "lemmings/level_name_tables.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>

namespace lemmings {
namespace {
std::uint32_t read_count(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 4) {
        throw std::invalid_argument("truncated level name count");
    }
    const auto* bytes = stream.bytes.data() + stream.cursor;
    stream.cursor += 4;
    return static_cast<std::uint32_t>(bytes[0]) |
           (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) |
           (static_cast<std::uint32_t>(bytes[3]) << 24);
}

std::string read_name(LevelByteStream& stream, std::int32_t capacity = 255) {
    char name[260]{};
    read_level_string(stream, name, capacity);
    return name;
}

std::uint64_t read_u64(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 8) {
        throw std::invalid_argument("truncated object double");
    }
    std::uint64_t bits = 0;
    for (int i = 0; i < 8; ++i) {
        bits |= static_cast<std::uint64_t>(stream.bytes[stream.cursor++]) << (8 * i);
    }
    return bits;
}

std::vector<std::array<std::uint64_t, 3>> read_triples(LevelByteStream& stream) {
    const auto count = read_count(stream);
    if (count > (stream.bytes.size() - stream.cursor) / 24) {
        throw std::invalid_argument("invalid object triple count");
    }
    std::vector<std::array<std::uint64_t, 3>> triples(count);
    for (auto& triple : triples) {
        for (auto& bits : triple) bits = read_u64(stream);
    }
    return triples;
}

std::uint8_t read_byte(LevelByteStream& stream) {
    if (stream.cursor >= stream.bytes.size()) {
        throw std::invalid_argument("truncated object flag");
    }
    return stream.bytes[stream.cursor++];
}

using ClassOneHead = CompiledLevelPrefix::FirstObjectHead::ClassOneHead;

ClassOneHead read_class_one(LevelByteStream& stream, int depth) {
    if (depth > 32) throw std::invalid_argument("nested object depth exceeded");
    ClassOneHead class_head{};
    class_head.first_triples = read_triples(stream);
    class_head.second_triples = read_triples(stream);
    const auto count = read_count(stream);
    if (count > (stream.bytes.size() - stream.cursor) / 8) {
        throw std::invalid_argument("invalid object reference count");
    }
    class_head.named_references.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        const auto name = read_name(stream, 260);
        const auto value = read_count(stream);
        class_head.named_references.push_back({name, value});
    }
    class_head.following_polygon_count = read_count(stream);
    if (class_head.following_polygon_count >
        (stream.bytes.size() - stream.cursor) / 20) {
        throw std::invalid_argument("invalid polygon count");
    }
    class_head.polygons.reserve(class_head.following_polygon_count);
    for (std::uint32_t i = 0; i < class_head.following_polygon_count; ++i) {
        ClassOneHead::Polygon polygon{};
        polygon.allocated_bytes = read_count(stream);
        polygon.marker_matches = read_name(stream) == "PolyList 09";
        polygon.type = read_count(stream);
        polygon.record_count = read_count(stream);
        polygon.extra_value = read_count(stream);
        if (polygon.type != 4) {
            throw std::invalid_argument("unrecovered polygon type");
        }
        if (polygon.record_count > (stream.bytes.size() - stream.cursor) / 96) {
            throw std::invalid_argument("invalid type-four polygon count");
        }
        polygon.type_four_records.resize(polygon.record_count);
        for (auto& record : polygon.type_four_records) {
            record.leading_value = read_count(stream);
            for (auto& bits : record.triple) bits = read_u64(stream);
            record.scalar = read_u64(stream);
            for (auto& component : record.components) {
                component.value = read_count(stream);
                for (auto& bits : component.pair) bits = read_u64(stream);
            }
        }
        class_head.polygons.push_back(std::move(polygon));
    }
    for (auto& bits : class_head.following_doubles) bits = read_u64(stream);
    class_head.following_name = read_name(stream, 128);
    for (auto& bits : class_head.final_doubles) bits = read_u64(stream);
    class_head.following_nested_object_count = read_count(stream);
    if (class_head.following_nested_object_count >
        (stream.bytes.size() - stream.cursor) / 4) {
        throw std::invalid_argument("invalid nested object count");
    }
    class_head.nested_class_one.reserve(class_head.following_nested_object_count);
    for (std::uint32_t i = 0; i < class_head.following_nested_object_count; ++i) {
        const auto tag = read_count(stream);
        if (tag != 1) throw std::invalid_argument("unrecovered nested class tag");
        class_head.nested_class_one.push_back(read_class_one(stream, depth + 1));
    }
    return class_head;
}
}

ClassOneRecord read_class_one_record(LevelByteStream& stream) {
    return read_class_one(stream, 0);
}

std::vector<std::string> read_level_name_table(LevelByteStream& stream,
                                                std::int32_t capacity) {
    const auto count = read_count(stream);
    if (count > (stream.bytes.size() - stream.cursor) / 4) {
        throw std::invalid_argument("invalid level name count");
    }
    std::vector<std::string> names;
    names.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        names.push_back(read_name(stream, capacity));
    }
    return names;
}

CompiledLevelPrefix read_compiled_level_prefix(LevelByteStream& stream) {
    CompiledLevelPrefix result{};
    result.header = read_compiled_level_header(stream);
    result.first_names = read_level_name_table(stream);
    result.second_names = read_level_name_table(stream);
    result.third_names = read_level_name_table(stream);
    result.following_object_count = read_count(stream);
    if (result.following_object_count > 1) {
        throw std::invalid_argument("unrecovered multiple top-level objects");
    }
    if (result.following_object_count != 0) {
        CompiledLevelPrefix::FirstObjectHead head{};
        head.outer_name = read_name(stream);
        head.leading_value = read_count(stream);
        head.inner_name = read_name(stream);
        for (auto& bits : head.serialized_double_bits) bits = read_u64(stream);
        head.marker_matches = read_name(stream) == "Object 09";
        head.class_tag = read_count(stream);
        if (head.class_tag != 1) {
            throw std::invalid_argument("unrecovered top-level class tag");
        }
        head.class_one_head = read_class_one_record(stream);
        head.first_object_flag = read_byte(stream) != 0;
        head.has_auxiliary_list = read_byte(stream) != 0;
        if (head.has_auxiliary_list) {
            head.auxiliary_list_count = read_count(stream);
            if (head.auxiliary_list_count != 0) {
                throw std::invalid_argument("unrecovered nonempty auxiliary list");
            }
        }
        head.has_final_name = read_byte(stream) != 0;
        if (head.has_final_name) head.final_name = read_name(stream, 128);
        result.first_object_head = std::move(head);
    }
    result.fourth_names = read_level_name_table(stream);
    result.fifth_names = read_level_name_table(stream, 260);
    result.following_short_string = read_name(stream, 128);
    return result;
}

} // namespace lemmings
