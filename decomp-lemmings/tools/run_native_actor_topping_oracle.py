"""Compare original topping state entry/update/leave with explicit model adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x417CCC,26,"7569ef28b556b0304189d342b0c349417949c5a465a5c1e4d4e24e937be0426b"),
    (0x417CE6,241,"c4c0b09dd5d8ef89fd33b58b365ce09dbcd773274472a4630b73ddbcef25f88a"),
    (0x417DD7,49,"3e90027ff9b98137c87c89cde1f9e434e7163f0794d2629fbfc2ef9f49c4853b"),
)
HELPERS = (0x40DEC0,0x40DF30,0x40DF50,0x40BAA0,0x49C5AC,0x49C8F4,0x49FC56)
CONSTANTS = ((0x4B0338,"0000000000000000"),(0x4B0698,"000000e08e79453e"),
             (0x4B07A8,"0000000000007040"),(0x4B02D0,"000000000000f03f"),
             (0x4BF41C,"666c697000"))

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
