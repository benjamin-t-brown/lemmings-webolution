"""Compare the unchanged original shadow constructor chain without adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x457F40, 557, "c1930a02a6306f4e888904a7996871933e77d195934a375d63461431f4aa72ae"),
    (0x4011A0, 39, "c946d75a8e84b4c660af2c6b118aaef8aba4dbcbe0a5a4e6dd820282e646e700"),
    (0x459940, 22, "90c0aa7faa40cddbc21c573c8301978496fb82ff811a68c2c28dd6a094a55521"),
    (0x459960, 43, "31c8e2ae1f03c23b1efcfa0193f266730cd9070163f6892d7cabe3926c5cb579"),
    (0x4400B0, 25, "1e4ae53f1441f11b973d8e31e984ffbf23a0986f1965b869f4afc628fead2a05"),
    (0x4821D0, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
    (0x4135A0, 29, "928c42c9f8e5f5ba1a08043a6e57ea8c4fda98c5fd4268074d7cd464bd6bf7d1"),
    (0x4135C0, 57, "c1ea3714d3563268d58297b1021f445ab22ed71f3e812c19770affc860847a44"),
    (0x459B00, 28, "1da575a340082503f2abfa3d9a2a5d642bf5450b6c780abdde10a109613d5c74"),
    (0x401B80, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
)
HELPERS = (0x4066A0, 0x4017F0, 0x413560)
CONSTANTS = ()

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = CONSTANTS
    runner.__doc__ = __doc__
    raise SystemExit(runner.main())
