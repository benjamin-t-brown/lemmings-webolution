# C1bg: selected index 0 and enable 1 are derived, not read from pause memory

2026-10-25, `c1bg-selected-index`, OpenCode disasm-agent session
`ses_e8be216bdffeXXRlTZUjopA05c`. Dispatched 11:30:58 MDT. The session's
idle outcome is `failed` at 11:45:02, the same second as the facilitator
session. The packet `build/tmp/c1bg-selected-index/round-note.md` was
already written. Zero stock starts. No production code changed.

## DERIVED result

For the normal init path and `MM_CLIMB_002`'s trailer counts, the selected
index at `panel+0x28` is **0** and the enable byte at `panel+0x2750` is **1**.
The extra quad is `panel+0x25A0`. At draw time, index 0 is the rectangle
**18, 12, 90, 44**. This is not a read of retained pause memory. `0x4C9200`
and `0x4CB928` are past the file image.

The archive member `COMPILED\MM_CLIMB_002.LVL` at box offset 47132210, size
699117, SHA-256
`95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567`,
ends its count run as ten dwords **10, 0, 0, 0, 0, 0, 0, 0, 0, 1** and then
dword **8**. The first strictly positive count is index 0, so the fallback
index 7 does not survive init.

## Facilitator check

Own exe reads, not the worker script. Exe SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

- `0x45A608` stores enable byte 0. `0x45A620` skips this setup when
  `0x450B80` returns nonzero. `0x45A629` writes index 7.
- `0x45A6B9` sets the loop index to 0. `0x45A6CB` compares it with 10.
- `0x45A84A` reads the enable byte. `0x45A852` skips the store when it is
  already nonzero. `0x45A85A` / `0x45A85E` `jle` skips a count that is not
  strictly positive. `0x45A866` stores that index. `0x45A898` sets enable 1.
- `0x45B86D` reads ten dwords through `0x402560` into `panel+i*4`, then one
  more dword.
- `0x45B676` reads the enable byte and `0x45B67E` skips the extra quad when
  it is zero. Otherwise `0x414870` receives left 18, top `index*40+12`,
  right 90, bottom `index*40+44`, and `0x45B6B5` submits that quad after the
  base-plate call at `0x45B66A`.
- `0x4C0E08` is the bytes `^tab_selected`. `0x45E90F` pushes that address and
  calls `0x45FE10`. `0x45A655` loads `[0x4E334C]` and `0x45A665` calls
  `0x440C30` on `panel+0x25A0`. The live handle value was not read.

The retained A7p script's only primary clicks are `button(0)` aimed at
`goto_level.NODE` `(206, 204)`, on the map, before the level. Later inputs
in that script are P, SPACE, mouse button 1 and mouse moves. The
next/previous handlers and the event executor were not re-opened. No later
writer is ruled out beyond that recorded input list. No render and no score.
The next draw uses this derived index; it does not pick plate 0 from the
stock ring.
