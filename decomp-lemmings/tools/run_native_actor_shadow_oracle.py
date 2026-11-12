"""Compare shadow update with explicit list/state/resource adapters and concrete geometry."""
import run_native_actor_walking_oracle as runner
from run_native_actor_terrain_probe_oracle import PINS as PROBE_PINS
from run_native_actor_shadow_segments_oracle import PINS as SEGMENT_PINS

PINS = (
    (0x458186,4775,"f58dc109bbc1701770603607a5c7453613f11a85e3c20c57b6a174bc09fddc86"),
    (0x413560,59,"e10637d1d1606b2cd36380cca7f81c8917aa7893dc17ffcb46e440a2bc70f821"),
    (0x4066D0,36,"a4ad0092f5c119225ea631a845d2868e0ce880df82c92534edcca4a4791c5134"),
    (0x401B80,14,"befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
    (0x406700,68,"699c6b6a605605e3da76d40aeb46eaa9af24f91e75b34166c02d29d7c0c2b306"),
) + PROBE_PINS + (SEGMENT_PINS[0],)
HELPERS = (0x42E913,0x42E8C7,0x430A70,0x42F22E,0x4066A0,0x403EB0,0x4017F0,
    0x404D10,0x40DE60,0x40BAA0,0x49C5AC,0x40DEC0,0x40DF30,0x40DF50,0x49C8F4,0x49FC56,
    0x43D56C,0x43E580,0x43EA20,0x4027C0,0x402680,0x401830)
CONSTANTS = (
    (0x4B0338,"0000000000000000"),(0x4B0698,"000000e08e79453e"),
    (0x4B07A8,"0000000000007040"),(0x4B02D0,"000000000000f03f"),
    (0x4B03E8,"0000000000002440"),(0x4B0400,"0000000000e06f40"),
    (0x4B03F8,"0000000000000040"),(0x4B1048,"000000000000d03f"),
    (0x4B07F0,"0000000000002040"),(0x4B08F8,"0000000000003040"),
)

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
