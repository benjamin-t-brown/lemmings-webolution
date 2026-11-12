# Original pseudorandom generator

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

## Static reading

| Routine | Bytes | Evidence |
| --- | ---: | --- |
| `0x49C84A` | 30 | `ev_6cd304055420653548c3b817ab63d30468af93a72cdaf54756739c3d7daafb63` |
| `0x4040A0` | 25 | `ev_de913a4bf6e2087eb6763ad6098f170600d23c69ea9f9f4e0ebb7341c593b541` |
| `0x404080` | 18 | `ev_57f7a225ef5bddf5ce511f8e6ed87f8502bba25d5d16fe725eea75234a29b563` |

The integer generator advances global `0x4C4570` as
`seed = seed * 0x343FD + 0x269EC3` modulo 2^32 and returns bits 16..30.
The image initializes the seed to 1. The unit wrapper divides the result by
32767 in x87 extended precision. The signed wrapper subtracts 0.5 and doubles
that extended result. Endpoints 0/1 and -1/1 are inclusive.

The public API makes the 32-bit state explicit. It does not silently reseed or
claim where the game's other startup paths may reseed it. [Original assembly](game_random.asm)
and the runner pin the whole routines, initial seed and both floating constants.

## Unit checks

The known first five integer results from seed 1 are checked. All 32,768
possible next outputs are reached by inverting the odd multiplier modulo 2^32;
the unit and signed ranges and exact endpoints are checked.

## Native comparison

The x86 oracle executes all three original routines without adapted callees.
For each possible output it compares integer/unit/signed results and final seed
(98,304 calls), then compares a continuous 100,000-call signed sequence starting
at `0xFFFFFFFF`. O0 and O2 pass, including the full extended-precision results.
The forced-state oracle also executes 20,000 complete effect updates with this
unchanged original random chain (40,000 total cases including adapted-random
mutation cases). This is offline native comparison, not live-game replay.
