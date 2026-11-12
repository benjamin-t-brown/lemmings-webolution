#include "lemmings/actor_terminal.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>
using namespace lemmings;
namespace {
using Triple = std::array<double, 3>;
struct Effects : ActorTerminalEntryCallbacks, ActorTerminalUpdateCallbacks {
    int effects[2]{}, entities[2]{}, child{};
    unsigned allocated = 0, inserted = 0;
    std::vector<TerminalEffectKind> kinds;
    std::vector<Triple> positions, directions, transformed;
    std::vector<int> counts, modes;
    ActorTerminalState* state = nullptr;
    double* remaining = nullptr;
    bool mutate = false;
    std::uint8_t finished = 0;
    void event_44535b(TerminalActor& a, std::int16_t id, std::int32_t value, std::uint8_t flag) override {
        assert(a.flag_28 == 0 && id == -7 && value == 91 && flag == 0);
        a.category_4c = 3; // Category is read after this callback.
    }
    void* create_effect(TerminalEffectKind k) override { kinds.push_back(k); return &effects[allocated++]; }
    void set_count(void*, std::int32_t n) override { counts.push_back(n); }
    void set_position_08(void*, const Triple& p) override { positions.push_back(p); }
    void set_pair(void*, double a, double b) override { assert(a == 4 && b == 0.5); }
    void set_mode(void*, std::int32_t n) override { modes.push_back(n); }
    Triple world_position_43d56c(const Triple& p) override { transformed.push_back(p); return {p[0]*2, p[1]*3, p[2]*4}; }
    void set_direction_54(void*, const Triple& p) override { directions.push_back(p); }
    Triple sample_triple_403f40(std::uint32_t x, std::int32_t y) override { assert(x == 2 && y == 4); return {1,2,3}; }
    std::int32_t child_count_4018d0(void*) override { return 1; }
    void* child_4018b0(void*, std::int32_t i) override { assert(i == 0); return &child; }
    void set_child_triple_403820(void* p, const Triple& v) override { assert(p == &child && (v == Triple{1,2,3})); }
    void set_triple_4017d0(void*, const Triple& v) override { assert((v == Triple{255,127,127})); }
    void* create_entity_432080(const Triple& p, void*, std::uint8_t flag) override {
        assert((p == Triple{2.5,8,0.25}) && flag == 1); return nullptr;
    }
    void insert_entity_401890(void*& p) override { assert(p == nullptr); ++inserted; }
    void advance_model_2c(TerminalActor&, double elapsed) override {
        assert(elapsed == 0.5);
        if (mutate) { *remaining = 0.75; state->elapsed_08 = 1.25; }
    }
    std::uint8_t finished_model_44(TerminalActor&) override {
        assert(*remaining == 0 && !std::signbit(*remaining)); return finished;
    }
};
}
int main() {
    ActorTerminalState state{99}; ActorBehavior behavior;
    initialize_actor_terminal(state, behavior);
    assert(state.elapsed_08 == 0 && behavior.type_14 == 0x1a && behavior.phase_10 == 0 && behavior.extra_11 == 0);
    TerminalActor actor; actor.position_08 = {2.5,100,0.25}; actor.direction_44 = -1;
    actor.identifier_3a = -7; actor.flag_28 = 5;
    Effects cb;
    enter_actor_terminal(actor, 8, 91, cb);
    assert(actor.position_08[1] == 100 && actor.flag_28 == 0 && actor.category_4c == 3);
    assert((cb.kinds == std::vector{TerminalEffectKind::effect_4072a0, TerminalEffectKind::effect_402870}));
    assert((cb.positions == std::vector<Triple>{{2.5,8,0.25},{2.5,4,0.25}}));
    assert((cb.directions == std::vector<Triple>{{0,-12,0},{0,-12,0}}));
    assert((cb.transformed == std::vector<Triple>{{2.5,8,0.25},{2.5,4,0.25},{2.5,8,0.25},{2.5,4,0.25}}));
    assert((cb.counts == std::vector{6,10}) && (cb.modes == std::vector{5,5}) && cb.inserted == 2);
    cb.state = &state; double remaining = 0.5; cb.remaining = &remaining;
    assert(update_actor_terminal(state, actor, remaining, cb) == 0 && state.elapsed_08 == 0.5);
    remaining = 0.5; cb.mutate = true; cb.finished = 0x80;
    assert(update_actor_terminal(state, actor, remaining, cb) == 2 && state.elapsed_08 == 2);
    leave_actor_terminal();
}
