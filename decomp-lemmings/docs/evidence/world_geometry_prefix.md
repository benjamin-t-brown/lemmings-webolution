# World geometry prefix after the level name tables

The exact installed reference is `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The loader `0x004432C1` calls `0x00439FD3` immediately after the fifth
name table and its following string. The analyzed `0x00439FD3` body is
499 bytes at file offset 237523, SHA-256
`e7b3563f87612f7107ef07b13a61917eef7c68c43ac32ea2e4b8987a71d08426`
(REA `ev_75d76f3fcf7e94adb7d1584eecab8ba88de19ba65ddf65ce7bce79d15bd3118c`).
Its first file-read call is
`0x0043D0F0` (REA pseudocode
`ev_6f31dc6bd2dd543b9fd605c413f9568ac844301294ee65991220a4e8ceb4230c`).
It reads one four-byte value and one eight-byte double, then calls
`0x0043E5A0` and `0x0043E500` for two counted arrays. Each record of
the first array is read by `0x0043D09A`: 32 pairs of three-double values,
or 1,536 serialized bytes per record. `0x0043D060` reads 32 doubles per
record of the second array, or 256 bytes each. These are stream shapes;
the field meanings and original allocation layout are not established.

| Address | File offset | Bytes | SHA-256 |
| --- | ---: | ---: | --- |
| `0x0043D0F0` | 250096 | 72 | `f30fedcc3a703e9bcdbbe8f8222cc8336a2825cf61b9489d6e54c73fde468170` |
| `0x0043E5A0` | 255392 | 120 | `5fd554e70889cfd81d6adc48546f5a973aedeb8e6b5a08760b9c2103f98a3168` |
| `0x0043D09A` | 250010 | 86 | `c3e809faab40047ae2dc863b714ad04eb88eea8df620759d046c8dcc34205f05` |
| `0x0043E500` | 255232 | 117 | `6e86c5611943d8346ef6deef6e00d8fd561d2f53806792aa5739492c2c645ac7` |
| `0x0043D060` | 249952 | 58 | `85d22dfdf5b11d9e2f8d1370eecc39d4777f71e9bfb1c819e232adf649e939ba` |

The verifier pins those bodies. REA pseudocode for the array readers and
records is `ev_6c2f2a02382f7ad914b199d4ae93235f41fb94eb74638902e61fb8f567fafe3e`,
`ev_53616f5415398d78ebbb58bb48834be1ad04250a8f618ff017f5ba560a362d58`,
`ev_38bbdf72e058be4dcd109e28a06542b23a57e5f532cd0622d8d4d0d98c01365a`,
and `ev_884fbab88717f24ce58b7d0cdfc14a4a76792c783e046fb0a015c8dab29ec32a`.

All 105 installed `.LVL` entries start this section with value 112,
double 10.0, seven first-array records, and seven second-array records.
`0x00439FD3` then reads another four-byte value and calls `0x00469578`
for a class-one record. The first value is one in every installed fixture.
The shared class-one file reader parses this record and its nested records
using the same read path described in [the level loader note](004432c1.md).

The remaining file-read path of `0x00439FD3` reads another four-byte value
and double, then four counted arrays: 32-bit integers through `0x0043B640`,
triples of 32-bit integers through `0x0043B9C0`/`0x004279C9`, another integer
array, and a byte array through `0x0043B700`. `0x0043B5B0` then reads 6,144
raw bytes. Two bounded strings follow. Finally, 32 calls to `0x004608A0`
read counted lists through `0x00460C50`; each `0x00461353` record contains
one 32-bit integer, two 16-bit integers, and five doubles. The short reader
is `0x00423360`. The verifier pins each helper's exact body bytes, lengths,
and file offsets. REA pseudocode evidence includes
`ev_3cbffbc72facfb111f8bcef180af1e8baf6e2f9cfc8d78b2cce9723eaaae6468`,
`ev_8150ee0daf85caf25f3b32d26eab808469eba5e2810da1728eec837b08d47c6f`,
`ev_1114a735f2b65f123bb9009ad7606a90fd6b30b8decb5883eebe40928d280519`,
`ev_265f0e86671b12d0e6d509f1f4fbf05067f13cc32bd4e80e13e90c30b348d00b`,
`ev_fa56942b6dbe30ac6d7ac725c7067ef941290c8942bd02c9edc4936ac1f12735`,
`ev_abb002b545ea396b47c6baaf512e9cc5fa40c472cbf2e96c1d4f9f470e062a26`,
and `ev_78cb300b03b96120badb0385d0a2ef1d7fcacca179417f3d08b9635b07a37ce2`.

All 105 installed levels have value 96, double 0.36, 49,858 integers in the
first array, 24,576 in the second integer array, 12,288 bytes in the byte
array, and six entries in each of the 32 tail lists. The integer-triple
count varies by level. The two strings have `INN_` and `OUT_` prefixes and
track level names. The C++ reader preserves raw double bits and read order.
The optional installed-fixture test compares every serialized value and
the final byte cursor on x64 and x86. It does not execute the original
world loader or reconstruct its in-memory object graph.
