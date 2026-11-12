#include "lemmings/emitter_update.hpp"

namespace lemmings {

bool update_emitter(EmitterState& emitter, double elapsed, EmitterWorld& world,
                    EmitterCallbacks& callbacks) {
    if (emitter.effect_started_ae == 0 && world.suppress_effect_4e32a0 == 0) {
        callbacks.effect_44521b(world.effect_value_4e32b4, 0);
        emitter.effect_started_ae = 1;
    }
    if (emitter.opened_ac == 0) {
        callbacks.update_model_432383(emitter, elapsed);
        if (callbacks.model_finished_44(emitter.model_44) != 0) {
            emitter.opened_ac = 1;
            elapsed = callbacks.model_remainder_48(emitter.model_44);
            emitter.countdown_c0 = 1.3; // binary64 at 0x004B0F48
        }
    }
    if (emitter.opened_ac != 0 && emitter.remaining_98 > 0) {
        // The executable stores the x87 result to binary64 before comparing.
        emitter.countdown_c0 = static_cast<double>(
            static_cast<long double>(emitter.countdown_c0) - elapsed);
        // FCOMP; FNSTSW AX; TEST AH,0x41 admits less, equal AND unordered.
        if (!(emitter.countdown_c0 > 0.0)) {
            --emitter.remaining_98;
            void* storage = callbacks.allocate_actor_49c1a8(0x3A0);
            void* actor = storage ? callbacks.construct_actor_42de51(storage,
                static_cast<double>(emitter.coordinate_9a),
                static_cast<double>(emitter.coordinate_9c), emitter.actor_type_b0) : nullptr;
            if (emitter.flip_vertical_ad != 0) callbacks.flip_vertical_42f5e3(actor);
            if (emitter.direction_b4 == -1) callbacks.flip_horizontal_42f66b(actor);
            if (emitter.alternate_b6 == 1) {
                emitter.direction_b4 = emitter.direction_b4 == 1 ? -1 : 1;
            }
            if (callbacks.update_actor_14(actor, -emitter.countdown_c0) == 0) {
                callbacks.insert_4090b0(actor);
                callbacks.register_43b536(actor);
            } else {
                callbacks.insert_426180(actor);
            }
            --world.remaining_by_type_4cba8c[callbacks.actor_type_4c(actor)];
            --world.remaining_total_4cbaa0;
            emitter.countdown_c0 = static_cast<double>(
                static_cast<long double>(emitter.countdown_c0) + emitter.interval_b8);
            callbacks.refresh_444abc();
        }
    }
    return true;
}

} // namespace lemmings
