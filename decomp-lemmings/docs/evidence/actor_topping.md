# Climb topping / flip behavior, type 0x1E

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The climbing collision path selects constructor `0x417C80` (already pinned in
the climbing evidence), whose vtable `0x4B0AF8` selects these entry/update/leave
functions. The entry animation name is `flip`; the name alone is not evidence
of additional gameplay behavior.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Entry | `0x417CCC` / `0x17CCC` | 26 | `7569ef28b556b0304189d342b0c349417949c5a465a5c1e4d4e24e937be0426b` |
| Update | `0x417CE6` / `0x17CE6` | 241 | `c4c0b09dd5d8ef89fd33b58b365ce09dbcd773274472a4630b73ddbcef25f88a` |
| Leave | `0x417DD7` / `0x17DD7` | 49 | `3e90027ff9b98137c87c89cde1f9e434e7163f0794d2629fbfc2ef9f49c4853b` |

Full original instruction bytes: [actor_topping.asm](actor_topping.asm).
REA static evidence:

- Vtable: `ev_b63366d8d2083c1a5c6c1f95ad9a237e4a5f0c97536937398b632a95e06907a8`
- Entry: `ev_d5e1cf1b518752d58fd537c1a2cf6a1ec8c8fff801a6cc6c14d75c91522f0dc8`
- Update: `ev_c0e18facb18391d4ada48dc202ad27a879d9a8f6e87ac005e64782ad578401a4`
- Leave: `ev_a68426fff1a4fd93476e3391d079f8a85b26edaf5aec887906141a8613799e60`

## Static behavior

Entry selects animation `flip`. Update advances the inline model with the full
input double, then queries completion slot `+44`. If its low byte is zero,
remaining becomes positive zero and result is zero, regardless of budget sign.

On completion it truncates current X with the signed-64/low-32 x87 projection,
adds horizontal direction with 32-bit wrap, wraps the result around the cylinder,
truncates again, and stores the signed integer as double X. It similarly adds
three times vertical direction to truncated Y with wrapping integer arithmetic,
clamps and truncates, and stores integer Y. It calls model slot `+48`, stores
that returned value to remaining as binary64, selects landed state, and returns 1.
The model value is retained as slot-based `model_time_48`; the routine does not
subtract it from the supplied budget or independently compute elapsed excess.

Leave subtracts vertical direction from Y first, then horizontal direction from
X, with binary64 stores and no wrapping, clamping, or sample-offset adjustment.
None of these routines accesses behavior object fields; unknown fields remain
outside the logical view. Graphics/model/attachment are explicit callbacks.

## Unit tests and original-byte comparison

Units cover entry, unfinished/finished paths, non-boolean completion bytes,
X wrapping, Y clamping, callback order, remaining-time replacement and leave.
They pass on x64 and x86.

The x86 oracle executes unchanged entry/update/leave and original coordinate,
height, conversion and finite absolute-value helpers. Adapters replace animation
selection, model advance/completion/time, and landed-state selection. Twenty
thousand cases at O0 and O2 compare ordered traces, projected NaN values, exact
finite coordinates/remaining, directions, return value, and other actor bytes.
They include signed zero, infinities, NaNs, signed 32/64-bit conversion limits,
integer overflow in vertical movement, negative/zero time, and callbacks that
change actor coordinates/directions/budget between reads. A landed-state adapter
can also invoke the unchanged original leave, matched by the recovered leave.

This is a controlled original-byte comparison with synthetic model effects,
not a full game observation. Positive height and terminating X wrapping are
required; exceptional CRT handling and FPU status flags remain unverified.

Audit on 2026-10-29: all three procedure byte listings in `actor_topping.asm`
match the pinned executable byte for byte. Fresh x64/x86 unit builds and O0/O2
native comparisons passed with the counts above.
