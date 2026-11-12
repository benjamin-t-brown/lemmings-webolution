"""Compare original file-runtime helpers and complete string reads at the CRT boundary."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x0047DB2F, 68, "58a31fa3db446f8568be5b3f9c4a825de8b90add559be12f6a30d7c89efb4a58"),
    (0x0047DB73, 127, "07fa7b1c6b1312d93bd37eafda9db96b451f01d33f272d3bc0624b9f64233232"),
    (0x0047E395, 120, "efb411804ff8b80768f70b471282eb9849d919cb709b4ef9ab03166393c204a5"),
    (0x0047F040, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
    (0x0047E844, 87, "d0f2429aa3e193d964dc67437a108b2de4dbbb541e2134eea74c8c11dd503446"),
    (0x0047EEF7, 70, "70601e5ef5678f380b7f37bac8241e2962339b38963c2e1c97408c26823acd39"),
    (0x0047DAC8, 29, "aec40f1e210009a794b263010b78bce2e4aa490c9c5560c2cd8d25330b64dfc7"),
    (0x0047DC28, 72, "aff14242f418c9727edf3f555fd0daf4b70029be83de56454d342b2f4a47394a"),
    (0x0047EF3D, 11, "012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),
    (0x0047EE27, 17, "d500a64daa2531474397cfd9958b6e1b88fc9b0b86d04aa2ea4015b50431bc91"),
    (0x0047EE38, 84, "f001d4cc9ee000cc449d69d658a34d67bdcc2fe4216db4707487f96ac5c6e44f"),
    (0x0047EE8C, 20, "9a0a6ccfb070e91f4254c606398f78e0d8299c783663935500cb20bd7ba7afd7"),
    (0x0047DAA9, 31, "3911d4ea905910cfff6f69dae9b45866a66d81e313790b703e961dd6100daf17"),
    (0x49C750, 240, "de277c53bfccd0f688de14be2f3a7258615d8c770e4bd3f7cc88a8e29afb91b9"),
)
HELPERS = (0x402540, 0x402560, 0x47D940)

if __name__ == "__main__":
    runner.PINS = PINS
    runner.HELPERS = HELPERS
    runner.CONSTANTS = ()
    raise SystemExit(runner.main())
