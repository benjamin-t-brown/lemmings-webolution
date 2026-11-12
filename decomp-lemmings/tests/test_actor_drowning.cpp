#include "lemmings/actor_drowning.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <string>

namespace {
struct Context {
    std::string name;
    double advanced = 0;
    lemmings::ActorDrowningState* state = nullptr;
    double* remaining = nullptr;
    bool mutate = false;
};
void select(void* pointer,const char* name) {static_cast<Context*>(pointer)->name=name;}
void advance(void* pointer,double elapsed) {
    auto& context=*static_cast<Context*>(pointer);context.advanced=elapsed;
    if(context.mutate){context.state->elapsed_08=1.5;*context.remaining=0.75;}
}
}
int main() {
    using namespace lemmings;
    ActorDrowningState state{99};ActorBehavior behavior;
    initialize_actor_drowning(state,behavior);
    assert(state.elapsed_08==0&&behavior.phase_10==0&&behavior.extra_11==0&&behavior.type_14==0x22);
    Context context;const ActorDrowningHooks hooks{&context,select,advance};
    enter_actor_drowning(hooks);assert(context.name=="drown");
    for(double elapsed:{0.0,1.0,2.0,std::nextafter(2.0,3.0),-1.0,
        std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()}) {
        state.elapsed_08=elapsed;double remaining=0;
        assert(update_actor_drowning(state,remaining,hooks)==(elapsed>2.0?2u:0u));
        assert(remaining==0&&!std::signbit(remaining));
    }
    state.elapsed_08=1;double remaining=0.5;context.state=&state;context.remaining=&remaining;context.mutate=true;
    assert(update_actor_drowning(state,remaining,hooks)==2&&state.elapsed_08==2.25&&context.advanced==0.5&&remaining==0);
    context.mutate=false;state.elapsed_08=1.25;
    assert(update_actor_drowning(state,state.elapsed_08,hooks)==0&&state.elapsed_08==0);
    leave_actor_drowning();
}
