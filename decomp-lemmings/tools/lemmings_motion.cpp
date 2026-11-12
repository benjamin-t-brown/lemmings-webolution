#include "lemmings/actor_falling.hpp"
#include "lemmings/actor_landing.hpp"
#include "lemmings/actor_motion.hpp"
#include "lemmings/actor_walking_collision.hpp"
#include "lemmings/compiled_level.hpp"
#include "lemmings/sprite_resource.hpp"
#include "lemmings/world_coordinates.hpp"
#include "lemmings/world_state.hpp"

#include <bit>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>

// Diagnostic composition of recovered primary-state routines. This is not
// the original actor constructor, scheduler, world loop, or resource manager.
namespace {
using namespace lemmings;
struct Boundary : std::runtime_error { using std::runtime_error::runtime_error; };
enum class Motion { falling, walking, terminal, bounce, outside };
const char* name(Motion state) {
    switch(state) {
    case Motion::falling:return "falling";
    case Motion::walking:return "walking";
    case Motion::terminal:return "terminal_1a";
    case Motion::bounce:return "bounce_21";
    default:return "outside";
    }
}
struct Trace final : ActorFallingCallbacks, ActorWalkingCollisionCallbacks {
    CollisionGridHead grid;
    WorldCoordinateMesh mesh;
    AnimationPlayback animation;
    FallingActor fall;
    WalkingActor walk;
    ActorFallingState falling_state;
    ActorWalkingState walking_state;
    ActorBehavior placeholder;
    Motion state=Motion::falling;
    std::optional<Motion> pending;
    double remaining_budget=0;
    std::string clip;

    void select(std::string_view value) {
        clip=value;
        if(find_animation_clip(*animation.resource_1f8,clip.c_str()) ==
            static_cast<std::int32_t>(animation.resource_1f8->clips.size()))
            throw Boundary("selected_sprite_lacks_animation");
        select_animation(animation,clip.c_str());
    }
    std::uint8_t has_state_42ecc9(FallingActor&,std::int32_t) override {return 0;}
    std::uint8_t has_state_42ecc9(WalkingActor&,std::int32_t) override {return 0;}
    void select_animation_42fd25(FallingActor&,std::string_view v) override {select(v);}
    void select_animation_42fd25(WalkingActor&,std::string_view v) override {select(v);}
    long double model_duration_4c(void*) override {return animation_duration(animation);}
    long double model_elapsed_48(void*) override {return animation_time(animation);}
    void log_umbrella_time_40f860(double) override {}
    long double height_43da38(std::array<double,3>& p,double y) override {
        return world_depth_at_y(mesh,p,y);
    }
    void advance_model_2c(void*,double dt) override {advance_animation(animation,dt);}
    std::int32_t landing_416433(ActorFallingState& s,FallingActor& a) override {
        ActorLandingHooks hooks;
        hooks.context=this;
        hooks.bounce_notice=[](void*,std::int32_t,std::int32_t){};
        hooks.create_state=[](void* p,ActorLandingTransition type) {
            auto& self=*static_cast<Trace*>(p);
            self.pending=type==ActorLandingTransition::state_40b3a0 ? Motion::terminal :
                         type==ActorLandingTransition::bounce_40ec7e ? Motion::bounce : Motion::walking;
            return &self.placeholder;
        };
        hooks.attach_state=[](void*,ActorBehavior*){};
        hooks.select_landed_state=[](void* p){static_cast<Trace*>(p)->pending=Motion::walking;};
        return resolve_actor_landing(a.geometry,s.distance_18,a.umbrella_49,grid,hooks) ? 1:0;
    }
    [[noreturn]] static void gadget_boundary() {throw Boundary("gadget_runtime_required");}
    std::int32_t gadget_count_41cb60() override {gadget_boundary();}
    void* gadget_at_41cb40(std::int32_t) override {gadget_boundary();}
    std::uint8_t gadget_contains_430ba1(void*,const std::array<double,3>&) override {gadget_boundary();}
    std::uint8_t gadget_enabled_41cba0(void*) override {gadget_boundary();}
    void activate_gadget_20(void*) override {gadget_boundary();}
    void set_gadget_actor_a4(void*,WalkingActor&) override {gadget_boundary();}
    std::int32_t gadget_mode_128(void*) override {gadget_boundary();}
    std::string_view gadget_animation_406500(void*) override {gadget_boundary();}
    void clear_states_42e78a(WalkingActor&) override {} // Scenario has no extras.
    ActorBehavior* create_state(WalkingCollisionTransition type) override {
        if(type!=WalkingCollisionTransition::falling_415fa0)
            throw Boundary("additional_behavior_required");
        pending=Motion::falling;
        return &placeholder;
    }
    void attach_42e696(WalkingActor&,ActorBehavior*) override {}
    void enter_state_08(ActorBehavior*,WalkingActor&) override {
        throw Boundary("additional_behavior_required");
    }
    void move_y_42ee74(WalkingActor& a,double y) override {move_actor_sample_y(a.geometry,y,grid);}
    ActorWalkingHooks walking_hooks() {
        ActorWalkingHooks h;
        h.context=this;
        h.set_animation=[](void* p,WalkingActor&,std::string_view v){static_cast<Trace*>(p)->select(v);};
        h.advance_animation=[](void* p,WalkingActor&,double dt){advance_animation(static_cast<Trace*>(p)->animation,dt);};
        h.collision=[](void* p,ActorWalkingState&,WalkingActor& a){
            auto& self=*static_cast<Trace*>(p);
            return resolve_actor_walking_collision(a,self.grid,self);
        };
        h.surface=[](void* p,ActorWalkingState&,WalkingActor& a){
            ActorWalkingSurfaceHooks s;
            s.gadget_count=[](void*)->std::int32_t{gadget_boundary();};
            s.gadget_at=[](void*,std::int32_t)->void*{gadget_boundary();};
            s.gadget_contains=[](void*,void*,const std::array<double,3>&)->std::uint8_t{gadget_boundary();};
            track_actor_walking_surface(a,static_cast<Trace*>(p)->grid,s);
        };
        h.gadget_contact=[](void*,void*,WalkingActor&)->bool{gadget_boundary();};
        // Original startup initializer 0x42F93C sets these two targets.
        h.finish=[](void*,WalkingActor& a){approach_actor_walking_depth(a,0.25,0.75);};
        return h;
    }
    const ActorGeometryState& geometry() const {return state==Motion::falling ? fall.geometry : walk.geometry;}
    bool tick(double dt) {
        remaining_budget=dt;
        unsigned primary_updates=0;
        while(remaining_budget>0) {
            // Driver safeguard; this is not an instruction from the game.
            if(++primary_updates>64) throw Boundary("primary_update_limit");
            pending.reset();
            const auto result=state==Motion::falling ?
                update_actor_falling(falling_state,fall,remaining_budget,grid,*this) :
                update_actor_walking(walking_state,walk,remaining_budget,walking_hooks());
            if(result==0) return true;
            if(result==2) {walk.geometry=geometry();state=Motion::outside;return false;}
            if(result!=1) throw Boundary("unhandled_update_result");
            if(!pending) throw Boundary("transition_without_target");
            const auto p=geometry();
            // The diagnostic projects supported attachment requests after the
            // current primary update returns. It does not run the scheduler.
            state=*pending;
            pending.reset();
            if(state==Motion::walking) {
                walk.geometry=p; walking_state={}; enter_actor_walking(walk,walking_hooks());
            } else if(state==Motion::falling) {
                fall.geometry=p; falling_state={}; enter_actor_falling(fall,*this);
            } else {walk.geometry=p; return false;}
        }
        return true;
    }
};

void usage() {
    std::cout << "Usage: lemmings_motion --archive <lemmings.box> [--patch <patch.box>]\n"
                 " --level <entry.LVL> --sprite <entry.SPR> [--emitter 0] [--ticks 600] [--dt 0.02]\n"
                 "Primary falling/walking diagnostic; stops at unsupported behavior or gadget.\n"
                 "Reads assets in place and writes CSV. Does not run the complete game.\n";
}
template<class Number>
Number parse_number(std::string_view option, std::string_view value) {
    Number result{};
    const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
    if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size())
        throw std::invalid_argument(std::string(option)+": invalid numeric value: "+std::string(value));
    return result;
}
}
int main(int argc,char** argv) {
    try {
        std::string base,patch,level_name,sprite_name;
        int emitter=0,ticks=600; double dt=0.02;
        for(int i=1;i<argc;++i) {
            const std::string arg=argv[i];
            if(arg=="--help" || arg=="-h") {usage();return 0;}
            if(i+1==argc) throw std::invalid_argument("missing option value");
            const std::string value=argv[++i];
            if(arg=="--archive") base=value;
            else if(arg=="--patch") patch=value;
            else if(arg=="--level") level_name=value;
            else if(arg=="--sprite") sprite_name=value;
            else if(arg=="--emitter") emitter=parse_number<int>(arg,value);
            else if(arg=="--ticks") ticks=parse_number<int>(arg,value);
            else if(arg=="--dt") dt=parse_number<double>(arg,value);
            else throw std::invalid_argument("unknown option: "+arg);
        }
        if(base.empty() || level_name.empty() || sprite_name.empty() || emitter<0 ||
           ticks<0 || ticks>1000000 || !(dt>0 && dt<=1) || !std::isfinite(dt))
            throw std::invalid_argument("invalid arguments; see --help");
        std::vector<BoxArchiveIndex> archives;
        if(!patch.empty()) archives.push_back(read_box_archive_index(patch));
        archives.push_back(read_box_archive_index(base));
        auto loaded=load_compiled_level(archives,level_name);
        auto cells=collision_grid_from_level(loaded.level.world);
        const auto location=find_box_entry(archives,sprite_name);
        if(!location) throw std::invalid_argument("sprite entry not found");
        const auto sprite=parse_sprite_resource(archives[location->archive].read_entry(location->entry));
        const auto clips=sprite_animation_clips(sprite);
        const AnimationResource resource{clips};
        const auto& emitters=loaded.level.gadgets.entities_425e50;
        if(static_cast<std::size_t>(emitter)>=emitters.size())
            throw std::invalid_argument("emitter index outside level's "+std::to_string(emitters.size())+" emitter records");
        const auto& source=emitters[static_cast<std::size_t>(emitter)];
        Trace trace;
        trace.grid=cells.view(); trace.mesh=world_coordinate_mesh(loaded.level.world);
        trace.animation.resource_1f8=&resource;
        auto& actor=trace.fall.geometry;
        actor.position={static_cast<double>(std::bit_cast<std::int16_t>(source.shorts_98_9a_9c[1])),
                        static_cast<double>(std::bit_cast<std::int16_t>(source.shorts_98_9a_9c[2])),0.5};
        actor.category_4c=std::bit_cast<std::int32_t>(source.value_b0);
        actor.pair_350={0,3}; actor.pair_360={0,0};
        if(source.flag_ad) reverse_actor_direction_44(actor);
        if(std::bit_cast<std::int16_t>(source.short_b4)==-1) reverse_actor_direction_40(actor);
        trace.walk.mode_50=1; trace.walk.depth_adjust_5e=1;
        // Other actor-constructor effects, the initial extra behavior, emitter
        // countdown and animation resource overrides are outside this scenario.
        enter_actor_falling(trace.fall,trace);
        std::cerr << "Diagnostic primary-state composition; excludes original actor construction,\n"
                     "extra behaviors, emitter timing, gadget runtime and the world scheduler.\n";
        std::cout << "tick,state,x,y,depth,clip,frame,fraction,requested_dt,consumed_dt,remaining_dt,elapsed,pending_state,stop_reason\n" << std::setprecision(17);
        double elapsed=0;
        auto row=[&](int tick,double requested,std::string_view reason) {
            const auto& p=trace.geometry().position;
            const double consumed=static_cast<double>(static_cast<long double>(requested)-trace.remaining_budget);
            elapsed=static_cast<double>(static_cast<long double>(elapsed)+consumed);
            std::cout << tick << ',' << name(trace.state) << ',' << p[0] << ',' << p[1] << ',' << p[2]
                      << ',' << trace.clip << ',' << trace.animation.frame_1cc << ',' << trace.animation.fraction_1c0
                      << ',' << requested << ',' << consumed << ',' << trace.remaining_budget << ',' << elapsed << ','
                      << (trace.pending ? name(*trace.pending) : "") << ',' << reason << '\n';
        };
        row(0,0,ticks==0 ? "tick_limit" : "");
        for(int tick=1;tick<=ticks;++tick) {
            try {
                const bool proceed=trace.tick(dt);
                row(tick,dt,!proceed ? name(trace.state) : tick==ticks ? "tick_limit" : "");
                if(!proceed) {std::cerr << "Stopped at " << name(trace.state) << " on tick " << tick << '\n';return 0;}
            } catch(const Boundary& error) {
                row(tick,dt,error.what()); std::cerr << "Stopped at " << error.what() << " on tick " << tick << '\n'; return 0;
            }
        }
        std::cerr << "Completed " << ticks << " diagnostic ticks\n";
        return 0;
    } catch(const std::exception& error) {std::cerr << error.what() << '\n';return 1;}
}
