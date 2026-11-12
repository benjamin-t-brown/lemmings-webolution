"""Verify forced behavior with explicit random/world/effect adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x416BF0, 48, "c154ec4cc4b6f32abeede8734b498f8bf86f6d1e1c96ccb50ea10eb9fda709dc"),
    (0x416C3C, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
    (0x416C49, 587, "eee345ebccdbec34ed3b6341e41eb69b5d299cb0c67673fa393459d2ecc21a54"),
    (0x401320, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
)
HELPERS = (0x418390,0x401830,0x4017F0,0x403EB0,0x49C5AC,0x404080,0x4040A0,0x49C84A)
CONSTANTS = (
    (0x4B03A8, "000000000000e03f"),
    (0x4B03B0, "00000000c0ffdf40"),
    (0x4B0338, "0000000000000000"),
    (0x4B03F8, "0000000000000040"),
    (0x4B10B8, "03000000"),
)

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
