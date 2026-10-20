# Phase A4af — Static Audit of Tube Child Renderers (`0x42C5C9`, `0x4323EB`, `0x42EEF2`), `0x47A75D` Control Flow, and Whole-Binary `0x470F29` / `0x4C24E0` / `SetTextureStageState` Call Sites

## Scope and Question

Following `A4ae` (`docs/scratch/phase-a4-pre-actor-subcalls-static.md`), this static-only round audits the three `vtable + 0x10` render methods in `0x4C6A10 + 0x1E88` (`0x4C8898`) — `0x42C5C9` (`vtable 0x4B1020 + 0x10`), `0x4323EB` (`vtable 0x4B0E00 + 0x10`), and `0x42EEF2` (`vtable 0x4B10E8 + 0x10`, `Lemming::Render`) — together with `0x47A75D` and the complete `.text` inventory of `0x470F29` (`SetFilterMode`), `0x4C24E0`, and `call dword ptr [reg + 0xA0]` (`SetTextureStageState`) in `C:\progs\Lemmings Revolution\Lemmings Revolution.exe` (SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`).

No live game process or Frida session was launched.

## Artifacts and Hashes

- Worker static verifier: `build/tmp/a4af-tube-child-renderers-filter-static/verify_a4af.py`
  - SHA-256: `b8bc2617d71b1609201a3afede9a517e1928e55567099de8eb411f3c50af4f91`
- Worker JSON report: `build/tmp/a4af-tube-child-renderers-filter-static/verified.json`
- Worker predictions and round note:
  - `build/tmp/a4af-tube-child-renderers-filter-static/predictions.md`
  - `build/tmp/a4af-tube-child-renderers-filter-static/round-note.md`
- Independent facilitator verifier & output:
  - `build/tmp/a4af-facilitator/verify_a4af_facilitator.py`
  - `build/tmp/a4af-facilitator/facilitator_verified.json`

## 1. Exact Disassembly of the Three `vtable + 0x10` Renderers in `0x4C8898` and How `0x42EEF2` Reaches `0x47A75D`

1. **`0x42EEF2` (`vtable 0x4B10E8 + 0x10`, `Lemming::Render`, `[0x42EEF2, 0x42F183)`):**
   - Has **zero** direct `E8` calls to `0x470F29` or `0x470FAF` and **zero** references to `0x4C24E0`.
   - At `0x42F080..0x42F0DF`:
     - Tests `byte ptr [this + 0x38c]` (`0x42F088: 8a 91 8c 03 00 00`).
     - When `byte ptr [this + 0x38c] == 0` (`0x42F090: 74 38` -> `0x42F0CA`):
       - `0x42F0CA` (`8b 8d 6c ff ff ff`): `mov ecx, dword ptr [ebp - 0x94]` (`lemming`)
       - `0x42F0D0` (`83 c1 78`): `add ecx, 0x78` (`ecx = lemming + 0x78`)
       - `0x42F0D3` (`8b 85 6c ff ff ff`): `mov eax, dword ptr [ebp - 0x94]`
       - `0x42F0D9` (`8b 50 78`): `mov edx, dword ptr [eax + 0x78]` (`vtable = 0x4B14A8`)
       - `0x42F0DC` (`ff 52 04`): `call dword ptr [edx + 4]` -> **`0x43CC68`** (`*(uint32_t*)(0x4B14A8 + 4) = 0x43CC68`).
     - Inside **`0x43CC68` (`[0x43CC68, 0x43CD57)`)**:
       - `0x43CD45` (`8b 4d e4`): `mov ecx, dword ptr [ebp - 0x1c]` (`lemming + 0x78`)
       - `0x43CD48` (`81 c1 b0 00 00 00`): `add ecx, 0xb0` (`ecx = lemming + 0x78 + 0xb0 = lemming + 0x128`)
       - `0x43CD4E` (`e8 0a da 03 00`): `call 0x47a75d`
     - (When `byte ptr [this + 0x38c] != 0`, `0x42F0C3` calls `0x43C9D2` on `ecx = lemming + 0x78`, which at `0x43CABF..0x43CAC8` adds `0xb0` and calls `0x47A190`, the scaled sprite path, which contains no `0x470F29` calls.)
   - *(Correction to `A4ae` next-step prose: `0x42EEF2` reaches `0x47A75D` via `lemming+0x78` `vtable 0x4B14A8 + 4` -> `0x43CC68` -> `0x43CD4E`, not via `0x4752C0` / `0x475A50`.)*

2. **`0x4323EB` (`vtable 0x4B0E00 + 0x10`, `[0x4323EB, 0x432410)`):**
   - `0x4323F2..0x432409`: if `dword ptr [this + 0x44] != 0`, loads `ecx = dword ptr [this + 0x44]`, `edx = dword ptr [ecx]`, and dispatches `0x432409` (`ff 52 04`: `call dword ptr [edx + 4]`).
   - Contains **zero** direct `E8` calls (`0` calls to `0x470F29` or `0x470FAF`) and **zero** references to `0x4C24E0`.

3. **`0x42C5C9` (`vtable 0x4B1020 + 0x10`, `[0x42C5C9, 0x42C714)`):**
   - Updates sub-transforms on `[this + 0x158] + 0x128` via `0x412C90` / `0x412760` when `[this + 0x1B1]` changes, and at `0x42C709` (`e8 dd 5c 00 00`) tail-calls **`0x4323EB`**.
   - Contains **zero** calls to `0x470F29` or `0x470FAF` and **zero** references to `0x4C24E0`.

---

## 2. Complete Control-Flow Verification of `0x47A75D` (`[0x47A75D, 0x47AB96)`)

1. **Entry filter setter (`0x47A766..0x47A776`):**
   - `0x47A766` (`83 3d 3c d5 58 00 00`): `cmp dword ptr [0x58d53c], 0`
   - `0x47A76D` (`75 0c`): `jne 0x47a77b`
   - `0x47A76F` (`6a 00`): `push 0`
   - `0x47A771` (`b9 80 cd 58 00`): `mov ecx, 0x58cd80`
   - `0x47A776` (`e8 ae 67 ff ff`): `call 0x470f29`
2. **Draw body (`[0x47A77B, 0x47AB84)`):**
   - Contains **zero** `ret` instructions (the only `ret` in `[0x47A75D, 0x47AB96)` is `0x47AB93: c2 10 00` `ret 0x10`).
   - Backward CFG reachability in `build/tmp/a4af-facilitator/verify_a4af_facilitator.py` (with `call` instructions properly falling through to the next instruction, narrowing the worker's `verify_a4af.py` `fall_through()` which returned `None` on `call` and produced `paths_to_ret_enumerated = 0`) proves that **zero** instructions before `0x47AB84` can reach `0x47AB93` (`ret 0x10`) without executing `0x47AB84..0x47AB8B`.
3. **Exit filter setter (`0x47AB84..0x47AB93`):**
   - `0x47AB7F` (`e8 12 00 00 00`): `call 0x47ab96`
   - `0x47AB84` (`6a 01`): `push 1`
   - `0x47AB86` (`b9 80 cd 58 00`): `mov ecx, 0x58cd80` *(note: `ecx` is `0x58CD80`, not `0x4C24D0`)*
   - `0x47AB8B` (`e8 99 63 ff ff`): `call 0x470f29`
   - `0x47AB90` (`8b e5`): `mov esp, ebp`
   - `0x47AB92` (`5d`): `pop ebp`
   - `0x47AB93` (`c2 10 00`): `ret 0x10`

---

## 3. Whole-Binary Inventory of `SetTextureStageState` (`+0xA0`), `0x4C24E0`, `0x470F29`, and `0x47A75D`

Independent facilitator verification (`build/tmp/a4af-facilitator/verify_a4af_facilitator.py`) establishes across the entire `Lemmings Revolution.exe` PE image and `.text` section (`[0x401000, 0x49EC00)`):

1. **All `.text` `call dword ptr [reg + 0xA0]` (`SetTextureStageState`) sites (`24` total):**
   - `0x470F86` and `0x470FA3` inside `0x470F29` (`SetFilterMode`: stage `0`, states `0x10` `MAGFILTER` and `0x11` `MINFILTER`).
   - `0x470FE9` inside `0x470FAF` (stage `0`, state `0x0C` `ADDRESS`).
   - `21` sites at `0x4726B1..0x47296F` inside `0x472617` (material binder: stage `0`, states `1..6` `COLOROP`/`COLORARG1`/`COLORARG2`/`ALPHAOP`/`ALPHAARG1`/`ALPHAARG2`).
   - **Conclusion:** `0x470F29` is the **only** function in `.text` that requests stage-0 `MAGFILTER` (`0x10`) or `MINFILTER` (`0x11`) via `call [reg + 0xA0]`.
2. **All `.text` references to `0x4C24E0` (`global_alpha_ref`) (`3` total):**
   - `0x470F39` (`c7 05 e0 24 4c 00 ef 00 00 00`: `mov dword ptr [0x4c24e0], 0xef`, inside `0x470F29`).
   - `0x470F65` (`c7 05 e0 24 4c 00 a0 00 00 00`: `mov dword ptr [0x4c24e0], 0xa0`, inside `0x470F29`).
   - `0x472896` (`8b 15 e0 24 4c 00`: `mov edx, dword ptr [0x4c24e0]`, read inside `0x472617` before `SetRenderState(0x18, edx)`).
   - **Conclusion:** `0x470F29` is the **only** function in `.text` that writes `[0x4C24E0]` by immediate address.
3. **All `.text` callers and PE pointer references to `0x470F29` (`SetFilterMode`):**
   - Literal 4-byte pointer `29 0f 47 00` (`0x470F29`) occurs **0 times** in the entire PE file (`Lemmings Revolution.exe`), so `0x470F29` is not stored in any vtable or static function table.
   - Across `.text`, `0x470F29` has **exactly 10 `E8` call sites**:
     - `0x47022B` (device init in `0x46FFF0`: `push *[ecx+0x4b4]; call 0x470f29`).
     - `0x470F1E` (inside `0x470F06`: stores `byte [ecx+0x3a1] = arg0` and calls `0x470F29(0)`; `0x470F06` has a single `.text` caller at `0x45F2B4`).
     - **Four bracketed `(entry, exit)` pairs** (`ecx = 0x58CD80`), each of which calls **`0x470F29(1)`** before returning/exiting:
       1. `0x44E150`: entry `0x44E165` (`push [this+0x10]`), exit `0x44E1A5` (`push 1; mov ecx, 0x58cd80; call 0x470f29`).
       2. `0x454C82..0x454CB8`: entry `0x454C89` (`push 0`), exit `0x454CAF` (`push 1; mov ecx, 0x58cd80; call 0x470f29`).
       3. `0x45B639` (post-frame HUD loop called at `0x441A71`): loop entry `0x45B6D9` (`push 0`), loop exit `0x45B706` (`push 1; mov ecx, 0x58cd80; call 0x470f29`).
       4. `0x47A75D`: entry `0x47A776` (`push 0`, skipped when `[0x58D53C] != 0`), exit `0x47AB8B` (`push 1; mov ecx, 0x58cd80; call 0x470f29`).
4. **All `.text` callers of `0x47A75D` (`2` total):**
   - `0x43CD4E` (inside `0x43CC68`, `vtable 0x4B14A8 + 4`, invoked by `Lemming::Render` `0x42EEF2` at `0x42F0DC`).
   - `0x45B784` (inside `0x45B639`, after the `0x45B6D9..0x45B706` loop exits via `0x470F29(1)`).
5. **Static Consequence for Zoom0 vs Zoom1 (`[0x58D53C]`) in `0x47A75D`:**
   - Every steady-state bracketed caller of `0x470F29(0)` in `.text` (`0x44E150`, `0x454C82..0x454CAF`, `0x45B6D2..0x45B706`, and `0x47A75D`) restores `0x470F29(1)` on exit (`0x44E1A5`, `0x454CAF`, `0x45B706`, `0x47AB8B`), and neither `0x43CD4E` nor `0x45B784` lies inside another function's `0x470F29(0)..0x470F29(1)` bracket.
   - Therefore:
     - When `[0x58D53C] == 0` (zoom0), `0x47A75D` executes `0x47A776: call 0x470f29(0)` before `0x47AB7F` (selecting `MAGFILTER=1, MINFILTER=1`, `[0x4C24E0] = 0xEF`) and restores `0x470F29(1)` at `0x47AB8B`.
     - When `[0x58D53C] != 0` (zoom1), `0x47A76D: jne 0x47a77b` skips `0x47A776: call 0x470f29(0)`, so `0x47A75D` reaches `0x47AB7F -> 0x47AB96 -> 0x472617` under the `0x470F29(1)` state left by the preceding bracket exit (`0x47AB8B` / `0x45B706` / `0x44E1A5` / `0x454CAF`) — which, when `[0x58CD80 + 0x3A1] != 0`, sets `[0x4C24E0] = 0xA0` and requests `MAGFILTER=2, MINFILTER=2`.
     - *(Note: as established in `A4r`/`A4aa`, whether `0x472617` re-sends `ALPHAREF = [0x4C24E0]` on a given draw still depends on whether the material pointer cache `[0x5CD5F4]` or flags cache `[0x4C24E4]` misses at `0x472626` / `0x47265C`, and effective GPU/wrapper rasterization is not claimed from static bytes alone.)*
