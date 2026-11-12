"""Compare actor constructor orchestration, with declared subsystem adapters."""
import run_native_actor_walking_oracle as runner
import run_native_model_construction_oracle as model

PINS = (
    (0x42DE51,811,"15040d265ccb2da2faf319c56a20ef30547196c03d14c4d95c38169000fd5cfc"),
    (0x430A90,14,"befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
) + model.PINS
HELPERS = tuple(dict.fromkeys((0x4017F0,0x4066D0,0x401B80,0x406700,0x403EB0,0x4821D0) + model.HELPERS))

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=model.CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
