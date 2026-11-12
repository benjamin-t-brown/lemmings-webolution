#include "lemmings/model_update.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

namespace {
struct Model {
    std::vector<int> events;
    double value = -1;
    lemmings::ModelUpdateState* state = nullptr;
    bool mutate_flag = false;
    std::uint8_t finished = 0;
};
const lemmings::ModelUpdateCallbacks callbacks{
    [](void* p,double) {
        auto& model=*static_cast<Model*>(p);
        model.events.push_back(1);
        if(model.mutate_flag) model.state->remove_when_finished=1;
    },
    [](void* p) {
        auto& model=*static_cast<Model*>(p);
        model.events.push_back(2);
        return model.finished;
    },
    [](void* p) {static_cast<Model*>(p)->events.push_back(3);return 0.75;},
    [](void* p,double value) {
        auto& model=*static_cast<Model*>(p);
        model.events.push_back(4);model.value=value;
    }
};
}
int main() {
    lemmings::ModelUpdateState state{};
    assert(lemmings::update_model_entity(state,0.1,callbacks));
    Model model{};
    model.state=&state;
    state.model=&model;
    model.mutate_flag=true;model.finished=255;
    assert(!lemmings::update_model_entity(state,0.1,callbacks));
    assert((model.events==std::vector<int>{1,2}));
    model.mutate_flag=false;model.finished=0;state.remove_when_finished=0;
    for(double remaining : {1.0,0.6,0.0,
                            std::numeric_limits<double>::infinity()}) {
        state.blink_mode=1;state.blink_remaining=remaining;model.events.clear();
        assert(lemmings::update_blinking_model(state,0.0,callbacks));
        assert(model.value==0.0);
        assert((model.events==std::vector<int>{4,1}));
    }
    state.blink_mode=1;state.blink_remaining=0.2;model.events.clear();
    assert(lemmings::update_blinking_model(state,0.0,callbacks));
    assert(model.value==0.75);
    assert((model.events==std::vector<int>{3,4,1}));
    state.blink_mode=2;state.blink_remaining=0.01;model.events.clear();
    assert(lemmings::update_blinking_model(state,0.02,callbacks));
    assert(state.blink_mode==0&&state.blink_remaining==0.0&&model.value==0.0);
    lemmings::enable_blinking_model(state);
    assert(state.enabled==1&&state.blink_mode==1&&state.blink_remaining==1.0);
    assert(lemmings::update_blinking_model(state,
        std::numeric_limits<double>::quiet_NaN(),callbacks));
    assert(state.blink_mode==0&&state.blink_remaining==0.0&&model.value==0.75);
    lemmings::disable_blinking_model(state);
    assert(state.enabled==0&&state.blink_mode==2&&state.blink_remaining==1.0);
}
