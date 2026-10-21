# A7k — the subtracted double is an integer divided by 1000

2026-10-21. **READ.** The field at the scale compare is still not one
number. The wider-mask score is unchanged.

## Formula

On the path through `0x45DA48`, with `edx` loaded from `[ebp-0x94]`:

```text
[G+0x25FF8] = V_init - (double)(int32)[G+0x25C64] / 1000.0
```

`0x45DA1B` loads that dword with `fild`. `0x45DA21` divides by the qword
`1000.0` at `0x4B1758`. `0x45DA27` stores the double at `[ebp-0x28]`, and
`0x45DA33` / `0x45DA3C` copy both halves to `[G+0x1C968]` and
`[G+0x1C96C]`. `0x45DA48` calls `0x45CC09`, whose `fsub` uses
`[obj+0x1C968]`.

`V_init` is `1.5` when `[G+0x1CB7C] == 0` inside `0x45C9AB`, and `3.5`
otherwise. `0x45D875` skips the call to `0x45C9AB` when the byte returned
by `0x4605E0` is zero. The rectangle setup at `0x45E0FB` runs only when
`[G+0x26024]` is nonzero, and `0x45CAD6` sets that byte to `1` inside
`0x45C9AB`.

## Band

If the integer is `0` and `V_init` is `1.5`, the field is `1.5` and the
scaled top is `1020`, which includes y `432`. If `V_init` stays `3.5`, the
field stays above `1.5` until the integer is at least `2000`, and the
setup is skipped before that. The integer itself is not a constant on
this path.

## Facilitator check

`build/tmp/a7k-facilitator/verify.py` re-read the `fild` / `fdiv` /
`fstp`, the store through `[ebp-0x94]`, the skip at `0x45D875`, the
`+0x26024` test, the byte store of `1`, and the qword `1000.0`. Expecting
the store displacement `+0x1C96C` failed; the file uses `+0x1C968`.
Restored hash
`63c338a41407adee8fbeca81d7cd0f1ed26887cb1fd8091bfd7436beff122dca`.

Build had no work. CTest 24/24. No game process.

## Left open

`[G+0x25C64]` at this `fild` is unread. The gate remains
`3031085/137520225 = 0.0220410125`.
