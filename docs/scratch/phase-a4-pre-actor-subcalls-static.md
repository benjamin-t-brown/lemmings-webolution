# Phase A4ae — Static Disassembly Audit of `0x437BA3` (`0x441879`) and `0x43A9BB` / `0x43AA47` in `0x43A4EC`

## Scope and Question

Following `A4aa` (`docs/scratch/phase-a4-zoom1-filter-provenance.md`), this static-only round audits the two unresolved subcall sites from the `0x441845` frame-draw trace against the authority binary `C:\progs\Lemmings Revolution\Lemmings Revolution.exe` (SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`):

1. `0x441879: call 0x437ba3` (`ecx = [ebp-0xc] + 0x5140 = 0x4C6A08 + 0x5140 = 0x4CBB48`).
2. `0x43A9BB: call dword ptr [edx + 0x10]` and `0x43AA47: call dword ptr [edx + 0x10]` inside `0x43A4EC` (`[0x43A908, 0x43AA63)`), where `this` (`[ebp-0x58]`) is `0x4C6A08 + 8 = 0x4C6A10`.

No live game process or Frida session was launched.

## Artifacts and Hashes

- Worker static verifier: `build/tmp/a4ae-pre-actor-subcalls-static/verify_a4ae.py`
  - SHA-256: `0d74543696edfcd7ca0fd5224f5e6beeebf9395356567d318f2ab35cf977498b`
- Worker JSON report: `build/tmp/a4ae-pre-actor-subcalls-static/verified.json`
-Worker predictions and round note:
  - `build/tmp/a4ae-pre-actor-subcalls-static/predictions.md`
  - `build/tmp/a4ae-pre-actor-subcalls-static/round-note.md`
- Independent facilitator verifier & output:
  - `build/tmp/a4ae-facilitator/verify_a4ae_facilitator.py`
  - `build/tmp/a4ae-facilitator/facilitator_verified.json`

## 1. Subcall 1 — `0x441879: call 0x437ba3`

### Call Context in `0x441845` (`[0x441870, 0x441880)`)

- `0x441870` (`8b 4d f4`): `mov ecx, dword ptr [ebp - 0xc]` (`ecx = 0x4C6A08`)
- `0x441873` (`81 c1 40 51 00 00`): `add ecx, 0x5140` (`ecx = 0x4CBB48`)
- `0x441879` (`e8 25 63 ff ff`): `call 0x437ba3`

### Disassembly of `0x437BA3` (`[0x437BA3, 0x437BB9)`) and `0x47496D` (`[0x47496D, 0x4749BF)`)

- `[0x437BA3, 0x437BB9)` (file offset `0x37BA3`):
  - `0x437BA3` (`55`): `push ebp`
  - `0x437BA4` (`8b ec`): `mov ebp, esp`
  - `0x437BA6` (`51`): `push ecx`
  - `0x437BA7` (`89 4d fc`): `mov dword ptr [ebp - 4], ecx`
  - `0x437BAA` (`8b 4d fc`): `mov ecx, dword ptr [ebp - 4]`
  - `0x437BAD` (`83 c1 10`): `add ecx, 0x10` (`ecx = 0x4CBB58`)
  - `0x437BB0` (`e8 b8 cd 03 00`): `call 0x47496d`
  - `0x437BB5` (`8b e5`): `mov esp, ebp`
  - `0x437BB7` (`5d`): `pop ebp`
  - `0x437BB8` (`c3`): `ret`
- `[0x47496D, 0x4749BF)` (file offset `0x7496D`):
  - `0x47496D` (`55`): `push ebp`
  - `0x47496E` (`8b ec`): `mov ebp, esp`
  - `0x474970` (`51`): `push ecx`
  - `0x474971` (`89 4d fc`): `mov dword ptr [ebp - 4], ecx`
  - `0x474974` (`68 d8 d3 58 00`): `push 0x58d3d8`
  - `0x474979` (`e8 32 b4 fe ff`): `call 0x45fdb0`
  - `0x47497E` (`83 c4 04`): `add esp, 4`
  - `0x474981` (`8b 45 fc`): `mov eax, dword ptr [ebp - 4]`
  - `0x474984` (`8b 0d 24 ce 58 00`): `mov ecx, dword ptr [0x58ce24]`
  - `0x47498A` (`89 48 30`): `mov dword ptr [eax + 0x30], ecx`
  - `0x47498D` (`8b 55 fc`): `mov edx, dword ptr [ebp - 4]`
  - `0x474990` (`8b 42 34`): `mov eax, dword ptr [edx + 0x34]`
  - `0x474993` (`a3 24 ce 58 00`): `mov dword ptr [0x58ce24], eax`
  - `0x474998` (`8b 4d fc`): `mov ecx, dword ptr [ebp - 4]`
  - `0x47499B` (`8b 49 04`): `mov ecx, dword ptr [ecx + 4]`
  - `0x47499E` (`8b 55 fc`): `mov edx, dword ptr [ebp - 4]`
  - `0x4749A1` (`8b 42 04`): `mov eax, dword ptr [edx + 4]`
  - `0x4749A4` (`8b 10`): `mov edx, dword ptr [eax]`
  - `0x4749A6` (`52`): `push edx`
  - `0x4749A7` (`ff 52 04`): `call dword ptr [edx + 4]`
  - `0x4749AA` (`e8 31 72 fc ff`): `call 0x43bbe0`
  - `0x4749AF` (`8b 45 fc`): `mov eax, dword ptr [ebp - 4]`
  - `0x4749B2` (`8b 48 30`): `mov ecx, dword ptr [eax + 0x30]`
  - `0x4749B5` (`89 0d 24 ce 58 00`): `mov dword ptr [0x58ce24], ecx`
  - `0x4749BB` (`8b e5`): `mov esp, ebp`
  - `0x4749BD` (`5d`): `pop ebp`
  - `0x4749BE` (`c3`): `ret`

### Reachability Summary for `0x437BA3`

- Across the entire depth-4 direct `E8` reachability tree (`0x437BA3`, `0x47496D`, `0x45FDB0` `[0x45FDB0, 0x45FE00)`, `0x43BBE0` `[0x43BBE0, 0x43BBED)`, `0x43BCF0` `[0x43BCF0, 0x43BE0B)`, `0x43BBF0` `[0x43BBF0, 0x43BC7D)`, `0x43BE80` `[0x43BE80, 0x43BEEB)`, `0x43BE30` `[0x43BE30, 0x43BE6B)`, `0x43BE10` `[0x43BE10, 0x43BE25)`):
  - `0x45FDB0(0x58D3D8)` (`-> 0x43BCF0 -> 0x43BE80`) pushes a `0x60`-byte transform record onto stack `0x58D5A0` and updates `[0x58D59C]`.
  - `0x43BBE0()` (`-> 0x43BBF0 -> 0x43BE30 / 0x43BE10`) pops the top record from `0x58D5A0` and copies `0x10` dwords (`0x40` bytes) back into `0x58D3D8`.
  - **Direct `E8` calls to `0x470F29` (`SetFilterMode`):** `0`.
  - **Immediate references to `0x4C24E0`:** `0`.
  - **Indirect calls:** `1` (`0x4749A7: call dword ptr [edx + 4]` on `ecx = [0x4CBB58 + 4]`).

---

## 2. Subcall 2 — `0x43A9BB` and `0x43AA47` in `0x43A4EC` (`[0x43A908, 0x43AA63)`)

### Disassembly of the `this + 0x1E88` (`0x4C8898`) Downward Loop in `0x43A4EC`

In `0x441845`, `0x4418AD..0x4418BA` calls `0x43A4EC` with `ecx = [ebp-0xc] + 8 = 0x4C6A08 + 8 = 0x4C6A10` (stored in `[ebp-0x58]` at `0x43A50C`). Immediately after `0x43A908: call 0x470faf` (which sets stage-0 state `0x0C` at `0x470FD6`, not `MAGFILTER`/`MINFILTER` `0x10`/`0x11`):

- `0x43A908` (`e8 a2 66 03 00`): `call 0x470faf`
- `0x43A90D..0x43A924`: computes visible column range `[ebp-0x18]` and `[ebp-0x38]` from `this + 0x1E78` (`0x43BB60`) and `this + 0x1EA8` (`0x470DD0`).
- `0x43A93A` (`8b 4d a8`): `mov ecx, dword ptr [ebp - 0x58]` (`0x4C6A10`)
- `0x43A93D` (`81 c1 88 1e 00 00`): `add ecx, 0x1e88` (`0x4C8898`)
- `0x43A943` (`e8 38 8a fe ff`): `call 0x423380` — `[0x423380, 0x42338E)` returns `eax = dword ptr [ecx + 0xc]` (base pointer of the `4`-byte element pointer array).
- `0x43A948` (`89 45 c0`): `mov dword ptr [ebp - 0x40], eax`
- `0x43A94B` (`8b 4d a8`): `mov ecx, dword ptr [ebp - 0x58]`
- `0x43A94E` (`81 c1 88 1e 00 00`): `add ecx, 0x1e88`
- `0x43A954` (`e8 27 12 00 00`): `call 0x43bb80` — `[0x43BB80, 0x43BB8E)` returns `eax = dword ptr [ecx + 4]` (element count).
- `0x43A959` (`83 e8 01`): `sub eax, 1`
- `0x43A95C` (`89 45 d8`): `mov dword ptr [ebp - 0x28], eax` (loop index `i = count - 1` down to `0`)
- Loop body (`[0x43A961, 0x43AA4A)`):
  - Loads `elem = ((uint32_t*)[ebp-0x40])[i]`.
  - Compares `qword ptr [elem + 0x18]` against `qword ptr [0x4B02D0]` (`1.0`):
    - **Branch A (`[elem + 0x18] >= 1.0`, `0x43A98F..0x43A9C0`):**
      - Copies `byte ptr [elem + 0x28]` to `byte ptr [elem + 0x29]`.
      - If `byte ptr [elem + 0x28] != 0`:
        - `0x43A9AB..0x43A9B9`: loads `ecx = elem` and `edx = dword ptr [elem]` (vtable).
        - `0x43A9BB` (`ff 52 10`): `call dword ptr [edx + 0x10]`
    - **Branch B (`[elem + 0x18] < 1.0`, `0x43A9C0..0x43AA4A`):**
      - Converts `qword ptr [elem + 0x08]` via `0x40DEC0(this + 0x24)` and `0x49C5AC`, shifts `sar eax, 3`, tests whether `eax` falls in `[ebp-0x18]..[ebp-0x38]` and `byte ptr [elem + 0x28] != 0`, and writes the resulting boolean byte to `byte ptr [elem + 0x29]`.
      - If `byte ptr [elem + 0x29] != 0`:
        - `0x43AA37..0x43AA45`: loads `ecx = elem` and `edx = dword ptr [elem]` (vtable).
        - `0x43AA47` (`ff 52 10`): `call dword ptr [edx + 0x10]`

### Exhaustive `.text` Audit of `this + 0x1E88` (`0x4C6A10 + 0x1E88 = 0x4C8898`) and Push Helper `0x43B517`

Across the entire `.text` section (`[0x401000, 0x49EC00)`), displacement `0x1E88` (`88 1e 00 00`) appears at 10 sites, and the **only** push/append helper into `this + 0x1E88` is `0x43B517` (`[0x43B517, 0x43B534)`):

- `0x43B523` (`81 c1 88 1e 00 00`): `add ecx, 0x1e88`
- `0x43B529` (`e8 72 7e fe ff`): `call 0x4233a0` (appends `[ebp+8]` into the pointer array at `ecx + 0x0C`)

Across `.text`, `0x43B517` has **exactly 5 direct `E8` call sites**, all passing `ecx = 0x4C6A10` (`b9 10 6a 4c 00`):

| Call site VA | Enclosing function VA | Pushed `this` local | Object vtable VA | Vtable assignment / provenance | `vtable + 0x10` target (`call [edx+0x10]`) |
| --- | --- | --- | --- | --- | --- |
| `0x42A241` | `0x428A47` | `[ebp - 0x694]` | `0x4B1020` | `0x428B6B` (`c7 00 20 10 4b 00`: `mov dword ptr [eax], 0x4b1020`) | `0x42C5C9` |
| `0x42E153` | `0x42DE51` (Lemming ctor) | `[ebp - 0x60]` | `0x4B10E8` | `0x42DF1D` (`c7 02 e8 10 4b 00`: `mov dword ptr [edx], 0x4b10e8`) | `0x42EEF2` (`Lemming::Render`) |
| `0x42FCD1` | `0x42FB5F` (Lemming deserialize) | `[ebp - 0x50]` | `0x4B10E8` | `0x4B10E8 + 0x18` (`0x4B1100` -> `0x42FB5F`) | `0x42EEF2` (`Lemming::Render`) |
| `0x43212A` | `0x432080` | `[ebp - 0x28]` | `0x4B0E00` | `0x4320CC` (`c7 02 00 0e 4b 00`: `mov dword ptr [edx], 0x4b0e00`) | `0x4323EB` |
| `0x43231D` | `0x432283` | `[ebp - 0x14]` | `0x4B0E00` | `0x4B0E00 + 0x18` (`0x4B0E18` -> `0x432283`) | `0x4323EB` |

*(Facilitator narrowing of worker `verify_a4ae.py`: the worker's backward `55 8b ec` byte scan `find_function_prologue` hit mid-instruction `55 8b ec` bytes at `0x42A0EB` inside `0x428A47` and `0x42FBDB` inside `0x42FB5F`, and conflated the lemming's `+0x340` subobject constructor `0x415FA0` / `vtable 0x4B0A20` called at `0x42FC7F` with the `this` pointer `[ebp - 0x50]` pushed at `0x42FCD1`. Independent facilitator verification `build/tmp/a4ae-facilitator/verify_a4ae_facilitator.py` pins the exact 5 enclosing functions and 3 pushed vtables above without heuristic prologue scanning.)*

---

## 3. Architectural Correction to `A4x` and `A4aa`: Where Lemmings Are Drawn

1. **`A4x` / `A4aa` assumption:**
   - `A4x` (`docs/scratch/phase-a4-actor-filter-state.md` lines 31–33) and `A4aa` (`docs/scratch/phase-a4-zoom1-filter-provenance.md` line 37) treated the post-tube loop in `0x441845` at `0x4418D2..0x441910` (iterating `this + 0x1F70 = 0x4C6A08 + 0x1F70 = 0x4C8978` and calling `call dword ptr [edx + 0x10]` at `0x44190D`) as the lemming/actor draw loop, and therefore placed `0x43A908: call 0x470faf`, `0x43A9BB` / `0x43AA47`, and `0x43AC8A: call 0x470faf` **before** lemming draw.
2. **Static proof from `0x43B517` and `0x448BD0`:**
   - `0x4C6A08 + 0x1F70` (`0x4C8978`, iterated at `0x4418D2..0x441910`) is populated only at `0x448C38..0x448C86` inside `0x448BD0` (`push 0x2a8; call 0x49c1a8; call 0x428680; push eax; add ecx, 0x1f70; call 0x4233a0`), which constructs `0x2A8`-byte objects with **`vtable 0x4B1020`** (`+0x10 = 0x42C5C9`).
   - **Lemmings (`vtable 0x4B10E8`, `+0x10 = 0x42EEF2`) are NEVER pushed into `0x4C6A08 + 0x1F70`.** Instead, both the lemming constructor (`0x42DE51` at `0x42E153`) and the lemming deserialization method (`0x42FB5F` at `0x42FCD1`) push `this` (`vtable 0x4B10E8`) into **`0x4C6A10 + 0x1E88` (`0x4C8898`)** via `0x43B517`.
   - Therefore, **`0x42EEF2` (`Lemming::Render`, `vtable 0x4B10E8 + 0x10`) is invoked INSIDE `0x43A4EC` at `0x43A9BB` / `0x43AA47`** during the downward `0x43A90D..0x43AA63` walk of `0x4C6A10 + 0x1E88` (`0x4C8898`)!
3. **Consequences for the pre-lemming filter state:**
   - `0x43A908: call 0x470faf` (before the `0x43A90D..0x43AA63` `0x1E88` loop) and `0x43AC8A: call 0x470faf` (after the `0x1E88` loop) write stage-0 state `0x0C` (`0x470FD6 6a0c6a00`), not `MAGFILTER`/`MINFILTER` (`0x10`/`0x11`).
   - Because `0x43A90D..0x43AA63` walks `0x4C6A10 + 0x1E88` (`0x4C8898`) **downward** (`i = count - 1` down to `0`) inside `0x43A4EC`, any preceding element at a higher index `j > i` in `0x4C8898` (`0x4B1020 -> 0x42C5C9`, `0x4B10E8 -> 0x42EEF2`, or `0x4B0E00 -> 0x4323EB`) — and any post-loop `0x44190D` (`0x4B1020 -> 0x42C5C9`) or earlier-frame `0x47A75D` exit (`0x47AB8B: call 0x470f29(1)`) — is on the path leading into a lemming's `0x4704E0` material binder call!

## Next Step (`A4af`)

Audit the three `vtable + 0x10` renderers in `0x4C6A10 + 0x1E88` (`0x42C5C9` for `0x4B1020`, `0x4323EB` for `0x4B0E00`, and `0x42EEF2` for `0x4B10E8`) and all `.text` callers of `0x47A75D` / `0x470F29` statically against `Lemmings Revolution.exe` (completed in `docs/scratch/phase-a4-tube-child-renderers-filter-static.md`).
