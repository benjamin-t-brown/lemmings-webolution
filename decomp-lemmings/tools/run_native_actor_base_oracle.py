"""Compare the unchanged actor base constructor with a registry adapter."""
import run_native_actor_walking_oracle as runner

PINS = ((0x4230A2,202,"fa818c0a9e84b04f99175d5536e13e8f39096f386c873205b141680b840857d2"),)
HELPERS = (0x4066A0,0x4017F0)

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=()
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
