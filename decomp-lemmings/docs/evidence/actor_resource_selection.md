# Actor resource selection loader block

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Recovered range | File offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `0x4437F6..0x44389D` | `0x437F6` | 168 | `fe4d4c0f3b2bb0f022e9234d3a6e45ffafe72f67f2c2f349fad63297191fa0e7` |

Original instruction bytes: [actor_resource_selection.asm](actor_resource_selection.asm).
This range is an internal block of loader `4432C1`, whose REA procedure and
pseudocode evidence is
`ev_be59b3db1d78a25779bed92af28d71fe729d92198a81e14da0f5c46f128099da`.
The rest of that 4,372-byte loader is not claimed by this implementation.

## Static behavior

The block initializes a category index to zero and iterates four categories.
It reads enabled bytes from caller locals at `[EBP-1A0+index]`. A zero byte
stores `-1` to global ID array `4E335C[index]` without reading quality or calling
the resource manager. Any nonzero byte enables the category.

An enabled category reads its current quality DWORD from `[EBP-19C+index*4]`,
selects a suffix, formats `%s%s` into `[EBP-2AC]`, then calls resource-slot
registration `446040(this=58D23C,name)`. Its returned signed DWORD is stored to
the current output slot after the callback returns. The API deliberately calls
this operation **registration**; name deduplication or lookup is not assumed.

| Category | Prefix pointer / text | Quality 0 | Quality 1 |
| --- | --- | --- | --- |
| 0 | `4C0014` / `Lemmings` | `LemmingsLo` | `LemmingsHi` |
| 1 | `4C0020` / `LemmingsWater` | `LemmingsWaterLo` | `LemmingsWaterHi` |
| 2 | `4C0030` / `LemmingsAcid` | `LemmingsAcidLo` | `LemmingsAcidHi` |
| 3 | `4C0040` / `EvilRat` | `EvilRatLo` | `EvilRatHi` |

Prefix pointer table is `4BFFC8`; suffix pointer table `4BFFD8` contains
`4C0048` (`Lo`) and `4C004C` (`Hi`). Format string `%s%s` is at `4C0090`.
The native runner checks all table and string bytes directly against the pinned
executable before mapping them.

Inputs and outputs are explicit arrays. Enablement and quality are reread per
category, so callbacks can change later categories. Existing output IDs remain
visible until their respective write. A callback can change output slots; the
current slot is then overwritten with its return value, while other changes
persist until later iteration writes. Enabled categories require a valid
quality index 0 or 1; the original does not validate that index.

The surrounding loader's decisions that populate these arrays, resource-slot
registration internals, loading assets, and subsequent binding are outside this
recovery. The callback must return normally and names are valid during its call.

## Unit and original-byte comparison

Units pass on x64/x86 at O0/O2. They check the four category names, disabled
categories with otherwise-invalid quality values, non-boolean enabled bytes,
negative/zero returned IDs, and callbacks that alter future enablement/quality
and overwrite the live output array.

The native oracle executes the unchanged 168-byte block using a controlled EBP
frame. A small calling bridge supplies its required locals and preserves host
callee-saved registers. The block's original exit branch targets a `RET` shim
installed at `44389E`, immediately outside the claimed range; the surrounding
loader does not execute. Original CRT formatter `49CA6A` is adapted to the exact
fixed `%s%s` concatenation, and `446040` is adapted to the same synthetic
registration effects used by the source fixture. No claim is made about the
general formatter or resource-manager implementation.

**2,048 cases pass at both O0 and O2**: all 16 enabled masks, all 16 initial
quality masks, and eight callback mutation scripts. Enabled values include 1,
2, 128 and 255. Returned IDs include `-1`, zero, and signed-int extrema. Every
case compares names and callback order, output values visible at each callback,
final output/input arrays, manager and temporary-buffer pointers, and guards
around the selected frame-local regions. The full executable identity and block
hash are checked before execution.

This is controlled native comparison of an isolated loader block, not a full
level load or observed original-game resource-registration session.

```powershell
python tools/run_native_actor_resource_selection_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_actor_resource_selection_oracle.exe
```
