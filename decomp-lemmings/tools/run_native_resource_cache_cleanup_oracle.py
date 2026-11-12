"""Compare resource cleanup/recreation with original list and cache helper chains."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x44A050, 128, "2980ff59bd5433cc1c6be17943ee79716f080b010344036cf30c9f1cbcf3c056"),
    (0x485380, 119, "17521df7dddb9bf7b35758205af9f4ad60b088d24dec346d6d7471be42ef2716"),
    (0x485400, 108, "7f51d37fb8bbb35d412d659e1520a925aee38497aa68f57aee85f028a4645935"),
    (0x4854D0, 82, "e96bdc0739f627eb8cd5ccb9a9967b534585b126bcdac6b45ff370ca6337271f"),
    (0x485530, 94, "a5731f0116afc2a1d31429ae66fae6e2c457bd0790c99cfaf93ed081a5eec0d2"),
    (0x449FB0, 157, "d0846021a0b8ba1b65a98969ece55b4c5291a8118a85f405c7294c6e03ee4dd5"),
    (0x44BE00, 83, "04b91154d74e1bdbdf66c92beed1f12361b9786019d4cd960c75e373e8b40f6e"),
    (0x44C060, 46, "2e436a1f2aef81ceb70d00bd612286e23e46a5916c0e124289f7357dfc7786ac"),
    (0x44C090, 11, "012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),
    (0x430760, 17, "a26cd3064709e57ee483cfbbb2753f8da501718ce4ced1e834a5123dd2fcaa3d"),
    (0x430780, 29, "279bb61281a416389bf3313e5358634b5d927b0d349db8573597cecd1ad504d0"),
    (0x4306F0, 106, "c9ac76eb0fdf7ee30c39be22275c04a317e44ba8e990c4aa6d4030d792b31b72"),
    (0x4307A0, 30, "3f6d2d5c7711f92761fa78394636a1ea22836c94e313ce6fc2378ef7e44d1c34"),
    (0x406640, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
)
HELPERS = ()
CONSTANTS = ()

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = CONSTANTS
    runner.__doc__ = __doc__
    raise SystemExit(runner.main())
