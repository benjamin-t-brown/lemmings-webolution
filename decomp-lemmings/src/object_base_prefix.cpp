#include "lemmings/object_base_prefix.hpp"

#include <stdexcept>
#include <utility>

namespace lemmings {
namespace {
std::uint8_t read_u8(LevelByteStream& stream) {
    if (stream.cursor >= stream.bytes.size()) {
        throw std::invalid_argument("truncated object flag");
    }
    return stream.bytes[stream.cursor++];
}

std::uint32_t read_u32(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 4) {
        throw std::invalid_argument("truncated object field");
    }
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= static_cast<std::uint32_t>(stream.bytes[stream.cursor++]) << (8 * i);
    }
    return value;
}

std::uint64_t read_u64(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 8) {
        throw std::invalid_argument("truncated object double");
    }
    std::uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= static_cast<std::uint64_t>(stream.bytes[stream.cursor++]) << (8 * i);
    }
    return value;
}

void skip_records(LevelByteStream& stream, std::uint32_t count,
                  std::size_t stride) {
    if (stream.cursor > stream.bytes.size() ||
        count > (stream.bytes.size() - stream.cursor) / stride) {
        throw std::invalid_argument("truncated auxiliary records");
    }
    stream.cursor += static_cast<std::size_t>(count) * stride;
}

std::string read_name(LevelByteStream& stream, int capacity) {
    char text[260]{};
    read_level_string(stream, text, capacity);
    return text;
}

std::vector<std::array<std::uint64_t, 3>> read_triples(LevelByteStream& stream) {
    const auto count = read_u32(stream);
    if (count > (stream.bytes.size() - stream.cursor) / 24) {
        throw std::invalid_argument("invalid child triple count");
    }
    std::vector<std::array<std::uint64_t, 3>> triples(count);
    for (auto& triple : triples) {
        for (auto& bits : triple) bits = read_u64(stream);
    }
    return triples;
}

ObjectBasePrefix::Child read_child(LevelByteStream& stream, unsigned depth) {
    if (depth > 256) {
        throw std::invalid_argument("object nesting exceeds supported depth");
    }
    ObjectBasePrefix::Child child{};
    child.class_tag = read_u32(stream);
    if (child.class_tag == 1) {
        child.first_triples = read_triples(stream);
        child.second_triples = read_triples(stream);
        const auto references = read_u32(stream);
        if (references > (stream.bytes.size() - stream.cursor) / 8) {
            throw std::invalid_argument("invalid child reference count");
        }
        child.named_references.reserve(references);
        for (std::uint32_t i = 0; i < references; ++i) {
            const auto name = read_name(stream, 260);
            const auto value = read_u32(stream);
            child.named_references.push_back({name, value});
        }
        child.polygon_count = read_u32(stream);
        if (child.polygon_count > (stream.bytes.size() - stream.cursor) / 24) {
            throw std::invalid_argument("invalid child polygon count");
        }
        child.polygons.reserve(child.polygon_count);
        for (std::uint32_t polygon_index = 0;
             polygon_index < child.polygon_count; ++polygon_index) {
            ObjectBasePrefix::Child::Polygon polygon{};
            polygon.allocated_bytes = read_u32(stream);
            polygon.marker_matches = read_name(stream, 255) == "PolyList 09";
            polygon.type = read_u32(stream);
            polygon.record_count = read_u32(stream);
            polygon.extra_value = read_u32(stream);
            const std::uint32_t vertices_per_record =
                polygon.type == 4 || polygon.type == 5 ? 3 :
                polygon.type == 6 || polygon.type == 7 ? 4 : 0;
            if (vertices_per_record == 0) {
                throw std::invalid_argument("unrecovered child polygon type");
            }
            const bool planar = polygon.type == 4 || polygon.type == 6;
            const std::size_t record_bytes = planar ? 36 + 20 * vertices_per_record :
                4 + 52 * vertices_per_record;
            if (polygon.record_count >
                (stream.bytes.size() - stream.cursor) / record_bytes) {
                throw std::invalid_argument("invalid child polygon record count");
            }
            polygon.records.reserve(polygon.record_count);
            for (std::uint32_t i = 0; i < polygon.record_count; ++i) {
                ObjectBasePrefix::Child::PolygonRecord record{};
                record.leading_value = read_u32(stream);
                if (planar) {
                    for (auto& bits : record.plane_double_bits) bits = read_u64(stream);
                }
                record.vertices.resize(vertices_per_record);
                for (auto& vertex : record.vertices) {
                    vertex.value = read_u32(stream);
                    const auto doubles = planar ? 2 : 6;
                    for (int j = 0; j < doubles; ++j) {
                        vertex.double_bits[j] = read_u64(stream);
                    }
                }
                polygon.records.push_back(std::move(record));
            }
            child.polygons.push_back(std::move(polygon));
        }
    } else if (child.class_tag == 2) {
        ObjectBasePrefix::Child::ClassTwoData data{};
        for (auto& bits : data.leading_double_bits) bits = read_u64(stream);
        data.first_flag = read_u8(stream) != 0;
        data.extended_resource = read_u8(stream) != 0;
        if (data.extended_resource) {
            for (auto& value : data.resource_values) value = read_u32(stream);
            data.resource_name = read_name(stream, 255);
        } else {
            data.resource_name = read_name(stream, 260);
            for (auto& bits : data.resource_double_bits) bits = read_u64(stream);
        }
        data.final_value = read_u32(stream);
        child.class_two = std::move(data);
    } else if (child.class_tag == 3) {
        for (auto& bits : child.class_three_double_bits) bits = read_u64(stream);
    } else if (child.class_tag != 0) {
        throw std::invalid_argument("unrecovered child class tag");
    }
    for (auto& bits : child.following_double_bits) bits = read_u64(stream);
    child.following_name = read_name(stream, 128);
    for (auto& bits : child.final_double_bits) bits = read_u64(stream);
    child.nested_child_count = read_u32(stream);
    if (child.nested_child_count > (stream.bytes.size() - stream.cursor) / 4) {
        throw std::invalid_argument("invalid nested child count");
    }
    child.nested_children.reserve(child.nested_child_count);
    for (std::uint32_t i = 0; i < child.nested_child_count; ++i) {
        child.nested_children.push_back(read_child(stream, depth + 1));
    }
    return child;
}

std::vector<ObjectBasePrefix::Child> read_children(
    LevelByteStream& stream, std::uint32_t count) {
    if (count > (stream.bytes.size() - stream.cursor) / 4) {
        throw std::invalid_argument("invalid object child count");
    }
    std::vector<ObjectBasePrefix::Child> children;
    children.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        children.push_back(read_child(stream, 0));
    }
    return children;
}

ObjectBasePrefix::AuxiliaryRecord read_auxiliary_record(LevelByteStream& stream) {
    const auto start = stream.cursor;
    ObjectBasePrefix::AuxiliaryRecord record{};
    record.leading_double_bits = read_u64(stream);
    record.first_count = read_u32(stream);
    for (std::uint32_t i = 0; i < record.first_count; ++i) {
        read_u32(stream);
        skip_records(stream, read_u32(stream), 89);
    }
    record.second_count = read_u32(stream);
    for (std::uint32_t i = 0; i < record.second_count; ++i) {
        read_u32(stream);
        skip_records(stream, read_u32(stream), 40);
        read_u32(stream);
        skip_records(stream, read_u32(stream), 4);
        skip_records(stream, read_u32(stream), 4);
    }
    record.third_count = read_u32(stream);
    if (record.third_count > (stream.bytes.size() - stream.cursor) / 16) {
        throw std::invalid_argument("truncated auxiliary records");
    }
    // 0x47CDE7 reads the preorder target, axis selector and rate.
    record.spins.reserve(record.third_count);
    for (std::uint32_t i = 0; i < record.third_count; ++i) {
        record.spins.push_back({read_u32(stream), read_u32(stream), read_u64(stream)});
    }
    record.name = read_name(stream, 128);
    record.trailing_integer_count = read_u32(stream);
    skip_records(stream, record.trailing_integer_count, 4);
    record.serialized_bytes.assign(stream.bytes.begin() + start,
                                   stream.bytes.begin() + stream.cursor);
    return record;
}

std::vector<ObjectBasePrefix::AuxiliaryRecord> read_auxiliary_records(
    LevelByteStream& stream, std::uint32_t count) {
    if (count > (stream.bytes.size() - stream.cursor) / 24) {
        throw std::invalid_argument("invalid auxiliary record count");
    }
    std::vector<ObjectBasePrefix::AuxiliaryRecord> records;
    records.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        records.push_back(read_auxiliary_record(stream));
    }
    return records;
}
}

ObjectBasePrefix read_object_base_prefix(LevelByteStream& stream) {
    ObjectBasePrefix result{};
    result.source_extent.marker_begin = stream.cursor;
    result.marker_matches = read_name(stream, 255) == "Object 09";
    result.source_extent.typed_root_begin = stream.cursor;
    result.class_tag = read_u32(stream);
    if (result.class_tag != 0) {
        throw std::invalid_argument("unrecovered object base class tag");
    }
    for (auto& bits : result.double_bits) bits = read_u64(stream);
    result.name = read_name(stream, 128);
    for (auto& bits : result.final_double_bits) bits = read_u64(stream);
    result.child_count = read_u32(stream);
    result.children = read_children(stream, result.child_count);
    result.source_extent.tree_end = stream.cursor;
    result.first_flag = read_u8(stream) != 0;
    result.has_auxiliary_list = read_u8(stream) != 0;
    if (result.has_auxiliary_list) {
        result.auxiliary_count = read_u32(stream);
        result.auxiliary_records = read_auxiliary_records(stream, result.auxiliary_count);
    }
    result.source_extent.holder_end = stream.cursor;
    result.has_second_object = read_u8(stream) != 0;
    if (result.has_second_object) {
        auto& extent = result.second_object_source_extent.emplace();
        extent.marker_begin = stream.cursor;
        result.second_object_marker_matches = read_name(stream, 255) == "Object 09";
        extent.typed_root_begin = stream.cursor;
        result.second_object_class_tag = read_u32(stream);
        if (result.second_object_class_tag != 0) {
            throw std::invalid_argument("unrecovered second object class tag");
        }
        for (auto& bits : result.second_object_double_bits) bits = read_u64(stream);
        result.second_object_name = read_name(stream, 128);
        for (auto& bits : result.second_object_final_double_bits) bits = read_u64(stream);
        result.second_object_child_count = read_u32(stream);
        result.second_object_children =
            read_children(stream, result.second_object_child_count);
        extent.tree_end = stream.cursor;
        result.second_object_first_flag = read_u8(stream) != 0;
        result.second_object_has_auxiliary_list = read_u8(stream) != 0;
        if (result.second_object_has_auxiliary_list) {
            result.second_object_auxiliary_count = read_u32(stream);
            result.second_object_auxiliary_records =
                read_auxiliary_records(stream, result.second_object_auxiliary_count);
        }
        extent.holder_end = stream.cursor;
    }
    return result;
}

SerializedObject read_serialized_object(LevelByteStream& stream) {
    SerializedObject result{};
    result.source_extent.marker_begin = stream.cursor;
    result.marker_matches = read_name(stream, 255) == "Object 09";
    result.source_extent.typed_root_begin = stream.cursor;
    result.root = read_child(stream, 0);
    result.source_extent.tree_end = stream.cursor;
    result.first_flag = read_u8(stream) != 0;
    result.has_auxiliary_list = read_u8(stream) != 0;
    if (result.has_auxiliary_list) {
        result.auxiliary_records = read_auxiliary_records(stream, read_u32(stream));
    }
    result.source_extent.holder_end = stream.cursor;
    return result;
}

} // namespace lemmings
