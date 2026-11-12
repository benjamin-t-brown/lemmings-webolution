"""Compare vertical-range and terrain-distance routines with their original helper chain."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x42F6EC, 156, "a47aaf13325a019ad3242e30894400456f1a5a5a5cc459c9e0a5820fe700caee"),
    (0x42F788, 194, "5af155c4d6fdc639bd74190564f17b2b78e6c1f3654fd96aea231286b96adc8d"),
)
HELPERS = (0x42F22E, 0x4066A0, 0x403EB0, 0x4017F0, 0x404D10,
           0x40DE60, 0x40BAA0, 0x49C5AC)
CONSTANTS = ((0x4B0338, "0000000000000000"),)

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = CONSTANTS
    runner.__doc__ = __doc__
    raise SystemExit(runner.main())
