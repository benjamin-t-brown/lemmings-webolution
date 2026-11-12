"""Compare original already-open finalization blocks with tell/seek adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x0047E6CC, 41, "5788559ea557c7e7290a940732e627d1333c3eed890f4d8edd3bf1d80e008f58"),
    (0x0047E770, 172, "3a69b43387be5f2f815de749e31a157c4b8628068e511caf2bb73a54a8110b49"),
    (0x47DAA9, 31, "3911d4ea905910cfff6f69dae9b45866a66d81e313790b703e961dd6100daf17"),
)

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = ()
    runner.CONSTANTS = ()
    raise SystemExit(runner.main())
