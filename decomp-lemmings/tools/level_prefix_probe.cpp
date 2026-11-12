// Batch probe for the recovered Level 23 prefix and first object head.
#include "lemmings/level_name_tables.hpp"
#include "lemmings/world_geometry_prefix.hpp"
#include "lemmings/object_base_prefix.hpp"
#include "lemmings/level_entity_prefix.hpp"
#include "lemmings/level_gadget_records.hpp"

#include <cstdint>
#include <iostream>
#include <span>
#include <vector>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace {
std::uint64_t hash_names(const std::vector<std::string>& names) {
    std::uint64_t hash = 14695981039346656037ull;
    for (const auto& name : names) {
        for (unsigned char byte : name) {
            hash = (hash ^ byte) * 1099511628211ull;
        }
        hash = (hash ^ 0xFFu) * 1099511628211ull;
    }
    return hash;
}

std::uint64_t hash_triples(
    const std::vector<std::array<std::uint64_t, 3>>& triples) {
    std::uint64_t hash = 14695981039346656037ull;
    for (const auto& triple : triples) {
        for (const auto bits : triple) {
            for (int i = 0; i < 8; ++i) {
                hash = (hash ^ ((bits >> (8 * i)) & 0xFFu)) * 1099511628211ull;
            }
        }
    }
    return hash;
}

std::uint64_t hash_polygon_records(
    const std::vector<lemmings::CompiledLevelPrefix::FirstObjectHead::ClassOneHead::PolygonFourRecord>& records) {
    std::uint64_t hash = 14695981039346656037ull;
    const auto add = [&](std::uint64_t value, int bytes) {
        for (int i = 0; i < bytes; ++i) {
            hash = (hash ^ ((value >> (8 * i)) & 0xFFu)) * 1099511628211ull;
        }
    };
    for (const auto& record : records) {
        add(record.leading_value, 4);
        for (auto bits : record.triple) add(bits, 8);
        add(record.scalar, 8);
        for (const auto& component : record.components) {
            add(component.value, 4);
            for (auto bits : component.pair) add(bits, 8);
        }
    }
    return hash;
}

template <std::size_t N>
std::uint64_t hash_double_bits(const std::array<std::uint64_t, N>& values) {
    std::uint64_t hash = 14695981039346656037ull;
    for (auto bits : values) {
        for (int i = 0; i < 8; ++i) {
            hash = (hash ^ ((bits >> (8 * i)) & 0xFFu)) * 1099511628211ull;
        }
    }
    return hash;
}

void write_class(const lemmings::CompiledLevelPrefix::FirstObjectHead::ClassOneHead& value) {
    std::cout << ' ' << value.first_triples.size()
              << ' ' << hash_triples(value.first_triples)
              << ' ' << value.second_triples.size()
              << ' ' << hash_triples(value.second_triples)
              << ' ' << value.named_references.size();
    for (const auto& reference : value.named_references) {
        std::cout << ' ' << hash_names({reference.name}) << ' ' << reference.value;
    }
    std::cout << ' ' << value.following_polygon_count;
    for (const auto& polygon : value.polygons) {
        std::cout << ' ' << polygon.allocated_bytes
                  << ' ' << polygon.marker_matches
                  << ' ' << polygon.type
                  << ' ' << polygon.record_count
                  << ' ' << polygon.extra_value
                  << ' ' << hash_polygon_records(polygon.type_four_records);
    }
    std::cout << ' ' << hash_double_bits(value.following_doubles)
              << ' ' << hash_names({value.following_name})
              << ' ' << hash_double_bits(value.final_doubles)
              << ' ' << value.following_nested_object_count;
    for (const auto& nested : value.nested_class_one) {
        std::cout << ' ' << 1;
        write_class(nested);
    }
}

std::uint64_t hash_world_triples(const lemmings::WorldGeometryPrefix& world) {
    std::uint64_t hash = 14695981039346656037ull;
    for (const auto& block : world.first_blocks) {
        for (const auto& triple : block) {
            for (const auto bits : triple) {
                for (int i = 0; i < 8; ++i) {
                    hash = (hash ^ ((bits >> (8 * i)) & 0xFFu)) * 1099511628211ull;
                }
            }
        }
    }
    return hash;
}

std::uint64_t hash_world_doubles(const lemmings::WorldGeometryPrefix& world) {
    std::uint64_t hash = 14695981039346656037ull;
    for (const auto& block : world.second_blocks) {
        for (const auto bits : block) {
            for (int i = 0; i < 8; ++i) {
                hash = (hash ^ ((bits >> (8 * i)) & 0xFFu)) * 1099511628211ull;
            }
        }
    }
    return hash;
}

struct ByteHash {
    std::uint64_t value = 14695981039346656037ull;
    void add(std::uint64_t bits, int bytes) {
        for (int i = 0; i < bytes; ++i) {
            value = (value ^ ((bits >> (8 * i)) & 0xFFu)) * 1099511628211ull;
        }
    }
};

std::uint64_t hash_base_polygon_records(
    const std::vector<lemmings::ObjectBasePrefix::Child::PolygonRecord>& records,
    std::uint32_t type) {
    ByteHash hash;
    for (const auto& record : records) {
        hash.add(record.leading_value, 4);
        if (type == 4 || type == 6) {
            for (auto bits : record.plane_double_bits) hash.add(bits, 8);
        }
        for (const auto& vertex : record.vertices) {
            hash.add(vertex.value, 4);
            for (int j = 0; j < (type == 4 || type == 6 ? 2 : 6); ++j) {
                hash.add(vertex.double_bits[j], 8);
            }
        }
    }
    return hash.value;
}

std::uint64_t hash_raw_bytes(const std::vector<std::uint8_t>& bytes) {
    ByteHash hash;
    for (auto byte : bytes) hash.add(byte, 1);
    return hash.value;
}

std::uint64_t hash_words(const std::vector<std::uint32_t>& values) {
    ByteHash hash;
    for (auto value : values) hash.add(value, 4);
    return hash.value;
}

std::uint64_t hash_integer_triples(const lemmings::WorldGeometryPrefix& world) {
    ByteHash hash;
    for (const auto& triple : world.integer_triples) {
        for (auto value : triple) hash.add(value, 4);
    }
    return hash.value;
}

std::uint64_t hash_world_tail(const lemmings::WorldGeometryPrefix& world) {
    ByteHash hash;
    for (const auto& list : world.tail_lists) {
        hash.add(list.size(), 4);
        for (const auto& record : list) {
            hash.add(record.value, 4);
            hash.add(record.first_short, 2);
            hash.add(record.second_short, 2);
            for (auto bits : record.double_bits) hash.add(bits, 8);
        }
    }
    return hash.value;
}
void write_object_child(const lemmings::ObjectBasePrefix::Child& child) {
    std::cout << ' ' << child.class_tag;
    if (child.class_tag == 1) {
        std::cout << ' ' << child.first_triples.size()
                  << ' ' << hash_triples(child.first_triples)
                  << ' ' << child.second_triples.size()
                  << ' ' << hash_triples(child.second_triples)
                  << ' ' << child.named_references.size();
        for (const auto& reference : child.named_references) {
            std::cout << ' ' << hash_names({reference.name}) << ' ' << reference.value;
        }
        std::cout << ' ' << child.polygon_count;
        for (const auto& polygon : child.polygons) {
            std::cout << ' ' << polygon.allocated_bytes
                      << ' ' << polygon.marker_matches
                      << ' ' << polygon.type
                      << ' ' << polygon.record_count
                      << ' ' << polygon.extra_value
                      << ' ' << hash_base_polygon_records(polygon.records, polygon.type);
        }
    } else if (child.class_tag == 2) {
        const auto& data = child.class_two.value();
        ByteHash values;
        for (auto value : data.resource_values) values.add(value, 4);
        std::cout << ' ' << hash_double_bits(data.leading_double_bits)
                  << ' ' << data.first_flag << ' ' << data.extended_resource
                  << ' ' << hash_names({data.resource_name})
                  << ' ' << (data.extended_resource ? values.value :
                             hash_double_bits(data.resource_double_bits))
                  << ' ' << data.final_value;
    } else if (child.class_tag == 3) {
        std::cout << ' ' << hash_double_bits(child.class_three_double_bits);
    }
    std::cout << ' ' << hash_double_bits(child.following_double_bits)
              << ' ' << hash_names({child.following_name})
              << ' ' << hash_double_bits(child.final_double_bits)
              << ' ' << child.nested_child_count;
    for (const auto& nested : child.nested_children) write_object_child(nested);
}

void write_object_auxiliary(const lemmings::ObjectBasePrefix::AuxiliaryRecord& record) {
    std::cout << ' ' << record.leading_double_bits
              << ' ' << record.first_count
              << ' ' << record.second_count
              << ' ' << record.third_count
              << ' ' << hash_names({record.name})
              << ' ' << record.trailing_integer_count
              << ' ' << hash_raw_bytes(record.serialized_bytes);
}

void write_entities(const lemmings::LevelEntityPrefix& entities) {
    std::cout << ' ' << entities.registry_size << ' ' << entities.groups.size();
    for (const auto& group : entities.groups) {
        ByteHash hash;
        for (const auto& record : group.records) {
            hash.add(record.leading_double_bits, 8);
            hash.add(record.value, 4);
            for (auto bits : record.double_bits) hash.add(bits, 8);
        }
        std::cout << ' ' << group.flag << ' ' << group.records.size()
                  << ' ' << hash.value << ' ' << group.integer_values.size()
                  << ' ' << hash_words(group.integer_values)
                  << ' ' << hash_names({group.name});
    }
    std::cout << ' ' << entities.name_pairs.size();
    for (const auto& pair : entities.name_pairs) {
        std::cout << ' ' << pair.flag << ' ' << hash_names({pair.first, pair.second});
    }
    for (int subclass = 0; subclass != 2; ++subclass) {
        const auto& actors = subclass ? entities.subclass_actors : entities.actors;
        ByteHash hash;
        for (const auto& actor : actors) {
            for (auto bits : actor.position_bits) hash.add(bits, 8);
            hash.add(actor.field_20_bits, 8);
            hash.add(actor.field_30_bits, 8);
            hash.add(actor.flag_38, 1);
            hash.add(actor.flag_28, 1);
            hash.add(static_cast<std::uint16_t>(actor.id), 2);
            for (auto value : actor.values_40_44) hash.add(value, 4);
            for (auto flag : actor.flags_370_48_49_4a) hash.add(flag, 1);
            hash.add(actor.value_4c, 4);
            hash.add(actor.flag_5c, 1);
            hash.add(actor.flag_5f, 1);
            for (auto bits : actor.pairs_68_350_360) hash.add(bits, 8);
            if (subclass) hash.add(actor.subclass_flag_3a4, 1);
        }
        std::cout << ' ' << actors.size() << ' ' << hash.value;
    }
}

}


template<class T> void emit(T value) { std::cout << ' ' << value; }
template<class T, std::size_t N> void emit_array(const std::array<T, N>& values) {
    for (auto value : values) emit(value);
}
void emit_name(const std::string& value) { emit(hash_names({value})); }

void write_model(const lemmings::LevelModel& value) {
    emit(value.type);
    if (value.type == 0) {
        emit_name(value.name);
        emit_array(value.double_bits);
        const auto& object = value.object.value();
        emit(object.marker_matches);
        write_object_child(object.root);
        emit(object.first_flag);
        emit(object.has_auxiliary_list);
        if (object.has_auxiliary_list) {
            emit(object.auxiliary_records.size());
            for (const auto& record : object.auxiliary_records) write_object_auxiliary(record);
        }
        emit(value.has_final_name);
        if (value.has_final_name) emit_name(value.final_name);
    } else if (value.type == 1) {
        emit_array(value.resource_values);
        emit_name(value.name);
    } else {
        emit_array(value.type_two_double_bits);
        emit(value.value_230);
        emit(value.records_1c8.size());
        for (const auto& record : value.records_1c8) emit_array(record);
        emit(value.allocation_count);
        emit(value.discarded_value);
        for (const auto& name : value.child_names) emit_name(name);
        emit(value.children.size());
        for (const auto& child : value.children) write_model(child);
    }
}
void write_base(const lemmings::EntityBaseFields& value) {
    emit_array(value.double_bits);
    emit(value.flag_38);
    emit(value.flag_28);
    emit(static_cast<std::uint16_t>(value.id));
}
void write_basic(const lemmings::ModelEntity& value) {
    write_base(value);
    emit(value.flag_40);
    emit(value.model.has_value());
    emit(value.flag_41);
    if (value.model) write_model(*value.model);
}
void write_spatial(const lemmings::SpatialEntity& value) {
    write_basic(value);
    emit_array(value.double_bits_48_60);
    for (const auto& triples : value.triple_arrays) {
        emit(triples.size());
        for (const auto& triple : triples) emit_array(triple);
    }
}
template<class T, class F>
void emit_list(const std::vector<T>& list, F writer) {
    emit(list.size());
    for (const auto& value : list) writer(value);
}
void write_gadgets(const lemmings::LevelGadgetRecords& value) {
    emit_list(value.basic_entities, write_basic);
    emit_list(value.linked_entities, [](const auto& record) {
        write_basic(record);
        emit_array(record.double_bits_60_68);
        emit(record.value_50);
        for (const auto& name : record.names) emit_name(name);
        emit_array(record.matrix_118);
        emit_array(record.triple_b8);
        emit(record.registry_index);
    });
    emit_list(value.entities_425e50, [](const auto& record) {
        write_spatial(record);
        emit(record.value_b0);
        emit_array(record.shorts_98_9a_9c);
        emit_array(record.values_a0_a4_a8);
        emit(record.flag_ac);
        emit(record.short_b4);
        emit(record.flag_b6);
        emit(record.flag_ad);
    });
    emit(value.value_5568);
    emit_list(value.entities_426c6b, [](const auto& record) {
        write_spatial(record);
        emit(record.value_98);
        emit(record.flag_b0);
    });
    emit_list(value.spatial_entities, write_spatial);
    emit_list(value.entities_424187, [](const auto& record) {
        write_base(record);
        emit(record.flag_58);
        emit_array(record.values);
    });
    emit_list(value.entities_42acac, [](const auto& record) {
        write_spatial(record);
        write_model(record.model_15c);
        for (const auto* model : {&record.model_160, &record.model_164}) {
            emit(model->has_value());
            if (*model) write_model(**model);
        }
        emit_array(record.values_18c_128_168);
        emit_array(record.triple_170);
        emit(record.value_16c);
        emit_array(record.doubles_98_1a8_a0);
        emit_name(record.name_a8);
        for (const auto& triple : record.triples) emit_array(triple);
        emit_name(record.resource_name);
        emit(record.value_140);
    });
    emit_list(value.entities_4347ec, [](const auto& record) {
        write_spatial(record);
        for (const auto& model : record.models_1c8_1cc) write_model(model);
        emit(record.value_98);
        emit(record.double_a0);
        emit_array(record.triple_1b0);
        for (const auto& name : record.names) emit_name(name);
    });
    emit_list(value.basic_4342bf, write_basic);
    emit_list(value.spatial_41e038, write_spatial);
    emit_list(value.tagged_entities, [](const auto& record) {
        emit(record.type);
        write_spatial(record);
        emit(record.double_98);
        emit(record.flag_b0);
        emit(record.flag_a0);
        emit(record.values[0]);
        if (record.type == 3 || record.type == 4) emit(record.values[1]);
        if (record.type <= 2) {
            for (unsigned i = 0; i < (record.type == 0 ? 1u : 2u); ++i) {
                emit(record.record_arrays[i].size());
                for (const auto& item : record.record_arrays[i]) emit_array(item);
            }
        } else if (record.type == 4) emit_array(record.type_four_record);
    });
    emit_list(value.named_entities, [](const auto& record) {
        emit_name(record.type);
        if (record.type != "STAT") emit_name(record.subclass_name);
        write_spatial(record);
        emit(record.flag_98);
        emit(record.double_a8);
        emit(record.value_c0);
        emit(record.flag_1d1);
        emit(record.flag_1d2);
        for (const auto& name : record.names) emit_name(name);
        emit(record.flag_1d0);
        emit(record.flag_1d3);
        if (record.type == "DYNA") {
            emit_array(record.dynamic_pairs);
            emit_array(record.dynamic_values);
            emit_array(record.subclass_doubles);
            emit(record.dynamic_flag_200);
        } else if (record.type == "FALL") {
            for (const auto& name : record.falling_references) emit_name(name);
            emit_array(record.subclass_doubles);
            for (const auto& name : record.falling_names) emit_name(name);
        }
    });
    emit_array(value.values_27d0);
    emit(value.values_1fb0.size());
    for (auto word : value.values_1fb0) emit(word);
    emit_array(value.flags_50cc);
    emit(value.short_50a8);
    emit(value.value_50a0);
    emit(value.final_records.size());
    for (const auto& record : value.final_records) emit_array(record);
}

int main() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#endif
    for (;;) {
        std::uint8_t size_bytes[4]{};
        std::cin.read(reinterpret_cast<char*>(size_bytes), 4);
        if (std::cin.eof() && std::cin.gcount() == 0) break;
        if (!std::cin || std::cin.gcount() != 4) return 2;
        const std::uint32_t size = static_cast<std::uint32_t>(size_bytes[0]) |
            (static_cast<std::uint32_t>(size_bytes[1]) << 8) |
            (static_cast<std::uint32_t>(size_bytes[2]) << 16) |
            (static_cast<std::uint32_t>(size_bytes[3]) << 24);
        if (size < 40 || size > 8'000'000) return 2;
        std::vector<std::uint8_t> bytes(size);
        std::cin.read(reinterpret_cast<char*>(bytes.data()), size);
        if (!std::cin) return 2;
        lemmings::LevelByteStream stream{std::span<const std::uint8_t>(bytes)};
        const auto prefix = lemmings::read_compiled_level_prefix(stream);
        std::cout << prefix.header.marker_matches << ' '
                  << prefix.header.fields[0] << ' '
                  << prefix.first_names.size() << ' '
                  << prefix.second_names.size() << ' '
                  << prefix.third_names.size() << ' '
                  << prefix.fourth_names.size() << ' '
                  << prefix.fifth_names.size() << ' '
                  << prefix.following_object_count << ' '
                  << stream.cursor << ' '
                  << hash_names(prefix.first_names) << ' '
                  << hash_names(prefix.second_names) << ' '
                  << hash_names(prefix.third_names) << ' '
                  << hash_names(prefix.fourth_names) << ' '
                  << hash_names(prefix.fifth_names) << ' '
                  << hash_names({prefix.following_short_string}) << ' '
                  << prefix.first_object_head.has_value();
        if (prefix.first_object_head) {
            const auto& head = *prefix.first_object_head;
            std::cout << ' ' << hash_names({head.outer_name})
                      << ' ' << head.leading_value
                      << ' ' << hash_names({head.inner_name});
            for (auto bits : head.serialized_double_bits) std::cout << ' ' << bits;
            std::cout << ' ' << head.marker_matches << ' ' << head.class_tag
                      << ' ' << head.class_one_head.has_value();
            if (head.class_one_head) {
                write_class(*head.class_one_head);
                std::cout << ' ' << head.first_object_flag
                          << ' ' << head.has_auxiliary_list
                          << ' ' << head.auxiliary_list_count
                          << ' ' << head.has_final_name
                          << ' ' << hash_names({head.final_name});
            }
        }
        const auto world = lemmings::read_world_geometry_prefix(stream);
        std::cout << ' ' << world.leading_value
                  << ' ' << world.following_double_bits
                  << ' ' << world.first_blocks.size()
                  << ' ' << hash_world_triples(world)
                  << ' ' << world.second_blocks.size()
                  << ' ' << hash_world_doubles(world)
                  << ' ' << world.following_class_leading_value;
        write_class(world.following_class_one);
        ByteHash byte_hash;
        for (auto byte : world.byte_array) byte_hash.add(byte, 1);
        ByteHash fixed_hash;
        for (auto byte : world.fixed_bytes) fixed_hash.add(byte, 1);
        std::cout << ' ' << world.following_value
                  << ' ' << world.following_double_bits_2
                  << ' ' << world.first_integer_array.size()
                  << ' ' << hash_words(world.first_integer_array)
                  << ' ' << world.integer_triples.size()
                  << ' ' << hash_integer_triples(world)
                  << ' ' << world.second_integer_array.size()
                  << ' ' << hash_words(world.second_integer_array)
                  << ' ' << world.byte_array.size()
                  << ' ' << byte_hash.value
                  << ' ' << fixed_hash.value
                  << ' ' << hash_names({world.first_name})
                  << ' ' << hash_names({world.second_name})
                  << ' ' << hash_world_tail(world)
                  << ' ' << stream.cursor;
        const auto object = lemmings::read_object_base_prefix(stream);
        std::cout << ' ' << object.marker_matches
                  << ' ' << object.class_tag
                  << ' ' << hash_double_bits(object.double_bits)
                  << ' ' << hash_names({object.name})
                  << ' ' << hash_double_bits(object.final_double_bits)
                  << ' ' << object.child_count
                  << ' ' << !object.children.empty();
        for (const auto& child : object.children) write_object_child(child);
        std::cout << ' ' << object.first_flag
                  << ' ' << object.has_auxiliary_list
                  << ' ' << object.auxiliary_count;
        for (const auto& record : object.auxiliary_records) write_object_auxiliary(record);
        std::cout << ' ' << object.has_second_object
                  << ' ' << object.second_object_marker_matches
                  << ' ' << object.second_object_class_tag;
        if (object.has_second_object) {
            std::cout << ' ' << hash_double_bits(object.second_object_double_bits)
                      << ' ' << hash_names({object.second_object_name})
                      << ' ' << hash_double_bits(object.second_object_final_double_bits)
                      << ' ' << object.second_object_child_count;
            for (const auto& child : object.second_object_children) write_object_child(child);
            std::cout << ' ' << object.second_object_first_flag
                      << ' ' << object.second_object_has_auxiliary_list
                      << ' ' << object.second_object_auxiliary_count;
            for (const auto& record : object.second_object_auxiliary_records) {
                write_object_auxiliary(record);
            }
        }
        std::cout << ' ' << stream.cursor;
        write_entities(lemmings::read_level_entity_prefix(stream));
        std::cout << ' ' << stream.cursor;
        write_gadgets(lemmings::read_level_gadget_records(stream));
        std::cout << ' ' << stream.cursor << '\n';
    }
}
