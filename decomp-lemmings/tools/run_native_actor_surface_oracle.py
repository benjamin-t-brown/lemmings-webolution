"""Run original walking surface tracking and depth approach with adapted gadget queries."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x41C875, 666, "ee05836e53dc30772ff5830269c690acbf673158ca4493a914996d836bafa5c2"),
    (0x42F977, 352, "788c5b7df46a173591d45b52df0cd6041967fa65ab23ca21cd320e45d9755ae2"),
)
HELPERS = (0x42F22E, 0x4066A0, 0x403EB0, 0x4017F0, 0x404D10, 0x40DE60,
           0x40DEC0, 0x40DF30, 0x40DF50, 0x40BAA0, 0x49C5AC, 0x49C8F4, 0x49FC56)
CONSTANTS = ((0x4B0338, "0000000000000000"), (0x4B0698, "000000e08e79453e"),
             (0x4B07A8, "0000000000007040"), (0x4B02D0, "000000000000f03f"),
             (0x4B10E0, "7b14ae47e17a943f"))

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = CONSTANTS
    runner.__doc__ = __doc__
    raise SystemExit(runner.main())
