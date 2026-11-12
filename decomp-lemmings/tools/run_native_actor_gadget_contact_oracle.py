"""Execute the original gadget contact, interpolation and movement chain unchanged."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x42C714, 123, "1d027537cca62d719729ce5be13513877fbd04f8d7c4a286a80530619867d7ca"),
    (0x42C78F, 292, "acb54efd704224bb04bb9465531e2e1ee5bf17723cbfaa424a1e223426f13717"),
    (0x412C90, 27, "d5a8fdd6b139351863017b8caf4146b1a9126fc5310d4d0a4c4a2a82e7dcf08a"),
    (0x42EE74, 47, "4df50eff7bce9fd5c64817e772f5a5629c8c855b882dcba4557dd4e3a9b1111b"),
)
HELPERS = (0x42F22E, 0x4066A0, 0x403EB0, 0x4017F0, 0x404D10,
           0x40DF50, 0x40BAA0, 0x430BA1)
CONSTANTS = ((0x4B0338, "0000000000000000"),)

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = CONSTANTS
    runner.__doc__ = __doc__
    raise SystemExit(runner.main())
