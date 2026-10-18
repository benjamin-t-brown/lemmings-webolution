# Phase 3 round 4 — maxStepUp and lemmingHeight

Static read only. The stock exe was not started, attached, or restarted.
`src/sim/World.h` was not edited.

Image: `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, base
`0x400000`. Re-run with `python tools/harness/read_steppers.py` (`--at`,
`--consts`, `--len`). Capstone 5.0.7 on the default interpreter. No UCRT64
`bin` on `PATH`.

Numbers below are world units. One cell is one world unit. Nothing here is a
mask row, and `PIXELS_PER_UNIT = 2` was not applied.

The provisional classic values in `Rules` are `maxStepUp = 3` and
`lemmingHeight = 4`. Those immediates are not what these probes use.

## Result

| Field | Read | How it sits in the exe |
| --- | --- | --- |
| `maxStepUp` | **2** | Observation. The walk probe rises by `[+0x44]` at most twice, then a third solid cell is a wall. Spawn writes `[+0x44] = 1`. There is no loaded immediate 2 or 3. |
| `lemmingHeight` | **3.0** | Observation. The climber head test probes `Y + [+0x358]`, and that double is `3.0` from construction while vertical direction is `+1`. No immediate 4 in this path. |

Both are a static read of the shipped bytes. Neither has a live series. They
stay out of `Rules` until one exists.

## Walk — the step-up is two cells, then a wall

**Observation.** Walk vtable `0x004B0D28`:

| slot | address | bytes at the slot |
| --- | --- | --- |
| +0x00 | `0x0041CB10` | destructor |
| +0x04 | `0x0041BDD4` | stepper. The dword at `0x004B0D2C` is `d4 bd 41 00` |
| +0x08 | `0x0041BDBA` | enter hook |

The stepper has no direct `E8` caller. Dispatch is virtual, same shape as the
climb.

**Observation.** `0x0041BDD4`–`0x0041BF5E` (`ret 8`) loads three `.rdata`
doubles and no others (`--consts 0x41bdd4` through the `ret`):

| site | bytes | value |
| --- | --- | --- |
| `0x0041BDEB` | `dc 35 f8 03 4b 00` | `[0x004B03F8] = 2.0`, and only when `[lemming+0x50] == 3` |
| `0x0041BE02` | `dc 1d 38 03 4b 00` | `[0x004B0338] = 0.0`, the loop test on remaining time |
| `0x0041BE70` | `dd 05 b0 10 4b 00` | `[0x004B10B0] = 5.0`, the walk speed |

Horizontal motion is `5.0 * slice * [+0x40]` added to `X`, then `0x0040DEC0`.
That is the speed already measured. It is not a step height.

The on-grid check is the only call that can change the row. It runs after the
chunk was fully consumed (`[ebp-8]` set) and after `X` is snapped through
`0x0049C5AC`:

```
0041bee6  e8 11 01 00 00    call 0x0041bffc
```

**Observation.** `0x0041BFFC` has one direct caller, that instruction. Its
only `.rdata` double, over the whole function through `ret 4` at `0x0041C872`,
is `0.0` at `0x0041C028`. An immediate scan of `cmp` / `add` / `sub` found no
`3` and no `4`. Row changes are `add` or `sub` of `[lemming+0x44]`.

The position comes from `0x0042F22E` (called at `0x0041C069`):

```
0042f263  dd 81 60 03 00 00   fld   qword ptr [ecx+0x360]
0042f269  dc 40 08            fadd  qword ptr [eax+8]       ; X + [+0x360]
0042f275  dd 80 68 03 00 00   fld   qword ptr [eax+0x368]
0042f27b  dc 42 10            fadd  qword ptr [edx+0x10]    ; Y + [+0x368]
```

While vertical direction is `+1`, both offsets are `0.0` (bytes below), so the
probe row is the lemming's own `Y`. The function then reads two cells through
`0x0040DE60` on the world at `0x004C6A10`:

- `[ebp-0x18]` = cell at `(X, Y - [+0x44])`
- `[ebp-0x10]` = cell at `(X, Y)`, and `[ebp-0x1c]` keeps that row

**Observation.** If the cell at `Y` has bit `0x1`, the probe walks up the
forward column one direction-unit at a time, unrolled, and stops:

```
0041c47d  83 e2 01            and  edx, 1              ; cell(Y) & 1
0041c482  0f 84 67 02 00 00   je   0x41c6ef            ; clear: not a step
0041c48e  03 48 44            add  ecx, [eax+0x44]     ; row += direction
; probe. blocker bit 0x800 turns via 0x0042F66B and returns 0.
0041c517  83 e1 01            and  ecx, 1              ; cell(Y+dir) & 1
0041c51c  0f 84 8c 01 00 00   je   0x41c6ae            ; clear: adopt this row
0041c528  03 42 44            add  eax, [edx+0x44]     ; row += direction again
; probe. blocker turns, same as above.
0041c5b1  83 e0 01            and  eax, 1              ; cell(Y+2*dir) & 1
0041c5b6  0f 84 f2 00 00 00   je   0x41c6ae            ; clear: adopt this row
0041c5c1  8a 51 48            mov  dl, [ecx+0x48]      ; third solid cell
```

`0x0041C6AE` is the adopt path: it passes the row to `0x0042EE74`, which
subtracts `[+0x368]` and stores `Y`:

```
0042ee7e  dd 45 08            fld   qword ptr [ebp+8]
0042ee81  dc a0 68 03 00 00   fsub  qword ptr [eax+0x368]
0042ee92  e8 b9 f0 fd ff      call  0x0040df50          ; clamp
0042ee9a  dd 59 10            fstp  qword ptr [ecx+0x10]
```

The third solid cell does not adopt. If `[+0x48]` is set it constructs the
climb state; otherwise it turns through `0x0042F66B` and returns 0.

```
0041c5ef  e8 5c 71 ff ff      call  0x00413750
00413762  c7 00 60 09 4b 00   mov   dword ptr [eax], 0x004b0960
```

`0x00413750` has one caller, `0x0041C5EF`. `0x004B0960` is the climb vtable
(slot 1 at `0x004B0964` is `0x004137CA`). The constructor of the lemming
clears the byte first: `0x0042E0CD` `c6 41 48 00`.

**Observation.** `[+0x44]` is the integer `1` at spawn, inside `0x0042DE51`:

```
0042dfe7  c7 41 44 01 00 00 00   mov dword ptr [ecx+0x44], 1
```

`0x0042F5E3` negates it and preserves magnitude (`0x0042F5F0` `6b c9 ff`,
`imul ecx, ecx, -1`). So each `add` is one world unit, and the greatest rise
this probe writes is **2**. A stack of three solid cells (`bit 0x1`) in the
forward column is a wall.

**Hypothesis.** That rise is the quantity `Rules::maxStepUp` is standing in
for. The exe has no field of that name and no immediate `2`. The classic `3`
is not this probe's limit.

The same function's other edge, recorded so it is not folded into the step-up:
if the cell at `Y` lacks bit `0x1` and the cell at `Y - [+0x44]` is `0`, it
looks one more cell down (`0x0041C6FF`, `0x0041C741`). Two empty cells take
the transition at `0x0041C7A0` (`call 0x00415FA0`, return 1). One empty cell
with something under it writes `Y` one unit down, unless `[+0x4A]` is set.
Flat support (the cell one unit down is nonzero) leaves `Y` alone. That is a
one-cell step *down*, separate from the two-cell step up.

A copy of the position at `[ebp-0x2c]` is reduced by `[+0x44]` at `0x0041C093`
and is only passed to the destructor `0x00403EB0`. The cell probes read the
unadjusted pair at `[ebp-0x4C]` / `[ebp-0x44]`.

## Climb — the head sample is 3.0 above Y

**Observation.** Climb stepper `0x004137CA` is slot 1 of vtable `0x004B0960`.
On a consumed slice it calls the head test, and that is the test's only
direct caller:

```
004138e8  e8 d2 00 00 00    call 0x004139bf
```

`0x004139BF`–`0x00413C23` loads no `.rdata` double (`--consts 0x4139bf --len
0x400` is empty). It has no immediate `3` or `4`. It gets a position from
`0x0042F35F` at `0x004139E4`:

```
0042f394  dd 81 50 03 00 00   fld   qword ptr [ecx+0x350]
0042f39a  dc 40 08            fadd  qword ptr [eax+8]       ; X + [+0x350]
0042f3a6  dd 80 58 03 00 00   fld   qword ptr [eax+0x358]
0042f3ac  dc 42 10            fadd  qword ptr [edx+0x10]    ; Y + [+0x358]
```

`[+0x350]` is `0.0` at construction (below), so the column is the lemming's
`X`. The head test subtracts facing `[+0x40]` from that `X`, takes the biased
`Y` at `[ebp-0x1C]`, and probes:

```
00413a53  e8 08 a4 ff ff    call  0x0040de60
00413a58  83 38 00          cmp   dword ptr [eax], 0
00413a5b  75 13             jne   0x413a70          ; nonzero -> hit head
```

The taken branch logs `0x004BF2FC` (`Climber hit head`) and transitions. The
other branch, when that cell is `0` and `Y` is within the world height from
`0x0040BAA0`, probes again at `Y_biased - [+0x44]` and logs `0x004BF310`
(`Climber topping out`) when that cell is `0`. Both probes are one cell. No
span of 4 is added.

**Observation.** The bias is the double `3.0`, written as two dwords. IEEE
bytes `00 00 00 00 00 00 08 40`: low dword `0`, high dword `0x40080000`.

In `0x0042DE51` (direct callers `0x00425C30` and `0x004263AE`):

```
0042e0db  c7 80 50 03 00 00 00 00 00 00   mov dword ptr [eax+0x350], 0
0042e0e5  c7 80 54 03 00 00 00 00 00 00   mov dword ptr [eax+0x354], 0
0042e0f2  c7 81 58 03 00 00 00 00 00 00   mov dword ptr [ecx+0x358], 0
0042e0fc  c7 81 5c 03 00 00 00 00 08 40   mov dword ptr [ecx+0x35c], 0x40080000  ; +0x358 = 3.0
0042e109  c7 82 60 03 00 00 00 00 00 00   mov dword ptr [edx+0x360], 0
0042e113  c7 82 64 03 00 00 00 00 00 00   mov dword ptr [edx+0x364], 0          ; +0x360 = 0.0
0042e120  c7 80 68 03 00 00 00 00 00 00   mov dword ptr [eax+0x368], 0
0042e12a  c7 80 6c 03 00 00 00 00 00 00   mov dword ptr [eax+0x36c], 0          ; +0x368 = 0.0
```

`0x0042F5E3` rewrites the two Y offsets after `imul` by `-1` on `[+0x44]`.
Its direct callers are `0x0040BB59`, `0x0041AE1B`, and `0x00425C66`.

When the new `[+0x44]` is `1`:

```
0042f60c  c7 82 58 03 00 00 00 00 00 00
0042f616  c7 82 5c 03 00 00 00 00 08 40   ; +0x358 = 3.0
0042f623  c7 80 68 03 00 00 00 00 00 00
0042f62d  c7 80 6c 03 00 00 00 00 00 00   ; +0x368 = 0.0
```

When it is not `1`:

```
0042f63c  c7 81 58 03 00 00 00 00 00 00
0042f646  c7 81 5c 03 00 00 00 00 00 00   ; +0x358 = 0.0
0042f653  c7 82 68 03 00 00 00 00 00 00
0042f65d  c7 82 6c 03 00 00 00 00 08 40   ; +0x368 = 3.0
```

Upright (`[+0x44] == 1`, which is also the spawn write), the head test samples
**3.0** above the stored `Y`, and the walk probe's own offset `[+0x368]` is
`0.0`. After a vertical flip the `3.0` moves onto the walk offset. The walk
write subtracts `[+0x368]` again in `0x0042EE74`, so that swap changes which
row the walk samples and does not change the rise of 1 or 2. The head test
does not subtract the bias; the cell it calls the head is the biased one.

**Hypothesis.** `3.0` is the quantity `Rules::lemmingHeight` is standing in
for: the head-room sample used by `0x004139BF`. It is not a length loaded
inside that function. It is the Y bias of the position helper the function
calls, present from `0x0042DE51` and put back on `[+0x358]` whenever
`0x0042F5E3` leaves `[+0x44] == 1`.

## What was searched

The walk stepper, its chunk `0x0041BF61`, the walk probe `0x0041BFFC`, the
climb stepper, and the head test `0x004139BF` were disassembled through their
`ret`. `.rdata` doubles were listed with `--consts`. Integer `add` / `sub` /
`cmp` immediates in the walk probe were listed from a capstone pass. Field
offsets `+0x350`, `+0x358`, `+0x360`, and `+0x368` were found by scanning
`.text` for a disp32 memory operand (modrm mod `2`). The high dwords of the
doubles `3.0` (`0x40080000`) and `4.0` (`0x40100000`) were scanned as
immediates across `.text`. Callers are an `E8 rel32` scan of `.text`.

- The walk stepper's doubles are `2.0`, `0.0`, and `5.0`, as the lead said.
  The chunk next to it also loads `1.0` at `0x004B02D0` (`0x0041BFE3`), the
  on-grid fallback of one column, already described for the climb and the
  fall. No step height.
- No instruction in the walk probe or the head test adds or compares an
  immediate `3` or `4`.
- `0x40080000` occurs 7 times in `.text`. Three of them are the lemming
  stores above (`0x0042E0FC`, `0x0042F616`, `0x0042F65D`). The other four are
  not this probe: `push 0x40080000` at `0x00408260` (into `0x0041DD86`) and
  `0x004115D9` (into `0x00412810`), plus `0x0043B39E` and `0x0045347E`.
- `0x40100000` occurs 34 times. None of them is inside `0x0041BDD4`,
  `0x0041BFFC`, `0x004137CA`, `0x004139BF`, `0x0042F22E`, `0x0042F35F`,
  `0x0042EE74`, `0x0042F5E3`, or the `+0x350..+0x36F` stores in `0x0042DE51`.
  The classic `4` was not found on this path.

Aligned stores of `+0x350`, `+0x358`, `+0x360`, and `+0x368` are the
constructor zeros, the `3.0` pair, the flip, and the `fld` / `fadd` / `fsub`
in the two position helpers and in `0x0042EE74` / `0x0042F1C6`. No other
store of these four doubles showed up in that scan.
