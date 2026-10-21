# A7g — immediate 0x25C68 does not write base list H

2026-10-20. **Negative READ.** Every `.text` encoding of `0x25C68` adds that
offset and then calls or pushes the pointer. None of them stores it at
`+0x2018`. The wider-mask score is unchanged.

## Ten aligned adds

The byte immediate `68 5C 02 00` occurs at ten places. Each is the
displacement of `add reg, 0x25C68`.

| Instruction | Base | Next use |
| --- | --- | --- |
| `0x45BE4E` | `[ebp-0x14]` | `call 0x415140` |
| `0x45C6F9` | `[ebp-0x40]` | `call 0x449130` |
| `0x45D25D` | `[ebp-0x94]` | `push edx`, then `call 0x486847` |
| `0x45D286` | `[ebp-0x94]` | `call 0x47ACF9` |
| `0x45D29A` | `[ebp-0x94]` | `call 0x47AC50` |
| `0x45D2D3` | `[ebp-0x94]` | `call 0x44DF40` |
| `0x45D2EE` | `[ebp-0x94]` | `call 0x44E010` |
| `0x45E33C` | `[ebp-0x94]` | `call 0x44E150` |
| `0x4AD43E` | `[ebp-0x14]` | `call 0x449130`, then `ret` |
| `0x4AD52F` | `[ebp-0x40]` | `call 0x449130`, then `ret` |

`[ebp-0x94]` in `0x45CFF9` is that function's entry `ecx`, the same slot A7e
called G. The only `push ebp` / `mov ebp, esp` from `0x45CFF9` through
`0x45E342` is the entry at `0x45CFF9`, so the six adds in that span use G.
They produce `G+0x25C68`. `0x45E33C` then calls `0x44E150` on that object.
`0x44E150` is also the helper A7d saw on H. This call does not make the
object H, and it does not identify the lower-band caption.

`0x45BE4E` and `0x45C6F9` sit in earlier functions. Their frame slots are
not G. `0x4AD43E` and `0x4AD52F` are `mov` / `add` / `call` / `ret`
fragments in a run of similar fragments. `0x4AD52F` matches `0x45C6F9`'s
slot and callee. `0x4AD43E` uses `[ebp-0x14]`, like `0x45BE4E`, but calls
`0x449130` rather than `0x415140`.

## Facilitator check

`build/tmp/a7g-facilitator/verify.py` does not load the worker pins. It
found the same ten immediates, checked each base load and the following
call or push, and checked the single prologue across the pause-handler
span. Changing only `CONTROL_IMM` from `0x25C68` to `0x2018` failed the add
checks. Restored hash
`723a3e989bc242ce8a0fdcdc3a022c5095f4b765f8d01e63cdd78d9f2b5d7d21`, then the
checker passed. The shipped immediates stayed `0x25C68`.

Narrowed from the packet: six G adds, not seven. The high fragments are not
accepted as SEH, and `0x4AD43E` is not a mirror of `0x45BE4E`. Build had no
work. CTest 24/24. No Lemmings, webolution, or Ghidra process.

## Left open

A copy into H that never re-encodes `0x25C68` was not searched. `[ebp-0x10]`
at `0x444B8F` is still unread. The gate remains
`3031085/137520225 = 0.0220410125`.
