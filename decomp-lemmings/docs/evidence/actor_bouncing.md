# Bouncing behavior type 0x21

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Complete original bytes, addresses, PE offsets and per-procedure hashes appear
in [actor_bouncing.asm](actor_bouncing.asm), the central verifier and the runner.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Apex initialization | `0x40EC2A` / `0xEC2A` | 25 | `dfc6af5ce6c9a8e03afeab202ec312f8e5cb6f984bd2d3d341cd795bbc1ffea7` |
| Constructor evidence | `0x40EC7E` / `0xEC7E` | 48 | `6eecedeb1303e6e4de0ba7303e979661fb59d1939bb6125cbef3627b5f8f4cc4` |
| Entry | `0x40ECCA` / `0xECCA` | 464 | `5d7f1cdd55279cba0c7c587006761c1e63c68623f47cb5de4d3085691c31b35d` |
| Leave | `0x40EE9A` / `0xEE9A` | 42 | `c52aef7d08c81ee7f1335959a70a1b7e434d658d7d95ef45b9390fbb462a00a2` |
| Update | `0x40EEC4` / `0xEEC4` | 805 | `dcf476b9e2156235f9441036d143424b379f049e42f9f5a1e4a8e4680dc5be89` |
| Collision | `0x40F1E9` / `0xF1E9` | 884 | `e34eba1a8411729bb848a59a7d44a1c6346806f2f3ba649f6cf109ebcdb5e4a3` |
| X timing | `0x40F55D` / `0xF55D` | 155 | `9c6e9dbb8e4faa8d9df08b352c3aae1fc95d10f29672b23ab1fe27dbc421edf0` |
| Y timing | `0x40F5F8` / `0xF5F8` | 264 | `0eeb5e2880f3acabfc30e212d29b9969fff1bb7fd97b1b5d2c9b45e8fc557ba6` |
| Root table | `0x40F700` / `0xF700` | 260 | `0dd480aa60d9ea9290c13b09f994003b598162511da2af0361c98f13c0881232` |
| Sample X movement | `0x42EEA3` / `0x2EEA3` | 47 | `79274e8f53fdbfaddf4bd0a8a6f565e90b1cc683dcd4599d5709eddadced3881` |

REA static dossier IDs in the same order (excluding X movement):

- `ev_736421a89dcfd6c7766a53fafd3bdb5f82dbad93b5f150a6e49cc5617ec211ab`
- `ev_7c6dbebe78b4b5aace5bcda955030fe7f2b715a48a38dc1d60458661dd2fd066`
- `ev_b055aa8b3eff2548aa1babc97eb5b660b48b5a2de17eb118766b93a7a5626204`
- `ev_33577d61169174517046a0130a28072fe762a4553daa2fb72d083baf9ade8761`
- `ev_cebef0f804c30b95a39e0426f757cb5d9760b3a148c27c47ca3308710fcf5a4d`
- `ev_8bd8f891072b0dbdea9ed621c04d7a5dd6e1e58962096cffd50f2920d668dabe`
- `ev_13507238fc63372b9cec4e3fe9e131e82ec3e87ec30d4f2757273b65b1ec8807`
- `ev_383c64da04643222f9f421e175e958e3dcc54c3deaafc89468c6d01e0d8cade9`
- `ev_d3e60e3cbe064df34ae813eebecea55b48ded8d78ee37fae96879541cdffe42a`

X movement: `ev_cc2ad4d0116ef6534753f29166c8b96e9b417da0ff6e62975e0a85cf17eb2caf`.
Local disassembly establishes exact x87 branch tests and binary64 stores.

## Static behavior

The constructor sets type `0x21`, extra flag zero and elapsed zero through its
base, but leaves origins `+18/+20` and boundary byte `+28` untouched. Logical
views retain these offsets; zero defaults for unknown initial fields are fixture
choices. Model storage is inline at actor `+78`, represented by an explicit handle.

The original apex initializer stores `-35 / -25 = 1.4` at `0x4C55A8`. The table
initializer solves `35*t - 12.5*t*t = h` for integer heights 0 through 24,
ordering each root pair using that apex time, into 50 doubles at `0x4C5410`.
The last element, `times[49]`, is also the duration global at `0x4C5598`.
The source has one storage location for that alias. With normal initialization
the table increases from zero to 2.8; it skips the noninteger maximum height 24.5.

Entry samples the foot using `pair_360`, subtracts vertical direction, truncates
the resulting Y to its signed low-32-bit projection for origin `+18`, and copies
X to `+20`. It logs signed actor ID and that position before selecting `fall`.
It subtracts the actor's current vertical direction once more, truncates/clamps
Y, and truncates/wraps X for a nearest-actor search. Squared distances use
wrapping 32-bit arithmetic followed by signed comparison, initialized to
`0x20000`; ties retain the first actor. Only the nearest actor is then checked
for primary type `0xE`, whose `+18` byte becomes one. Leave removes state `0x2D`
only when its membership query's low byte is nonzero.

X timing divides the distance to the next integer X boundary by five, with
direction exactly one selecting increment; the other branch uses truncation
toward zero. A nonzero `+28` replaces zero or unordered time with 0.2.
Y timing scans adjacent table entries with inclusive lower/strict upper bounds,
returns time to the next entry and its mirrored height, and preserves the output
height if no interval matches. Its x87 comparisons, including unordered table
entries, are retained explicitly.

Update first advances the model by the entire supplied budget. If actor `+60`
is zero it creates/attaches effect `0x40E3C0`, then clears `+28`. Each iteration
selects the smaller X/Y time (X on unordered; Y on equality), clips to the
remaining budget, advances elapsed, and snaps elapsed to the correct mirrored
table entry at a vertical boundary. Y is either that integer height or
`35*t - 12.5*t*t`, multiplied by vertical direction and added to origin Y.
X moves by horizontal direction times five times step. After outer wrap/clamp,
the recovered sample movement helpers subtract `pair_360` and wrap/clamp again.
The debug position format receives Y before X. Boundary collision runs before
`+28` is set to one, even if attachment changes it. A partial step sets it zero.
Result zero forces remaining to positive zero.

Collision samples head and foot cells before either branch. At a Y boundary,
the first nonzero foot value without bit `0x80` selects landed state, even when
it also has trampoline bit `0x1000`. Otherwise bit `0x1000` after the apex
attaches a new bounce, then nonempty head attaches falling, then elapsed equal
or unordered to the final time attaches falling. X-boundary collision queries
wall contact and reverses direction on a nonzero byte, returning zero.

## Unit checks, native comparison and limits

Unit tests check table monotonicity/duration alias, timing, closest-actor tie
ordering, floor-before-trampoline priority, head collision, final-time fallback,
wall reversal, state removal and the zero/NaN-budget pre-loop effect path.
They pass on x64 and x86.

The native harness executes unchanged entry, leave, update, collision, timing,
table initialization, sample movements, sampling, coordinate/grid helpers and
bounce/falling constructors. Every original range and constant is verified
against the executable before execution. Explicit adapters handle model advance,
animation selection, logging, allocation, effect `0x40E3C0` construction,
attachment, landed-state selection, wall contact, actor lookup/state removal,
and CRT square root. The latter supplies rounded host sqrt for the table's
positive integer discriminants; original exceptional CRT sqrt is not claimed.

O0 and O2 each pass 21,098 update/collision trace cases, 5,000 entry/leave pairs,
672 helper cases and five initialized tables. They compare complete ordered
traces, exact finite coordinates/times/directions/offsets, boundary/nearby flags,
return values, and absence of unexpected actor/state writes in entries,
updates and collisions. Explicit entry cases check distance ties, a nearest
actor with an ineligible primary type, and signed squared-distance overflow.
Deterministic cases cover every table interval, both sides of the apex, exact
duration, floor masks/priority, null allocations, unordered final time, initial
model callbacks changing budget/elapsed/effect flag, attachment changing the
boundary flag, and zero/negative/NaN budgets crossed with zero/nonzero effect
flags and successful/null allocations. NaN trace payloads are projected; x87
status flags are not compared.

The helper cases include 192 altered-table checks with NaN, infinities and zero
in lower/upper interval entries, including the zero-or-unordered retry path.
Units were also rerun at unoptimized x86 and optimized x64. Decimal constants
whose literal evaluation can retain x87 precision are compared as stored
binary64 bit patterns in the unit test.

Valid positive grids and terminating X coordinates are required. The update's
Y helper must find a valid interval when it takes a vertical substep: the original
uses an uninitialized stack height otherwise. No fabricated behavior or progress
guard is added for that invalid domain. Full effect construction, actor lifetime,
renderer and full-game runtime remain outside these comparisons.
