"""Compare unchanged resource manager constructors, without effect adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x00484E40, 31, "abc654c126fc53484988c6d82c1b27bd769cfac8fe24f09aac83e92549a14ee1"),
    (0x00485120, 87, "e2c88a9d6d75312e95dfe18ca4f3458937bf4f21db6f2591f9d729e5ad7899dd"),
    (0x004852D0, 44, "f96b6ded611f158614ece584fd9815320401773e881c7483472a6d0f6d90ec02"),
    (0x004855F0, 119, "637f730d8f47f10710df0a739f47f5dcff74504268cd8f24f1b2458afb229b81"),
    (0x00485730, 22, "7153939bf56a7e8321d61a27f4b79a78b95b23606a048e8b3faa8027df24a98e"),
    (0x00485470, 53, "1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704"),
)
PINS += ((0x430920, 33, "65eb72c5dcc4eef9e79634224883eed9512f5f5d518e120100eaca3f5333f080"),)
HELPERS = ()

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = ()
    raise SystemExit(runner.main())
