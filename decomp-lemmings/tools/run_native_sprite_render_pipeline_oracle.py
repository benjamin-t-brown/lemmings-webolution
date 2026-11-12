"""Complete original submission/clip/pack path; heap, atexit and COM adapted."""
import run_native_actor_walking_oracle as runner
import run_native_sprite_render_submission_oracle as submission
import run_native_sprite_render_clip_oracle as clipping
import run_native_sprite_render_draw_oracle as drawing

PINS=tuple(dict.fromkeys(submission.PINS+clipping.PINS+drawing.PINS))
CONSTANTS=tuple(dict.fromkeys(clipping.CONSTANTS+drawing.CONSTANTS))

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=()
    runner.CONSTANTS=CONSTANTS
    raise SystemExit(runner.main())
