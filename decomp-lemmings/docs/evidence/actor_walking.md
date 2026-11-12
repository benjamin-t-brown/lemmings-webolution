# Walking state timing and update

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The runner checks the complete executable and every copied code block before
executing it in a separate owned x86 process. No game executable is committed.

| Routine | VA | File offset | Bytes | SHA-256 |
|---|---|---|---|---|
| Entry | `0x41BDBA` | `114106` | 26 | `3f6b5071b426e3f9c52b6a6b01178c429de0bf7aa21f512430c91a6649b4e9dd` |
| Update | `0x41BDD4` | `114132` | 397 | `cba6f8e3d252cf51973ff1b03314e897f6d87bbf1091cf61045e426095364a3f` |
| Boundary time | `0x41BF61` | `114529` | 155 | `efe9de72310047c149230e15bfb60288411eb328ccc26c0c95a07fc0a3faf415` |

REA entry evidence:
`ev_89c3a70e9fc8aeb3836390b57162ed27d8565886da08f8cf3561f3a1faf2bde9`.
REA update evidence:
`ev_f5b3635c4bb8093c01d6e8ce821598be702471bf5442c8d06509e3d37bffeca3`.
Local byte disassembly establishes full-width animation arguments, arithmetic
store points and unordered x87 branch behavior; decompiler argument fragments
are not treated as the actual argument width.

## Recovered behavior

Entry requests `walk`. Boundary time is the distance to the next integer X
divided by five. Direction exactly one increments the truncated low signed
32-bit coordinate with wrap; every other direction takes the subtraction path.
When flag `+18` is nonzero, zero or unordered boundary time becomes 0.2.

Mode `+50 == 3` halves remaining time. Each iteration chooses the boundary time
when it is less than or equal to remaining time, including the unordered case.
It moves X by five times step times direction, wraps X, advances the embedded
model, then subtracts step from remaining time. At a boundary it truncates X,
runs collision, and sets flag `+18` to one *after* collision. Partial steps clear
the flag. Surface and gadget callbacks still run when collision returned a
nonzero result. The gadget pointer is read after surface processing; a false
low-byte return clears it. Result zero forces remaining time to positive zero.
The final adjustment callback always runs.

No iteration cap or artificial progress is introduced. Valid direction/time
combinations and callback behavior must make the original loop terminate.

## Verification

Unit tests check entry selection, mode scaling, collision flag overwrite,
surface-installed gadget, gadget clearing, post-transition callback order,
zero boundary replacement, NaN boundary replacement and negative/NaN remaining
time reset. The x64 unit test passes.

The native oracle executes all three routines unchanged, together with original
wrap, integer-conversion and ordinary absolute-value helpers. It compares
15,376 entry/update cases, exact boundary and remaining-time bits, X bits,
direction, gadget pointer, flag, return values, complete ordered callback
traces, and absence of other writes. Inputs include both directions, integral
and fractional coordinates, wrap boundaries, zero/negative/NaN remaining time,
all callback scripts, direction changes during animation, surface insertion
and low-byte gadget results. An additional 72 helper-only cases exercise NaN,
infinities, signed 32-bit wrapping and signed 64-bit conversion limits.
Both unoptimized and optimized x86 native comparisons pass.

## Explicit boundaries

Adapters replace animation selection `42FD25`, model slot `+2C`, collision
`41BFFC`, surface `41C875`, gadget check `42C78F`, and final adjustment `42F977`.
The adapter observes calling conventions and validates object identities;
this evidence establishes the core loop with scripted effects, not the whole
walking collision or animation implementation. The native update domain has
finite X. Nonfinite X in the update reaches CRT exceptional `fabs` machinery
not loaded by this oracle; NaN and infinity are tested directly on the boundary
helper only. Source NaN update branches follow static x87 reading, without a
claim of native comparison for that complete exceptional path.
