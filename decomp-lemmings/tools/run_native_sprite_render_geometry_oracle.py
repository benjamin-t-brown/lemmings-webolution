"""Original sprite quad geometry chain, final rendering submission adapted."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x0047A190, 981, "909ce99b037561bda1f93eec51874a02f83feb547c286d0d95dd8a7277d8f94f"),
    (0x00440C50, 169, "6d40e3c40cec7c5483f1a91f50831fd0d31b8a6c1fa212bdc8a018b3bc312278"),
    (0x0047AB96, 36, "f9cabb25b6727a0a62e151ef3e1485bc9161f509b5de72d4e50203becc0343bf"),
    (0x00440C30, 25, "884e31507e541b231d3e767f6d563f0c137594fc226de99558f32b10cc4c10b2"),
    (0x00468D00, 30, "cd4f5020ae29638cd3347814186c48c1bf65ae888d92c953c19f521cc140d753"),
    (0x00468CE0, 27, "e041a9b5519845f716b94875e5e31131ee18a69b1a2421a976ee5ecb04a1bbe4"),
    (0x0047ABC0, 27, "dec4947d33139e09e25685b4f6aa1ed34b7eec203d39a293f6ee0d76cac06acb"),
)

CONSTANTS = (
    (0x4B03F8, "0000000000000040"),
    (0x4B02C8, "0000000000001040"),
)

if __name__=="__main__":
    runner.PINS=PINS
    runner.HELPERS=()
    runner.CONSTANTS=CONSTANTS
    raise SystemExit(runner.main())
