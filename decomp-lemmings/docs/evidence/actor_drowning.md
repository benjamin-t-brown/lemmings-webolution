# Drowning behavior, type 0x22

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The name is supported directly by entry's animation string `drown`, located at
VA `0x4BF130` (six bytes including the terminator).

| Routine | VA / PE file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Constructor | `0x408700` / `0x8700` | 48 | `5340d1a8f921a030a37df03ee26ab3f04c218ac1705941d7b97c29e7846fb151` |
| Entry | `0x40874C` / `0x874C` | 26 | `95343cf9ec4047afc2f53e367b9995503ab13f94fb4f86757755a943cce22f31` |
| Update | `0x408766` / `0x8766` | 99 | `982a0a9cfeefe0c8bde6cd56d296f2c7adaa42e54d9ab0ed69dfbc996f7a1c16` |
| Shared empty leave | `0x401320` / `0x1320` | 13 | `8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a` |

The complete instruction bytes/disassembly are recorded in
[actor_drowning.asm](actor_drowning.asm). REA evidence:

- Constructor: `ev_a5766f3687839b1158e8cb5821cd9a738603c5fc713ec802fa876c9dea87285f`
- Entry: `ev_8b3ff135758830e85dbac81e0bf71ff4e275150d535b8f85bbbf7ca6bf2e18f0`
- Update: `ev_b3f1b617b69a7c14b4a16ef38b4c7549b33803deac6445274fbd26dac39fb8d7`

## Recovered behavior

Constructor calls the common `0x418390`, sets type `+14=0x22` and extra
`+11=0`. The inherited elapsed `+08` and phase `+10` are zero. Other bytes
are left unknown. Entry calls `0x42FD25(actor,"drown")`.

Update calls inline model `actor+78` virtual slot `+2C` with the complete
remaining double. After that callback it reads state elapsed and remaining
again, adds with x87 extended intermediate precision and a binary64 store,
then writes positive zero to remaining. It returns 2 only when state elapsed
is **strictly greater than 2.0**; otherwise it returns zero. The threshold
bytes at VA `0x4B0568` are `00 00 00 00 00 00 00 40`. Unordered comparison
does not complete the behavior. It does not query model completion.

If remaining aliases state elapsed, clearing remaining also clears elapsed
before the comparison, so the result is zero even if the sum exceeded 2.0.
The source preserves this ordering. Animation selection and model advance
are explicit hooks and may be composed with recovered animation routines.

## Verification

Unit tests pass on x64 and x86, covering constructor metadata, the exact
animation name, zero/negative time, the exact threshold and next representable
value above it, infinities, NaN, callback mutation, and aliased time storage.

The x86 oracle executes unchanged constructor, entry, update, common base and
empty leave bodies. It passes **20,000** complete constructor byte checks and
entry/update traces at both O0 and O2. Input cases include random binary64
values, threshold boundaries, callback mutation, and aliasing. It checks the
animation target receives the actor base, while model advance receives
`actor+78`. Unknown constructor bytes remain unchanged. NaNs are compared as
a class, without claiming exceptional FPU status equivalence.

The runner verifies executable identity, code bytes, PE mappings, the name,
and threshold before execution. Only animation selection and model advance
are adapted; this does not verify their internals or a live game session.
