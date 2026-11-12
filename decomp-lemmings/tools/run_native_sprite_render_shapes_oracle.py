"""Original adjacent sprite geometry and filtering; submit and COM adapted."""
import run_native_actor_walking_oracle as runner
import run_native_sprite_render_geometry_oracle as geometry

NEW_PINS=(
    (0x0047A565,504,"278b0623c999e4559569ec81a130fe763a64f8cda06ad958dd780b89c9e61fc0"),
    (0x0047A75D,1081,"d4c121e7ebd9e111cc317f772c36f379b80b955f7cefd3283c28e5988d205e74"),
    (0x00470F29,134,"ae58b1faba935652a35df4eccf726dd10b24f98723bf87f6b6d0a3b33d2e2fd5"),
)
PINS=NEW_PINS+tuple(p for p in geometry.PINS if p[0]!=0x47A190)

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=()
    runner.CONSTANTS=geometry.CONSTANTS
    raise SystemExit(runner.main())
