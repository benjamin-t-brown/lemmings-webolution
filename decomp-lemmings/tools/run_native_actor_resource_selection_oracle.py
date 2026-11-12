"""Compare the isolated actor-resource selection loader block."""
import run_native_actor_walking_oracle as runner
PINS = ((0x4437F6,168,"fe4d4c0f3b2bb0f022e9234d3a6e45ffafe72f67f2c2f349fad63297191fa0e7"),)
CONSTANTS = (
    (0x4BFFC8,"14004c0020004c0030004c0040004c0048004c004c004c00"),
    (0x4C0014,"4c656d6d696e677300"),
    (0x4C0020,"4c656d6d696e6773576174657200"),
    (0x4C0030,"4c656d6d696e67734163696400"),
    (0x4C0040,"4576696c52617400"),
    (0x4C0048,"4c6f00"),
    (0x4C004C,"486900"),
    (0x4C0090,"2573257300"),
)
if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=()
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
