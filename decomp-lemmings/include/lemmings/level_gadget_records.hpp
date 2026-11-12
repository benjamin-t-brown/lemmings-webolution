#pragma once

#include "lemmings/level_model.hpp"

namespace lemmings {

// Names based on original reader addresses are intentional until class
// semantics can be established. Suffixes on fields are original object offsets.
struct EntityBaseFields {
    std::array<std::uint64_t, 5> double_bits{}; // +8,+10,+18,+20,+30
    bool flag_38 = false;
    bool flag_28 = false;
    std::int16_t id = 0;
};
struct ModelEntity : EntityBaseFields {
    bool flag_40 = false;
    bool flag_41 = false;
    std::optional<LevelModel> model;
};
struct SpatialEntity : ModelEntity {
    std::array<std::uint64_t, 6> double_bits_48_60{};
    std::array<std::vector<std::array<std::uint64_t, 3>>, 2> triple_arrays;
};
struct Entity42da25 : ModelEntity {
    std::array<std::uint64_t, 2> double_bits_60_68{};
    std::uint32_t value_50 = 0;
    std::array<std::string, 2> names;
    std::array<std::uint64_t, 9> matrix_118{};
    std::array<std::uint64_t, 3> triple_b8{};
    std::uint32_t registry_index = 0;
};
struct Entity425e50 : SpatialEntity {
    std::uint32_t value_b0 = 0;
    std::array<std::uint16_t, 3> shorts_98_9a_9c{};
    std::array<std::uint32_t, 3> values_a0_a4_a8{};
    bool flag_ac = false;
    std::uint16_t short_b4 = 0;
    bool flag_b6 = false;
    bool flag_ad = false;
};
struct Entity426c6b : SpatialEntity {
    std::uint32_t value_98 = 0;
    bool flag_b0 = false; // Serialized as four bytes.
};
struct Entity424187 : EntityBaseFields {
    bool flag_58 = false;
    // +40,+44,+48,+4C,+B4,+A0,+AC,+B8,+BC,+A8, in file order.
    std::array<std::uint32_t, 10> values{};
};
struct Entity42acac : SpatialEntity {
    LevelModel model_15c;
    std::optional<LevelModel> model_160;
    std::optional<LevelModel> model_164;
    std::array<std::uint32_t, 3> values_18c_128_168{};
    std::array<std::uint64_t, 3> triple_170{};
    std::uint32_t value_16c = 0;
    std::array<std::uint64_t, 3> doubles_98_1a8_a0{};
    std::string name_a8;
    std::array<std::array<std::uint64_t, 3>, 10> triples;
    std::string resource_name;
    std::uint32_t value_140 = 0;
};
struct Entity4347ec : SpatialEntity {
    std::array<LevelModel, 2> models_1c8_1cc;
    std::uint32_t value_98 = 0;
    std::uint64_t double_a0 = 0;
    std::array<std::uint64_t, 3> triple_1b0{};
    std::array<std::string, 4> names;
};
struct TaggedEntity : SpatialEntity {
    std::uint32_t type = 0;
    std::uint64_t double_98 = 0;
    bool flag_b0 = false;
    bool flag_a0 = false;
    std::array<std::uint32_t, 2> values{};
    std::array<std::vector<std::array<std::uint32_t, 4>>, 2> record_arrays;
    std::array<std::uint32_t, 4> type_four_record{};
};
struct NamedEntity : SpatialEntity {
    std::string type; // STAT, DYNA or FALL, from executable startup constants.
    std::string subclass_name; // DYNA/FALL only.
    bool flag_98 = false;
    std::uint64_t double_a8 = 0;
    std::uint32_t value_c0 = 0;
    bool flag_1d1 = false;
    bool flag_1d2 = false;
    std::array<std::string, 4> names;
    bool flag_1d0 = false;
    bool flag_1d3 = false;
    std::array<std::uint64_t, 4> dynamic_pairs{};
    std::array<std::uint32_t, 4> dynamic_values{};
    std::array<std::uint64_t, 2> subclass_doubles{};
    bool dynamic_flag_200 = false;
    std::array<std::string, 2> falling_references;
    std::array<std::string, 2> falling_names;
};
struct LevelGadgetRecords {
    std::vector<ModelEntity> basic_entities;       // 0x448650
    std::vector<Entity42da25> linked_entities;     // 0x448710
    std::vector<Entity425e50> entities_425e50;     // 0x4487F0
    std::uint32_t value_5568 = 0;
    std::vector<Entity426c6b> entities_426c6b;     // 0x4488F0
    std::vector<SpatialEntity> spatial_entities;   // 0x4489F0
    std::vector<Entity424187> entities_424187;     // 0x448AF0
    std::vector<Entity42acac> entities_42acac;     // 0x448BD0
    std::vector<Entity4347ec> entities_4347ec;     // 0x448CB0
    std::vector<ModelEntity> basic_4342bf;         // 0x448D90
    std::vector<SpatialEntity> spatial_41e038;     // 0x448E70
    std::vector<TaggedEntity> tagged_entities;     // 0x433DD9
    std::vector<NamedEntity> named_entities;
    std::array<std::uint32_t, 11> values_27d0{};
    std::vector<std::uint32_t> values_1fb0;
    std::array<bool, 5> flags_50cc{};
    std::uint16_t short_50a8 = 0;
    std::uint32_t value_50a0 = 0;
    std::vector<std::array<std::uint32_t, 3>> final_records;
};

LevelGadgetRecords read_level_gadget_records(LevelByteStream& stream);

} // namespace lemmings
