# A4r: actor material RGB modulation and conditional alpha/blend setup

Queue `a4r-actor-render-state`. Static read-only round; no game start or production
behavior change. Authority SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Worker packets, including both superseded versions, remain under
`build/tmp/a4r-actor-render-state/`.

## Result and scope

The material updater encodes texture-times-diffuse RGB on all three paths that
write material states. This gives static support to A4q's RGB modulation.
Texture-alpha preservation and a particular blend pair are still **unestablished
for the actor**: its resource flags and blend selector were not recovered.
This round reads encoded calls; it does not measure effective wrapper/device state,
setter success, rounding, or the remaining sprite-pixel residual.

The API interpretation is a **strong interface-layout inference**. The installed
MinGW `d3d.h` Device3 interface places `SetRenderState` at `+0x58`, `SetTexture` at
`+0x98`, and `SetTextureStageState` at `+0xA0`. The same global device pointer
`[0x58CDB8]`, argument counts and COM stack order agree at those call sites.
No creation GUID chain or runtime interface query was established. Numeric
operands below are READ independently of that inference; enum names use this mapping.

## Read path and caches

- `0x47ABAF e86c9df9ff` calls `0x414920` from actor billboard submission.
  It applies a resource via `0x414944 e8d6dc0500` to `0x47261F`, then calls the
  blend-selector routine via `0x41494F e82c040000` to `0x414D80`.
- The resource field is submit-object `+0x1A8`; its setter is
  `0x440C3D 8988a8010000`. The blend selector is the submit object's **first dword**,
  loaded at `0x41494C 8b02`; these are separate fields.
- Resource pointer cache `[0x5CD5F4]` skips the entire updater on equality
  (`0x472626 a1f4d55c00`, `0x47262E 0f8441030000`). Otherwise the updater binds
  stage-0 texture at `0x472653 ff9298000000` (`+0x98`).
- Flags cache `[0x4C24E4]` then compares against resource `+0x24`
  (`0x472662 3b5124`, `0x472665 0f840a030000`). Equal flags skip state setup even
  when a new texture was bound. The table describes the changed-flags path.
- Blend-selector cache `[0x4C227C]` independently skips its assignments on equality
  (`0x414D86 3b057c224c00`, `0x414D8C 0f84bf000000`).

## Material states

All 21 texture-stage setter calls in `0x47261F..0x472978` use **stage 0**.
Every color path assigns `COLOROP=4 (MODULATE)`, `COLORARG1=2 (TEXTURE)`, and
`COLORARG2=0 (DIFFUSE)`. Representative immediate tuples, in push order
`value, state, stage`, are `0x4726D1 6a046a016a00`,
`0x472798 6a046a016a00`, and `0x4727EB 6a046a016a00`.

Conditions are ordered; the first applicable row wins:

| Resource flags | Alpha operation | Alpha test | Key setter calls |
| --- | --- | --- | --- |
| `flags & 8` | SELECTARG1(TEXTURE) | disabled | `0x4726B1`, `0x4726CB`, render `0x47269A` |
| no bit 8, `flags & 0x10` | MODULATE(TEXTURE, DIFFUSE) | disabled | `0x47275E`, `0x472778`, `0x472792`, render `0x472747` |
| neither above, `flags & 3` | SELECTARG1(TEXTURE) | enabled, GREATER, reference from `[0x4C24E0]` | `0x47285F`, `0x47287A`, render `0x472893..0x4728C2` |
| neither above, no low-two bits, `flags & 4` | MODULATE(TEXTURE, DIFFUSE) | disabled | `0x472901`, `0x47291C`, `0x472937`, render `0x4728DC` |
| neither above, no low-three bits | SELECTARG1(TEXTURE) | disabled | `0x472954`, `0x47296F`, render `0x4728DC` |

The bit-8 and bit-0x10 paths return through jumps
`0x47271F e951020000` and `0x4727E6 e98a010000`. They **do not** reach the lower-bit
tests. This corrects the worker's final packet's phrase "after any color path" and
its overbroad alpha summary. Diffuse alpha itself is not established here.

## Blend selector

On a changed selector, `0x414D80` encodes:

| Selector | ALPHABLENDENABLE | SRCBLEND | DESTBLEND |
| --- | --- | --- | --- |
| 0 | false | no assignment | no assignment |
| 2 | true | ONE | ONE |
| 3 | true | SRCALPHA | ONE |
| other nonzero | true | SRCALPHA | INVSRCALPHA |

The literal setter tuples are checked at `0x414DB8`, `0x414DD4`, `0x414DEA`,
`0x414E02`, `0x414E1E`, `0x414E36`, and `0x414E4E`. For example,
`0x414DEF 6a056a13` pushes value 5 and state 19 (SRCALPHA/SRCBLEND), and
`0x414E23 6a066a14` pushes value 6 and state 20 (INVSRCALPHA/DESTBLEND).
The actor's selector value remains unresolved.

## Facilitator check (2026-10-19)

Independently reread the actor dispatch, field setter, blend routine, and full
858-byte material updater against the stock file. The separate checker
`python tools/harness/verify_actor_render_state_a4r.py` passes **34 setter tuples,
21 byte anchors, 3 SDK offsets and 20 enum definitions**, with output in
`build/tmp/a4r-actor-render-state/facilitator-verification.json`. Prerequisites:
normal harness Python with `pefile`/Capstone and the installed MinGW headers.
The checker also pins the two early-return jumps, preventing the final packet's
fallthrough error from entering this note.

Ghidra independently corroborated the control flow at `0x47261F` (REA Evidence
`ev_c1b7d4b3e8096a5548caafd2773ca6f39004fb42792979c808cc5ba09df1d6f0`, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`). Its bounded
summary is retained as `facilitator-rea-material.json`; original-byte reads remain
the authority. The owned REA session was closed.

Two repairs corrected bad address/enum citations and a misdecoded alternate color
branch. Verification still narrowed the final alpha summary as above; earlier
packets are preserved. Build reported no work; the full CTest run passed **20/20**.
No source behavior changed, so pixel/source mutations are not applicable. No game
or `webolution` process remains, and there were no live writes to restore. A2/A3
broader failures and the A4f/A4q sprite residual are unchanged.

Next bounded question: establish the actor submit object's selector and its
selected resource's `+0x24` flags, reusing retained evidence where possible.
