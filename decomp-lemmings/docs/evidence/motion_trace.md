# Falling/walking diagnostic composition

`lemmings_motion` is a new source diagnostic. It composes recovered falling,
landing, walking, terrain, world-depth and sprite-animation routines around one
synthetic actor. It reads installed assets in place and writes derived CSV.
It does not execute the original game or reproduce its actor/world scheduler.

## Run it

```powershell
build/lemmings_motion.exe --archive 'C:/progs/Lemmings Revolution/lemmings.box' --patch 'C:/progs/Lemmings Revolution/patch.box' --level 'COMPILED\MM_CLIMB_002.LVL' --sprite 'SPRITES\LEMMINGSHI.SPR' --ticks 600 --dt 0.02 > motion.csv
```

Use archive entry names with backslashes. `--emitter` selects the zero-based
`entities_425e50` record, default zero. Patch entries precede the base archive.
The selected sprite supplies clips directly; original category-resource lookup
and override lists are not constructed. A missing selected clip stops the
diagnostic instead of continuing with an invalid animation index.

`--ticks` accepts integers from zero through 1,000,000. `--dt` must be a finite
number greater than zero and at most one. Numeric options require a complete
numeric token; values such as `600junk` or `0.02junk` are rejected. Invalid
arguments, nonexistent records/resources and load errors return exit code 1.
Tick limits and explicitly reported unsupported boundaries return exit code 0:
they are successful diagnostic stops, not evidence of successful gameplay.

## Initial actor fields

The executable authority is the installed build with SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The following initialization facts were checked statically against that exact
binary. This is not a native comparison of the complete constructor.

| Original routine | File offset | Bytes | SHA-256 |
|---|---|---:|---|
| Actor constructor `0x42DE51` | `187985` | 811 | `15040d265ccb2da2faf319c56a20ef30547196c03d14c4d95c38169000fd5cfc` |
| Depth initializer `0x42F93C` | `194876` | 59 | `fa4d448743eff0d8dfdd5f73c696d9e85fc12593044d6556869d5a7f3b41c797` |

REA evidence: constructor
`ev_3a211ca1e2c46f9e9fc412ee932431d5c3c984cf364c108d5b1af9eaf7062e8a`;
depth initialization
`ev_b14e3313e60b0bb52a00c98c959a614bec16f096dc13923d3ed7fd463f10f40c`.

| Diagnostic value | Original evidence or explicit scenario choice |
|---|---|
| X/Y | Signed emitter shorts `+9A/+9C`; recovered emitter update passes these to `42DE51`. |
| Initial depth 0.5 | Constructor's initial XYZ temporary uses binary64 `0x3FE0000000000000`. |
| Category `+4C` | Emitter `+B0`, passed as constructor parameter 5. |
| Directions `+40/+44` initially 1 | Stores at `42DFDD` and `42DFE7`. |
| `pair_350 = {0,3}`, `pair_360 = {0,0}` | Constructor stores at `42E0DB` through `42E12A`. |
| Vertical/horizontal reversal | Recovered emitter applies `42F5E3` for nonzero `+AD`, then `42F66B` when signed `+B4 == -1`. |
| Climb/umbrella abilities zero | Constructor byte stores at `42E0CD` and `42E0D4`; this diagnostic does not assign skills. |
| Mode `+50 = 1` | Constructor store at `42E15B`; no double-pass scheduler mode is simulated. |
| Depth adjustment `+5E = 1` | Constructor byte store at `42DFAD`. |
| Falling primary state | Constructor allocates `415FA0`, then attaches it at `42E0C5`; diagnostic calls recovered falling entry directly. |
| Falling state retry byte `+21 = 0` | Explicit fixture choice. The falling constructor does not establish this byte. |
| No extra states or current gadget | Explicit reduced scenario. The full constructor attaches an extra state created at `4151F0`, which this tool omits. |
| One actor at tick zero | Explicit scenario; emitter opening/countdown, actor counts, alternation and initial overshoot update are omitted. |

Selected original instruction bytes establishing those assignments:

```text
42DFAD  c6 42 5e 01                 mov byte ptr [edx+0x5e],1
42DFDD  c7 40 40 01 00 00 00        mov dword ptr [eax+0x40],1
42DFE7  c7 41 44 01 00 00 00        mov dword ptr [ecx+0x44],1
42E0CD  c6 41 48 00                 mov byte ptr [ecx+0x48],0
42E0D4  c6 42 49 00                 mov byte ptr [edx+0x49],0
42E0FC  c7 81 5c 03 00 00 00 00 08 40  mov dword ptr [ecx+0x35c],0x40080000
42E15B  c7 42 50 01 00 00 00        mov dword ptr [edx+0x50],1
```

Depth initializer `42F93C` reads binary64 constants 0.5 at `4B10C8` and 0.25
at `4B10D0/4B10D8`. Its original 59 bytes are:

```text
55 8b ec dd 05 c8 10 4b 00 dc 25 d0 10 4b 00 dd 1d 30 5b 4c 00
dd 05 c8 10 4b 00 dc 05 d8 10 4b 00 dd 1d 28 5b 4c 00
dd 05 c8 10 4b 00 dc 05 d0 10 4b 00 dd 1d 20 5b 4c 00 5d c3
```

It sets `4C5B30 = 0.25`, `4C5B28 = 0.75`, `4C5B20 = 0.75`. The walking final
adjustment selects the first target for horizontal direction minus one and the
third otherwise. The tool supplies these two exact values to recovered
`approach_actor_walking_depth`.

## Tick and boundary accounting

Each tick starts with the requested `dt`. Recovered primary routines mutate the
remaining budget. Result zero ends the tick. Result one applies a supported
falling/walking transition, transfers geometry, resets the new behavior view,
calls its entry and continues with the remainder. Entry is projected after the
current primary update returns; original attachment, leave/destruction and extra
states are not run by this driver. Result two stops as `outside`.

Terminal `0x1A` and bounce `0x21` requests stop before entering those behaviors.
Gadget list/contact requests stop as `gadget_runtime_required`. Other state
creation requests stop as `additional_behavior_required`. These routines may
exist elsewhere in the library; the diagnostic does not claim to compose them.
State-query callbacks return absent and extra-state clearing is empty because
the scenario contains no extras. A driver safeguard permits at most 64 primary
update calls per tick, stopping as `primary_update_limit`; it is not an original
game rule. Original internal substep loops retain their own behavior.

CSV starts with a tick-zero row. Existing position/animation columns are followed
by these diagnostic columns:

| Column | Meaning |
|---|---|
| `requested_dt` | Budget given to this outer tick; zero at tick zero. |
| `consumed_dt` | Requested minus remaining budget after the call/stop. |
| `remaining_dt` | Actual remaining reference argument, including unused time at a boundary. |
| `elapsed` | Sum of reported consumed budgets, not an original world clock. |
| `pending_state` | Attachment target requested before an interrupted update returned. |
| `stop_reason` | Empty while continuing; `tick_limit` or the explicit terminal/boundary reason on the last row. |

A stopped row may reflect a partially completed update, with the last active
animation and a requested transition. It must not be interpreted as a complete
original world tick. Stderr reports the same stop and the scenario limitations;
stdout remains parseable CSV.

## Validation performed

The tool builds on x64 and x86. On both architectures, 600 ticks at `dt=0.02`
passed for `MM_CLIMB_002`, `MM_FLOAT`, `MM_FLOAT_CLIMB2` and patched `VH22_KF1`.
`MM_CLIMB_002` also passed 600 ticks at `dt=0.125` and a zero-tick run. Row counts,
states and stop reasons agreed; finite numeric fields agreed within `2e-11` and
requested/consumed/remaining accounting was checked on every row.

An x64 pass over all 102 effective installed levels used 3,000 ticks at `dt=0.1`;
each completed or explicitly stopped. Two representative boundaries were then
checked on both architectures with identical CSV:

- Patched `VH22_KF1`: terminal `0x1A` at tick 250, 0.05 consumed and 0.05 left.
- Patched `VH09_MT`: gadget boundary at tick 1, 0.1 consumed and zero left.

Missing/unknown options, partial numeric tokens, negative/out-of-range counts,
nonfinite/nonpositive `dt`, and an out-of-range emitter were rejected; `--help`
succeeded. Installed binaries/assets were read in place. These are source
composition checks using real fixtures, **not native comparisons of a complete
actor lifetime**. Original-byte comparisons of the individual recovered routines
remain in their separate evidence notes.
