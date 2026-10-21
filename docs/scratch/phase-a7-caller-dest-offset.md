# A7f — no direct builder caller passes list destination G+0x2018

2026-10-20. **Negative READ.** The direct `call 0x4404FA` sites do not fill H.
The wider-mask score is unchanged.

## Result

`.text` contains 17 direct `E8` calls to `0x4404FA`:

`0x41403B`, `0x44CAB0`, `0x44CD47`, `0x44CD82`, `0x44D0B9`, `0x44D348`,
`0x44D389`, `0x44FFA0`, `0x454BD0`, `0x454BEA`, `0x457BC7`, `0x457BE6`,
`0x45A819`, `0x45AE87`, `0x45B617`, `0x45CB01`, `0x45D27B`.

None of those sites has the immediate `0x2018` in the 160 bytes before the
call. The control site `0x45D25D` is still `add edx, 0x25C68`.

`0x44CD06` has one direct caller, `0x441A7F`, which is the frame's
`add ecx, 0x2018`. Its entry `ecx` is stored at `[ebp-0x94]`. Inside that
function the builder calls pass that `this` plus a different offset:

| Call | List pointer | Bytes |
| --- | --- | --- |
| `0x44CD47` | `this+0x758` | `0x44CD30` `add eax, 0x758` |
| `0x44CD82` | `this+0x77C` | `0x44CD6A` `add ecx, 0x77C` |
| `0x44D0B9` | `this+0x224` | `0x44D0A8` `add ecx, 0x224` |

The function returns at `0x44D0EC`. On the frame path, `this` is H, so these
are the already separated records H+`0x758`, H+`0x77C`, and H+`0x224`.

## 0x44D316 is a different this

`0x44D348` and `0x44D389` are in `0x44D316`, which stores entry `ecx` at
`[ebp-0x38]`. They pass that `this+0x758` and `this+0x77C`. This is not
`0x44D0ED`; that function stores its `this` at `[ebp-0xC]`.

The only direct caller is `0x444B98`:

```text
0x444B8F  mov ecx, [ebp-0x10]
0x444B92  add ecx, 0x2018
0x444B98  call 0x44D316
```

`[ebp-0x10]` is not shown to be the frame receiver. The `+0x2018` here is
the helper's `this`, and the builder list immediates remain `+0x758` and
`+0x77C`.

## Facilitator check

`build/tmp/a7f-facilitator/verify.py` does not load the worker pins. It
re-scanned `.text` for the 17 calls, rejected immediate `0x2018` in the 160
bytes before each, re-decoded the `0x44CD06` saves and the three list adds,
the frame `add 0x2018`, the `0x44D316` prologue and its two list adds, and
`0x444B92`. Changing only `CONTROL_IMM` from `0x25C68` to `0x2018` failed
the `0x45D25D` check. Restored hash
`9e184fb5452a877e65844e00b9fdab23917580a247d1ff1cd2748ca96393aeca` was the
48-byte-window script; the landed script uses 160 bytes and passes.

The worker's labels for unrelated bases (`0x200`, `0x189C`, `0x2754`,
`0x1110`, `0x26000`) were not each re-derived. They are not needed for the
negative: none of those call sites carries immediate `0x2018` in the
pre-call window. A pointer computed still earlier and held in a register
would not show up as that immediate.

Build had no work. CTest 24/24. No Lemmings, webolution, or Ghidra process.
No production or install writes.

## Left open

H is still unfilled by any direct builder call. Whether `G+0x25C68` is
copied into H is the remaining producer question. `[ebp-0x10]` at
`0x444B8F` is unread. The gate remains `3031085/137520225 = 0.0220410125`.
