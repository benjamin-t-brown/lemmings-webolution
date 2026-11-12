#pragma once
#include <array>
#include <cstdint>

namespace lemmings {
struct WorldUpdateEntity;
struct WorldEntityOperations {
    std::uint8_t (*update_14)(WorldUpdateEntity&, double elapsed);
    void (*deactivate_08)(WorldUpdateEntity&);
};
struct WorldUpdateEntity {
    const WorldEntityOperations* operations = nullptr;
    void* context = nullptr;
    std::uint8_t enabled_38 = 0;
    std::int16_t identifier_3a = 0;
    void* primary_behavior_340 = nullptr;
};
struct WorldEntityNode {
    WorldUpdateEntity* entity = nullptr;
    WorldEntityNode* next = nullptr;
    WorldEntityNode* previous = nullptr;
};
struct WorldEntityList {
    WorldEntityNode begin, end;
    WorldEntityList();
    WorldEntityList(const WorldEntityList&)=delete;
    WorldEntityList& operator=(const WorldEntityList&)=delete;
};
struct WorldEntityArray {
    std::int32_t count=0;
    WorldUpdateEntity** data=nullptr;
};
enum class WorldArray : unsigned {
    array_1eb8, array_1ef0, array_1f00, array_1f50, array_1f60,
    array_1f70, array_1f90, array_1f20, array_1fa0
};
struct WorldEntityPass {
    WorldEntityList list_1ea0, list_1ec8;
    std::array<WorldEntityArray,9> arrays;
    WorldEntityArray& array(WorldArray kind) {return arrays[static_cast<unsigned>(kind)];}
};
struct WorldEntityPassHooks {
    void* context=nullptr;
    void (*enqueue_4090b0)(void*,WorldUpdateEntity&)=nullptr;
    void (*unlink_list_node)(void*,WorldEntityNode&,bool actor_list)=nullptr;
    void (*actor_removed_notice)(void*,std::int16_t identifier)=nullptr;
    void (*erase_array_index_4465d0)(void*,std::int32_t index)=nullptr;
    // Pointer to the live entry, as passed to 0x446C70 after disposal callback.
    void (*erase_array_entry_446c70)(void*,WorldUpdateEntity** entry)=nullptr;
    void (*register_43b536)(void*,WorldUpdateEntity&)=nullptr;
};
// Original [0x442B00,0x443160) block in 0x4423D2: +1EA0 through +1FA0.
// Calls virtual update even for nonpositive/nonfinite elapsed. Caches linked
// next before update; dynamic array counts and selected entries are reread.
// Callbacks own disposal/list removal/array removal/registration, preserving
// original valid-lifetime and traversal preconditions. Other world work omitted.
void update_world_entity_collections(WorldEntityPass&,double elapsed,const WorldEntityPassHooks&);
} // namespace lemmings
