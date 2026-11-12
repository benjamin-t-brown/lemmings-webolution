"""Execute explosion behavior with explicit world and effect adapters."""
import run_native_actor_walking_oracle as runner
PINS = (
    (0x4081B0, 48, "4ff6d61e427a98994a56355f53482c5d9f00867748bb7925f8f0e9983a5dae83"),
    (0x4081FC, 43, "e69d9f4c4674c390c17b8de3eabee08f4a38d4c424614eff3faccc4c9b238762"),
    (0x408227, 1193, "f02fb6c649efad7aa5b483160da57e11a4f87b46b788ab0a40e7bd514fc0ec93"),
)
HELPERS = (0x401320,0x418390,0x406A00,0x42F183,0x4066A0,0x4066D0,0x406700,0x401B80,0x4017F0,0x401830,0x403EB0,0x49C5AC)
CONSTANTS = (
    (0x4B0338, "0000000000000000"),
    (0x4B03F8, "0000000000000040"),
    (0x4B10B8, "03000000"),
    (0x4B10BC, "02000000"),
    (0x4B02D0, "000000000000f03f"),
)
if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
