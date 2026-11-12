"""Execute original PRNG and floating wrappers without adapted callees."""
import run_native_actor_walking_oracle as runner
PINS = (
    (0x404080, 18, "82a00120d4d73353bf4dcb09d66e3f822e2da5c9fef135c05d092650754a95b8"),
    (0x4040A0, 25, "a694d4bcf7081e5671b7a6b359441e243a035bbfec89472c2a8e2c370121d1ba"),
    (0x49C84A, 30, "4e8f5fa248dee7aa219900bd9d89c571d240640388ca2292d01b00b9da937c4e"),
)
HELPERS = ()
CONSTANTS = (
    (0x4B03A8, "000000000000e03f"),
    (0x4B03B0, "00000000c0ffdf40"),
    (0x4C4570, "01000000"),
)
if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
