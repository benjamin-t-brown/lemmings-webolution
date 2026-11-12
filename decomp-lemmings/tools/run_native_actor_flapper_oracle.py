"""Compare original flapper state with explicit model/log/factory/attach adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x416910, 48, "85f69e148ff7d2c86459dd86caeca676c622c407f14e75dbe18f4e7ff4f4af8c"),
    (0x41695C, 66, "e4e562c9fb986547b0ba358a2ec03115c0415e02962df765140650f264f931be"),
    (0x41699E, 531, "abfa88e8f3f7ebbb7694dbd6bb96bf5c1df8cfb63c26b5a03775a4cadf83f625"),
    (0x401320, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
    (0x418390, 47, "254616efe4eeecb6423131c308a44bd17d051f5651e8a0adebbe4d242ca65772"),
)
HELPERS = (0x42F22E, 0x4066A0, 0x403EB0, 0x4017F0, 0x404D10, 0x40DE60, 0x40DEC0, 0x40DF30, 0x40DF50, 0x40BAA0, 0x49C5AC, 0x49C8F4, 0x49FC56, 0x416038, 0x406A00)
CONSTANTS = (
    (0x4B0A18, "0000000000003440"),
    (0x4B0338, "0000000000000000"),
    (0x4B0698, "000000e08e79453e"),
    (0x4B07A8, "0000000000007040"),
    (0x4B03E8, "0000000000002440"),
    (0x4B03F8, "0000000000000040"),
    (0x4BF3E0, "6578706c6f646500"),
    (0x4B0A58, "c06b41009e6941005c69410020134000"),
)

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = CONSTANTS
    runner.__doc__ = __doc__
    raise SystemExit(runner.main())
