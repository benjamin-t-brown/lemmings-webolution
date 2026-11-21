#include "lemmings/level_gadget_records.hpp"

#include <bit>
#include <stdexcept>

namespace lemmings {
namespace {
class Reader {
public:
    LevelByteStream& stream;
    std::uint64_t integer(std::size_t width) {
        if (stream.cursor > stream.bytes.size() ||
            width > stream.bytes.size() - stream.cursor) {
            throw std::invalid_argument("truncated gadget field");
        }
        std::uint64_t bits = 0;
        for (std::size_t i = 0; i < width; ++i) {
            bits |= static_cast<std::uint64_t>(stream.bytes[stream.cursor++]) << (8 * i);
        }
        return bits;
    }
    std::uint32_t word() { return static_cast<std::uint32_t>(integer(4)); }
    std::uint16_t short_word() { return static_cast<std::uint16_t>(integer(2)); }
    bool flag() { return integer(1) != 0; }
    std::string name(int capacity = 128) {
        char bytes[260]{};
        read_level_string(stream, bytes, capacity);
        return bytes;
    }
    template<class T, std::size_t N>
    void array(std::array<T, N>& values) {
        for (auto& value : values) value = static_cast<T>(integer(sizeof(T)));
    }
    template<class T, class F>
    std::vector<T> records(F read, std::size_t minimum = 4) {
        const auto count = word();
        if (count > (stream.bytes.size() - stream.cursor) / minimum) {
            throw std::invalid_argument("invalid gadget record count");
        }
        std::vector<T> result;
        result.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i) result.push_back(read());
        return result;
    }
    EntityBaseFields base() { // 0x00423291
        EntityBaseFields value{};
        array(value.double_bits);
        value.flag_38 = flag();
        value.flag_28 = flag();
        value.id = std::bit_cast<std::int16_t>(short_word());
        return value;
    }
    ModelEntity model_entity() { // 0x00432283
        ModelEntity value{};
        static_cast<EntityBaseFields&>(value) = base();
        value.flag_40 = flag();
        const auto has_model = flag();
        value.flag_41 = flag();
        if (has_model) value.model = read_level_model(stream);
        return value;
    }
    SpatialEntity spatial() { // 0x0043130E
        SpatialEntity value{};
        static_cast<ModelEntity&>(value) = model_entity();
        array(value.double_bits_48_60);
        for (auto& list : value.triple_arrays) {
            list = records<std::array<std::uint64_t, 3>>([&] {
                std::array<std::uint64_t, 3> triple{};
                array(triple);
                return triple;
            }, 24);
        }
        return value;
    }
    Entity42da25 linked() {
        Entity42da25 value{};
        static_cast<ModelEntity&>(value) = model_entity();
        array(value.double_bits_60_68);
        value.value_50 = word();
        for (auto& text : value.names) text = name();
        array(value.matrix_118);
        array(value.triple_b8);
        value.registry_index = word();
        return value;
    }
    Entity425e50 read_425e50() {
        Entity425e50 value{};
        static_cast<SpatialEntity&>(value) = spatial();
        value.value_b0 = word();
        array(value.shorts_98_9a_9c);
        array(value.values_a0_a4_a8);
        value.flag_ac = flag();
        value.short_b4 = short_word();
        value.flag_b6 = flag();
        value.flag_ad = flag();
        return value;
    }
    Entity426c6b read_426c6b() {
        Entity426c6b value{};
        static_cast<SpatialEntity&>(value) = spatial();
        value.value_98 = word();
        value.flag_b0 = word() != 0;
        return value;
    }
    Entity424187 read_424187() {
        Entity424187 value{};
        static_cast<EntityBaseFields&>(value) = base();
        value.flag_58 = flag();
        array(value.values);
        return value;
    }
    Entity42acac read_42acac() {
        Entity42acac value{};
        static_cast<SpatialEntity&>(value) = spatial();
        value.model_15c = read_level_model(stream);
        if (flag()) value.model_160 = read_level_model(stream);
        if (flag()) value.model_164 = read_level_model(stream);
        array(value.values_18c_128_168);
        array(value.triple_170);
        value.value_16c = word();
        array(value.doubles_98_1a8_a0);
        value.name_a8 = name();
        for (auto& triple : value.triples) array(triple);
        value.resource_name = name(260);
        value.value_140 = word();
        return value;
    }
    Entity4347ec read_4347ec() {
        Entity4347ec value{};
        static_cast<SpatialEntity&>(value) = spatial();
        for (auto& model : value.models_1c8_1cc) model = read_level_model(stream);
        value.value_98 = word();
        value.double_a0 = integer(8);
        array(value.triple_1b0);
        for (auto& text : value.names) text = name();
        return value;
    }
    TaggedEntity tagged() {
        TaggedEntity value{};
        value.type = word(); // 0x00433DD9 dispatch.
        if (value.type > 4) throw std::invalid_argument("unrecovered tagged entity type");
        static_cast<SpatialEntity&>(value) = spatial();
        value.double_98 = integer(8); // 0x00433CEE shared base.
        value.flag_b0 = flag();
        value.flag_a0 = flag();
        value.values[0] = word();
        if (value.type == 3 || value.type == 4) value.values[1] = word();
        if (value.type <= 2) {
            const int arrays = value.type == 0 ? 1 : 2;
            for (int i = 0; i < arrays; ++i) {
                value.record_arrays[i] = records<std::array<std::uint32_t, 4>>([&] {
                    std::array<std::uint32_t, 4> record{};
                    array(record);
                    return record;
                }, 16);
            }
        } else if (value.type == 4) {
            array(value.type_four_record); // 0x00433FB9.
        }
        return value;
    }
    NamedEntity named() {
        NamedEntity value{};
        value.type = name(255);
        if (value.type != "STAT" && value.type != "DYNA" && value.type != "FALL") {
            throw std::invalid_argument("unrecovered named entity type");
        }
        if (value.type != "STAT") value.subclass_name = name(255);
        static_cast<SpatialEntity&>(value) = spatial();
        value.flag_98 = flag(); // 0x004364F5.
        value.double_a8 = integer(8);
        value.value_c0 = word();
        value.flag_1d1 = flag(); // 0x004333D2.
        value.flag_1d2 = flag();
        for (auto& text : value.names) text = name();
        value.flag_1d0 = flag();
        value.flag_1d3 = flag();
        if (value.type == "DYNA") { // 0x0042576F.
            array(value.dynamic_pairs);
            array(value.dynamic_values);
            array(value.subclass_doubles);
            value.dynamic_flag_200 = flag();
        } else if (value.type == "FALL") { // 0x00427CCD.
            for (auto& text : value.falling_references) text = name();
            array(value.subclass_doubles);
            for (auto& text : value.falling_names) text = name();
        }
        return value;
    }
};
}

LevelGadgetRecords read_level_gadget_records(LevelByteStream& stream) {
    Reader reader{stream};
    LevelGadgetRecords result{};
    result.basic_entities = reader.records<ModelEntity>([&] { return reader.model_entity(); }, 47);
    result.linked_entities = reader.records<Entity42da25>([&] { return reader.linked(); }, 175);
    result.entities_425e50 = reader.records<Entity425e50>([&] { return reader.read_425e50(); }, 130);
    result.value_5568 = reader.word();
    result.entities_426c6b = reader.records<Entity426c6b>([&] { return reader.read_426c6b(); }, 111);
    result.spatial_entities = reader.records<SpatialEntity>([&] { return reader.spatial(); }, 103);
    result.entities_424187 = reader.records<Entity424187>([&] { return reader.read_424187(); }, 85);
    result.entities_42acac = reader.records<Entity42acac>([&] { return reader.read_42acac(); });
    result.entities_4347ec = reader.records<Entity4347ec>([&] { return reader.read_4347ec(); });
    result.basic_4342bf = reader.records<ModelEntity>([&] { return reader.model_entity(); }, 47);
    result.spatial_41e038 = reader.records<SpatialEntity>([&] { return reader.spatial(); }, 103);
    result.tagged_entities = reader.records<TaggedEntity>([&] { return reader.tagged(); });
    result.named_entities = reader.records<NamedEntity>([&] { return reader.named(); });
    reader.array(result.values_27d0);
    result.values_1fb0 = reader.records<std::uint32_t>([&] { return reader.word(); });
    for (auto& flag : result.flags_50cc) flag = reader.flag();
    result.short_50a8 = reader.short_word();
    result.value_50a0 = reader.word();
    result.final_records = reader.records<std::array<std::uint32_t, 3>>([&] {
        std::array<std::uint32_t, 3> record{};
        reader.array(record);
        return record;
    }, 12);
    return result;
}

} // namespace lemmings
