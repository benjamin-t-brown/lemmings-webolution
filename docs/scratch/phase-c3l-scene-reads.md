# C3l: kind 5/7 readers, type-2 load, kind-1/5 draws (offline, directive Q1 round 2 of 2)

Worker note: `build/tmp/c3l-scene-reads/{prediction.md,round-note.md,reads.py,reads.out}`. Offline; no game, no debugger, REA not used (capstone via `read_steppers.py` on mouv).

**Body cap breached.** The directive cap is ten substantive bodies, hard; the worker read about 17 (the kind 5/7 and type-2 loads each pull chains of one-call wrappers) and did not stop. Recorded as a breach, not waived. The reads below are still verified; the cap stays ten for the next round.

| Term | Tag | Address |
| --- | --- | --- |
| Shared element `0x47a130` = u32 + 2 f64 + 3 f64 + f64 = **52 B** (calls `0x402560`, `0x4821de`, `0x480bb0`, `0x402580`, then `0x47a100`) | READ | `0x47a137..0x47a18a` |
| Kind 5 record = u32 + 3 elements = **160 B**; kind 7 = u32 + 4 elements = **212 B** (kind 5 `0x469b49` -> `0x46a140` -> `0x46b0d0`/`0x46b100`; kind 7 `0x469b8d` -> `0x46a1e0` -> `0x46b3d0`/`0x46b420`; loop compares 3 at `0x479fcd`, 4 at `0x47a092`) | READ (the brief's `0x46af80` is a different reader, kind 0's chain) | worker table |
| Type-2 load `0x47b44c` (vtable `0x4b1ad8` +0xc): f64 x3 to `+0x320/328/330`, u8 to `+0x110`, u8 to `+0x338`; if 0: string (`0x47d940`) then f64 x4; else `0x468bf2` u32 x4 then string; then u32 (`0x402560`, to `0x406240`); then node head `0x46bb73` | READ | `0x47b44c..0x47b5da` |
| All 17 type-2 records in MM_CLIMB_002 take the `+0x338 == 0` branch; the 1 branch is read but not exercised | READ / parsed | `reads.out` |
| Scenery record 2 parses past `0x967a2` to `0x98d41` (17 type-2 `^lolli*`/`^sheep*`, then `thenull` with 29 children); Entrance and Exit parse | READ layout | `reads.out` |
| 16259 B block after `0x98d41` (starts `01 01 01`, `myanim` at `0x9cc3f`) before the next Object tag `0x9ccc4`; Entrance trailer 12709 B, Exit 170 B | UNREAD | |
| Kind-1 draw `0x46a280` -> emitter `0x46a810`: 3 vertices per record picked by the record's own indices into the transformed array `[0x58ce24]`, so draw order is record order | READ (field names uv/colour UNREAD: consumer `0x46d372` not read) | |
| Kind-5 draw `0x46a340` -> `0x46afa0`: skipped when `0x418350` <= 0.0 | READ skeleton; meaning UNREAD | |
| `[this+0x100]` = first f64 after the node name; compared with depth z | READ values; meaning DECODED from the compare only | |
| `0x43bd80` bind, `0x58d498` object (runtime), `0x418350`, `0x46d372`, type-2 draw `0x47d0c0`, kinds 2/3/4, what `0x406240`/`0x440c30`/`0x440c50`/`0x474519` do with the type-2 string and f64x4 | UNREAD | |

Prediction miss recorded by the worker: it expected an f64 header plus the node head for type-2; there is a two-way branch.

**Facilitator check.** Re-read on mouv: `0x47b44c` (head f64 reads to `+0x320/328/330`, `+0x110`, `+0x338`, the `jne 0x47b5b0` split, calls `0x47d940`, `0x474519`, `0x402580` x4, `0x440c30`, `0x440c50`, `0x468bf2`, `0x402560`, `0x406240`) and `0x47a130` (call order). A separate scan of `MM_CLIMB_002.LVL` finds the same four Object tags (`0x59637, 0x91775, 0x9ccc4, 0xa0cd0`), 17 type-2 name strings (12 lolli, 5 sheep) and `myanim` at `0x9cc3f`. `reads.py` rerun is byte-identical to `reads.out`. Mutating one stride alone on a /tmp copy: kind 7 4 elements -> 3 drops parsed records 4 -> 1; kind 5 3 -> 2 drops them 4 -> 2. Narrowed: the kind 5/7 dispatch chain, the kind-1/5 draws and the `+0x338 == 1` branch were not re-read by me.

**Consequence.** Kinds 5/7 and the type-2 load are READ. The render (directive Q4) still needs the `0x43bd80` bind, the type-2 draw, `0x418350`, `0x46d372` and the stack bottom, so it does not run on this round. The directive's rounds for Q1 are spent (c3k is round 1, c3l round 2).
