"""Compare complete model construction with unchanged original bytes, no adapters."""
import run_native_actor_walking_oracle as runner

PINS = (
    (0x43CE00,136,"234d6c9a8f7339d98fa0eff288e46d5defcabb5544914f52c54084dbe5adfaab"),
    (0x465840,119,"45be7b301be996428b11a2a7d531d49a8294198ba2a1661ffbc906503725b667"),
    (0x4680F0,285,"e52c82772f8e310caa379fc7a8fe694a75024e1e218482e40eaa25ac6e4549d9"),
    (0x41A490,20,"8604081cbfaca3bd2be59e14d7329270bd8c943ec93d07607b4d57e4713e05c1"),
    (0x4409E0,35,"bc997c93547cee16c683ff9498104d940c6ac2e8a5cb9de58ecc9465a20ace44"),
    (0x440A10,469,"cec018a6b0b0958089c1a62379859f94c33673631d3524a87b73bd4fbb8a32ac"),
    (0x420D60,55,"ebc365e89b779b7dcb472eec894880fd1194961acea6a94f9d6734d02d8f37f5"),
    (0x420DA0,11,"012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),
    (0x49C5D3,106,"b80af3235f4307f2af0c9cb3c24267eb2cba55ad9a856b8d070ddbb17f7483d5"),
    (0x49C63D,24,"3f6add3a0b9e7cae0d44ae4404274273921e43355dbfca561cc807194b6fba45"),
)
HELPERS = (0x4017F0,0x401B80,0x4821D0)
CONSTANTS = ((0x4B15C0,"92cb7f48bf7d6d3f"),)

if __name__ == "__main__":
    runner.PINS=PINS
    runner.HELPERS=HELPERS
    runner.CONSTANTS=CONSTANTS
    runner.__doc__=__doc__
    raise SystemExit(runner.main())
