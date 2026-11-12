"""Compare actor/base destruction including original primary/extra cleanup."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x42E17C,216,"4b4d4cfd33548d7fd76b2bbdf9003672f9337a8965f62629706cac012ccf06ab"),
    (0x42316C,114,"0b156b4be9e5b5c74bea7f154e51cf248f96ab1cbfc8d277046bf76b384cdade"),
    (0x42E70A,27,"93fd8f8d455318447912a3fef1938aa1ab1fef475419d8b8c19f47a5c2cc2c0c"),
)
HELPERS = (0x423380,0x403EB0,0x42E78A,0x42E725,0x4302B0,0x430170,0x430190)

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=()
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
