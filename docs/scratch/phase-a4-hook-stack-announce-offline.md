# A4ad: offline post-attach `HOOK_INSTALL` ordering and mid-call TSS stack-tuple repair verified under `node:vm`

Queue `a4ad-hook-stack-announce-offline`, worker `round-agent`
(`7be7f62b-b69f-45cf-ab16-7822259102e5`). Offline synthetic regression only:
zero game launches, zero Frida attachments, zero native providers. Worker
packet: `build/tmp/a4ad-hook-stack-announce-offline/round-note.md`.

## Verdict

The two remaining non-binder trace-script defects from A4ab (`HOOK_INSTALL`
emitted before `Interceptor.attach` succeeded, and mid-call `0x470F86`/`0x470FA3`
`esp[0]` mislabelled as `retaddr` when it is the pushed `device` pointer) are
repaired and protected by offline `node:vm` regression tests in
`build/tmp/a4ad-hook-stack-announce-offline/hooks_a4ad_candidate.js`
(SHA-256 `1445dad5da69fa799661b7a62b190026c80a941a26e455eb7bb9129d547dbc1b`)
and `build/tmp/a4ad-hook-stack-announce-offline/test/run.mjs`. Both
`build/tmp/a4ab-zoom1-filter-requests/hooks_a4ab.js` (`52c5f966...`) and
`build/tmp/a4ac-actor-binder-offline/hooks_a4ac_candidate.js` (`9100d0d0...`)
remain byte-identical, and the repaired A4ac `snapshotActors` body is preserved
verbatim.

This is **offline harness verification only**. It does not diagnose the A4ab
`D3DImm.DLL` crash, certify that five-hook or mid-instruction (`0x470F86` /
`0x470FA3`) Frida instrumentation is safe in the live stock process, achieve
live hook-unload readback, establish effective zoom1 filter/alpha state, or
explain A4z's full-scale pixel failure.

## Defects repaired in `hooks_a4ad_candidate.js`

1. **Post-attach `HOOK_INSTALL` and explicit failure reporting:**
   In `hooks_a4ab.js` / `hooks_a4ac_candidate.js` (lines 483–489), all five
   `HOOK_INSTALL` events were emitted inside an initial loop before the first
   `Interceptor.attach` ran. `hooks_a4ad_candidate.js` introduces
   `attachWithAnnounce(addr, kind, spec)` which executes
   `Interceptor.attach(addr, spec)` first, emits `HOOK_INSTALL` only after
   `attach` returns, and catches any attach throw to emit `HOOK_ATTACH_FAIL`
   without falsely claiming subsequent hooks installed (`TSS10`/`TSS11` attach
   only if `SETTER` attach succeeds).
2. **Mid-call `0x470F86` / `0x470FA3` pre-call stdcall stack layout vs function-entry `0x470F29`:**
   At `0x470F86` (`ff90a0000000`, `call dword ptr [eax+0xa0]`) and `0x470FA3`
   (`ff91a0000000`, `call dword ptr [ecx+0xa0]`), the `call` instruction has not
   yet pushed a return address when `onEnter` runs. The four stdcall arguments
   are already on the stack at `esp+0` (`device`), `esp+4` (`stage`), `esp+8`
   (`state`), and `esp+12` (`value`). `hookTss` now records `retaddr: null` and
   `stack_convention: 'pre_call_stdcall_4'` while reading `device: hexN(esp.readU32())`,
   `stage: u32(esp.add(4))`, `state: u32(esp.add(8))`, and
   `value: u32(esp.add(12))` (and `TARGET_SETTER.tss10`/`tss11` compact records
   preserve `retaddr: null`). Function-entry `SETTER` (`0x470F29`, `55 8bec`)
   continues to record the caller's true return address at `esp[0]` via
   `retaddr: esp.readPointer().toString()` and `arg` at `esp+4`.

## Executable byte pins re-read (`0x470F29..0x470FAE`, 134 bytes)

Authority SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
file offset `0x70F29`, chunk SHA-256
`ae58b1faba935652a35df4eccf726dd10b24f98723bf87f6b6d0a3b33d2e2fd5`:

- `0x470F29`: `55 8bec 83ec08 894df8` (`push ebp; mov ebp, esp; sub esp, 8; mov [ebp-8], ecx`).
- `0x470F32..0x470F6E`: default `[ebp-4] = 1`, `[0x4C24E0] = 0xEF`; if `(arg & 0xff) != 0`
  and `byte ptr [ecx+0x3a1] != 0`, overwrite `[ebp-4] = 2`, `[0x4C24E0] = 0xA0`.
- `0x470F6F..0x470F8B` (stage-0 `MAGFILTER` = `0x10`):
  `8b45fc` (`mov eax, [ebp-4]`), `50` (`push eax` -> `[esp+12]` value),
  `6a10` (`push 0x10` -> `[esp+8]` state), `6a00` (`push 0` -> `[esp+4]` stage),
  `8b4df8 8b5138 8b45f8 8b4838 8b01`, `52` (`push edx` -> `[esp+0]` device),
  `ff90a0000000` (`0x470F86`: `call dword ptr [eax+0xa0]`).
- `0x470F8C..0x470FA8` (stage-0 `MINFILTER` = `0x11`):
  `8b4dfc` (`mov ecx, [ebp-4]`), `51` (`0x470F8F`: `push ecx` -> `[esp+12]` value),
  `6a11` (`push 0x11` -> `[esp+8]` state), `6a00` (`push 0` -> `[esp+4]` stage),
  `8b55f8 8b4238 8b4df8 8b5138 8b0a`, `50` (`push eax` -> `[esp+0]` device),
  `ff91a0000000` (`0x470FA3`: `call dword ptr [ecx+0xa0]`).
- `0x470FA9..0x470FAE`: `8be5 5d c20400` (`mov esp, ebp; pop ebp; ret 4`).

## Facilitator check (2026-10-20)

- **Independent PE disassembly checker (`build/tmp/a4ad-facilitator/verify_pe_disasm.py`):**
  verifies the authority SHA-256, maps VA `0x470F29` through the PE section table
  to file offset `0x70F29`, checks the 134-byte chunk SHA-256 (`ae58b1fa...`),
  and verifies every instruction boundary and byte sequence across `0x470F29..0x470FAE`.
  - **Facilitator narrowing on `predictions.md` line 27:** the worker's `predictions.md`
    informally wrote "`50` (value)" for the second push block before `0x470FA3`.
    The actual byte at `0x470F8F` is `51` (`push ecx`, after `8b4dfc mov ecx, [ebp-4]`),
    whereas `0x470FA2` is `50` (`push eax`, device). The worker's `round-note.md`
    correctly cites `8b4dfc 51`.
- **Independent `node:vm` checker (`build/tmp/a4ad-facilitator/verify_a4ad.cjs`):**
  1. Verifies `hooks_a4ab.js` (`52c5f966...`), `hooks_a4ac_candidate.js`
     (`9100d0d0...`) and `hooks_a4ad_candidate.js` (`1445dad5...`) SHA-256
     digests and confirms `snapshotActors` in `hooks_a4ad_candidate.js` is
     character-for-character identical to `hooks_a4ac_candidate.js`.
  2. Mounts `hooks_a4ac_candidate.js` and `hooks_a4ad_candidate.js` in an
     independent `node:vm` harness and verifies:
     - `hooks_a4ac_candidate.js` emits all 5 `HOOK_INSTALL` events before attach 0
       and records `retaddr: '0x50000'` (the device pointer) on `TARGET_TSS`.
     - `hooks_a4ad_candidate.js` emits `HOOK_INSTALL` strictly after each
       `Interceptor.attach` call (`[0, 1, 2, 3, 4]` prior installs at each attach),
       emits `HOOK_ATTACH_FAIL` when attach 3 (`0x470f29`) throws (leaving only 2
       `HOOK_INSTALL` records), records `retaddr: null`,
       `stack_convention: 'pre_call_stdcall_4'`, `device: '0x50000'`, `stage: 0`,
       `state: 0x10`/`0x11`, `value: 2` on `TARGET_TSS`, and preserves
       `retaddr: '0x47ab90'` on function-entry `TARGET_SETTER`.
  3. Runs **four isolated single-feature mutations** on `hooks_a4ad_candidate.js`
     against `node build/tmp/a4ad-hook-stack-announce-offline/test/run.mjs`,
     restoring and verifying the exact SHA-256 (`1445dad5...`) after each:
     - `M1_emit_install_before_attach` -> FAILS test suite;
     - `M2_revert_tss_retaddr_to_esp0` -> FAILS test suite;
     - `M3_shift_tss_stage_state_offsets` -> FAILS test suite;
     - `M4_revert_setter_entry_retaddr` -> FAILS test suite.
- **Suite and clean environment:** full build (`cmake --build --preset dev-debug`)
  and **20/20 CTest** pass. All files under `build/tmp/a4ad-hook-stack-announce-offline`,
  `build/tmp/a4ad-facilitator` and `build/tmp/a4ad-hook-stack-announce-offline-brief.md`
  are LF. Zero `Lemmings Revolution` processes and zero `LemWeb` scheduled tasks.

## Next bounded action

Before any future live zoom1 instrumentation attempt is even considered, first
exhaust static provenance of the unresolved pre-actor draw calls on the frame/tube
path (`0x437BA3` at `0x441879`, the two indirect tube subdraws at `0x43A9BB` and
`0x43AA47`, and the object-loop vtables dispatched before actors at `0x44190D`) to
determine whether any pre-actor pass in the same frame writes `[0x4C24E0]` or calls
`0x470F29`.
