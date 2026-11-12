# Region-selected terminal behavior, type 0x2E

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The spawned monitor chooses this behavior when terrain bit `0x40` is set and
a region matches. Its exact gameplay name is not established.

| Routine | VA / PE file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Constructor | `0x406C20` / `0x6C20` | 48 | `80426f004f3e5f3aad6c62a452cdc08a93e2c7d127d8fd597ee161a5409428c9` |
| Entry | `0x406C6C` / `0x6C6C` | 1418 | `86ca371273326093ff33932bb22bc3621912c81a71190cf31002606569fc3fcc` |
| Update | `0x4071F6` / `0x71F6` | 104 | `f5737cd3bb0aed25041d2a5462df5d47394decffea281a32467e1fd112c08fdb` |
| Empty leave | `0x407260` / `0x7260` | 13 | `8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a` |

Original bytes/disassembly: [actor_region_terminal.asm](actor_region_terminal.asm).
REA evidence:

- Constructor: `ev_3ffb4687013df5620964549b114b06d1b6ad0b6459f11502b0571840026120e3`
- Entry: `ev_f5d63e497ad28ddae9acc59b44db78a6d67732b9ffa0a680e1406967474de635`
- Update: `ev_7c3d6a18ce68a0e51c6b1ea11e6c89cbdb1a72f5e3bb2aabf4d5887b42a9532f`
- Leave: `ev_4288c6454524d4bc36152d68159eccfad973ee66121733ffb2feb021c6f6807e`

## Static recovery

Constructor metadata sets elapsed `+08=0`, phase `+10=0`, extra `+11=0`, and
type `+14=0x2E`. Unknown object fields are preserved. Entry captures actor
position, increments the copied Z by 1 and then Y by 4, storing each binary64
result. These values are pinned at `0x4B02D0` (1.0) and `0x4B02C8` (4.0).

The first effect constructor is `0x4072A0` for category 3 and `0x4018E0`
otherwise. It receives count 6, the live actor position, pair `(4,0.5)`, flag
1 through `0x407710`/`0x401F30`, and mode 12. Its direction is the world
transform of the captured offset minus the world transform of the live
position; the live position is transformed first. The offset's truncated X/Y
sample a triple that is assigned to every child, with child count reread on
each loop test. The effect is wrapped in an entity at the live actor position
and inserted.

The second effect constructor is `0x402870`; it receives count 10, triple
`(255,127,127)`, live actor position, pair `(4,0.5)`, mode 4, and a freshly
computed direction using the same captured offset. Its entity also uses the
live actor position. Position reads remain live across callbacks, which may
change the actor between calls. The captured offset remains unchanged.

This entry issues no world event and does not clear actor byte `+28`. It
does not clamp position or use direction `+44`. Effect allocation must succeed
because the original dereferences it; entity allocation can produce null and
still reaches insertion.

The update body is byte-for-byte identical to terminal type `0x1A`'s
`0x40B9F2`: advance model, accumulate reread elapsed/remaining, clear remaining,
query model completion, return 2 for a nonzero completion byte and zero
otherwise. The source shares that already recovered implementation. Leave is
empty. The entry shares the terminal effect callback vocabulary and adds the
single flag setter; graphics/resource implementations remain explicit boundaries.

## Verification

Unit tests pass on x64 and x86. They verify metadata, unchanged actor flag,
the ordinary category branch, fixed offset, two live positions, all effect
parameters, transform order, null entity insertion, and update mutation.

The native oracle passes **10,000 entry, constructor, and update comparisons**
at O0 and O2. Original bodies and basic numeric helpers execute unchanged.
Cases cover both category branches, zero through six children, null entities,
finite/negative/infinite/NaN positions and times, model callback mutation,
and effect callbacks that mutate the actor position between later reads.
It compares ordered callback values, actor fields, constructor bytes, model
elapsed/remaining/result, and the correct model object offset. NaNs in traces
are compared as a class; exceptional FPU status is outside the claim.

Effect allocation/construction/setters, world transform/sample operations,
entity construction/insertion, and model advance/completion are adapters.
The test validates orchestration, not their internals or live game rendering.
The runner checks executable identity, PE mappings, code hashes, and both
numeric constants before executing any original bytes.
