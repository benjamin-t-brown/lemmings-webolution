"""Compare model destruction with the sole resource-manager release adapter."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x43CE88,87,"73e58fbb82c29c7895e026f9ee8aad7e654f3bc29131ee57e723e86268c2ba41"),
    (0x46820D,72,"6661abce883b6dcac51ead5b08874ff3d122b4cb1d8b5db60ee2fa3a167f744d"),
    (0x4658B7,31,"80e21f525f3b672e459c0312a260e3215576d838aeb474f304f07381369c8211"),
    (0x468255,81,"d5b6f8f9aedd65687bdf9ef315a880060ac8ae7923b17df0257718fa925810c4"),
    (0x440BF0,19,"758ba2e9bd1e02647438bd10c15567ba9d6a86806423ceb4ffc77e9f637a941f"),
    (0x440C10,32,"201ae284b6133141ba31e708af8af89a3def5234e7ef0139c68d8f3feb5cf57d"),
    (0x49C655,104,"e7a5b601ea8f3f9000f5f6c522c22bfc271fe8c8645a1ea1a270ef4b5a7ae5f7"),
    (0x49C6BD,24,"2c7fc6efbbb1db82eb61aeff1e738b410fed12f9b658526a8357e8fde26cce90"),
    (0x41A4B0,11,"012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),
)
PINS += ((0x420DA0,11,"012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),)
HELPERS = ()

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=()
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
