"""Compare original shadow segment construction with explicit point-list adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x459557,998,"fb8c4b16cb86d0cf1f14a8b550791262138a1d7bfeffe951bd79389cf706cd27"),
    (0x406700,68,"699c6b6a605605e3da76d40aeb46eaa9af24f91e75b34166c02d29d7c0c2b306"),
)
HELPERS=(0x4066A0,0x403EB0,0x4017F0,0x404D10,0x40DE60,0x40BAA0,0x49C5AC,
         0x40DEC0,0x40DF30,0x40DF50,0x49C8F4,0x49FC56)
CONSTANTS=((0x4B0338,"0000000000000000"),(0x4B0698,"000000e08e79453e"),
           (0x4B07A8,"0000000000007040"),(0x4B02D0,"000000000000f03f"))

if __name__=="__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
