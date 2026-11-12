# Gameplay commands: consolidated static contract

Revision 5, direct recovery, 2026-11-02 UTC. Status: **ready for independent review, not accepted**.
This document replaces revisions 1-4; their text is archived under
`build/gameplay-commands-research-001/direct-recovery/revision4-document.md`.
The current report is `build/gameplay-commands-research-001/report.md`.

Reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA/Ghidra 12.1.4, analysis profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
No original execution. The analysis session was closed without snapshots.
INSTR means inspected instructions/bytes; DECOMP means pseudocode; DERIVED means an inference from cited evidence.
Static contracts and source tests are not runtime fidelity acceptance.

## Route and current failure

Milestone: play/finish an installed level. Route:
`webolution --level MM_CLIMB_002 --sim` -> `src/app/main.cpp` ->
`RuntimeSkillSelection::consumePanelClick` -> `RuntimeSimState::advance` -> `World::tick`.
Source inspection: panel indices 8/9 consume clicks but perform no command; selection only accepts 0..7.
The level decoder retains eleven tail words, but production setup projects only the eight actor skills.
`World::nuke()` is not connected to this input path and applies a bomber fuse to every active actor at once.
It is not evidence of original Armageddon behavior.

## Evidence and review closure

Full direct-recovery Evidence envelopes, including exact addresses, complete assembly strings, provider limits and IDs:
`build/gameplay-commands-research-001/direct-recovery/evidence.json`.
Readable assembly and the vtable map are beside it. Earlier instruction evidence remains at
`build/gameplay-commands-research-001/instruction_evidence.md`; the previous report is archived beside the old document.
The installed read-only census remains `stock_census.txt`.

| Latest review gap | Direct recovery evidence | Finding |
| --- | --- | --- |
| Trigger/initial timestamp | Full 0x4839A0 ctor, 0x483B5D poll, 0x455E10; dispatcher 0x454725-0x4547C5 within full 0x4532E2 | Initial timestamp zero; release-pair formula, selected-stock guard and dispatch now instruction backed. |
| Selection/consumption | Full 0x45AC5C and 0x45B594 | Selection writes even at zero stock; consume decrements selected stock. |
| Update prologue/tail | Full 0x4423D2, specifically 0x4423FE-0x442414 and 0x442B00-0x4431C2; caller 0x45DA6A-0x45DD76 | Loaded gate, zero-dt paused call, normal entity removal, results call established. |
| List topology | Full 0x4466B0, 0x426180, 0x4262C0, 0x426360, 0x4090F0, 0x409110, 0x446790, 0x4467B0, 0x408FC0 | Two sentinels, newest-first normal walk and oldest-first command walk; neighbor links updated on removal. |
| State return 2 | Vtable 0x4B10E8 +0x14 -> 0x42E2BF; full update and 0x442C1C onward | Return 2 becomes false entity update; node leaves active list, without saved increment. |
| Detach hooks | 31 direct base-ctor callers, their vtable bytes, 11 unique slot +0xC targets (full assembly) | Includes real cleanup, not just no-ops; table below. |
| Contradictory revisions | This consolidated document and current report | No superseded contract embedded as active guidance. Timing choice separated from source evidence. |

## Fast-forward: independently implementable

Retained review already passed the fast-forward criterion. Evidence: earlier instruction_evidence.md and newly retained full
0x45CFF9/0x4532E2/0x455AFF, plus the mouse poll.

- **Set** (INSTR 0x455AFF-0x455B60): hover index 8, mouse button-0 currently down, writes input+0x20=1.
  Holding the button and moving onto the plate also sets it. No stock read/decrement; index 8 is not selected.
- **Latch/reset**: constructor 0x452BC2 and reset 0x455774 clear +0x20. The live plate path sets but does not toggle/clear it.
  Reset occurs on load/restart/result transitions, not pause/unpause (retained caller analysis).
  Replay event 5 can assign a value. The live-latch conclusion is supported by reviewed instruction and caller evidence;
  it is not an exhaustive proof about every possible aliased pointer write in the executable.
- **Input and pause** (INSTR 0x45DA6A-0x45DB0E): ordinary input dispatch is outside the pass loop and still runs paused.
  Positive-dt passes require pause byte game+0x55A0=0 and game state=2.
  Otherwise the caller invokes 0x4423D2 once with zero dt. Do not describe that as skipping the update entirely.
- **Budget** (INSTR 0x45DB0E-0x45DD5C): input+0x20 selects 7 passes (dword at 0x4B15F8) versus 1.
  Every pass reuses the full frame delta. Event boundaries split a pass into multiple update calls, followed by its remainder.
  Input is not repeated seven times. This is a simulation pass budget, not measured wall-clock speedup.
- **Delta** (INSTR 0x45D927-0x45DA3C): milliseconds converted to seconds; 0.1*1000 cap=100 ms.
  Flag game+0x1C9B3 forces 100 ms even for shorter frames; it is zeroed at startup. The __ftol operand is the cap expression.
  The original continuous-dt/event slicing does not establish a 200 ms tick.
- **Authorized integration**: repeat the existing elapsed-time advance up to seven times when latched, preserve pause,
  completion stop and reload/reset handling. Do not recalibrate SimClock, introduce the original clamp into normal-speed
  timing, change hatch/digger rates or reopen D1/D3. Existing-clock quantization remains a stated limitation.
  Repeating simulation may repeat its sound events; a separate audio redesign is outside this task.

Residuals outside this bounded live command: event-5 recording emitter, all aliased +0x20 writers, meaning of the
force-100-ms flag, full audio scheduling. Existing xref/caller searches did not establish these; no runtime claims follow.

## Armageddon input, stock and request

All facts in this section are INSTR except the explicitly measured resource census and stated inference.

1. Mouse ctor 0x4839A0 initializes previous buttons +8/+0xC, press edges +0x10/+0x11, last release
   +0x14/+0x18 and pulses +0x1C/+0x1D to zero. Current buttons are filled by successful polling.
2. Poll 0x483B5D reads DirectInput button bytes 0/1 (0x80 bit). On its normal successful path it clears pulses,
   derives press edges, then on button-0 release (current=0, previous=1) performs a signed 32-bit
   `now - lastRelease < 500` comparison. True sets pulse+0x1C=1 and lastRelease=0; false stores now.
   Right button has the analogous fields. Previous states are updated at the end.
   **Not an unconditional two-click requirement**: with lastRelease initially zero, a first release with timestamp
   less than 500 satisfies the same formula. No first-release sentinel test exists. Caller timestamp startup origin
   was not proved; do not claim that case can or cannot occur in normal startup.
   Error exits can precede pulse clearing/previous-state update; the one-poll statement applies to successful polls.
3. 0x455E10 reads mouse+0x1C directly. At 0x454725-0x45476A, hover=9 and that pulse lead to a signed
   `stock[selected] > 0` guard, 0x45B594, then 0x444A98. Preserve the selected index guard; do not substitute
   hovered stock. In ordinary mode-2 interaction a preceding press selects 9 through 0x455AFF.
4. 0x45AC5C stores the selected index even when its stock is zero. It enables +0x2750 when stock is nonzero
   and drives plate-9 confirmation presentation. A zero-stock select is not silently rejected.
   0x45B594 has no internal guard: it decrements selected stock, redraws counts except index 9, and clears +0x2750
   when new stock is zero.
5. 0x444A98 only changes request byte game+0x5138 from zero to one. Stock consumption is outside this helper:
   repeated direct calls are idempotent as a flag operation, not a complete stock-safe command.
6. Stock object is game+0x27D0 (0x4C91D8); selected is +0x28 (0x4C9200).
   Serialized eleven-word tail loads stock[0..9], then word10 to +0x2C (retained 0x45B86D evidence).
   MEASURED read-only census: all 102 effective installed levels have stock[9]=1, stock[8]=0, word10=8.
   MM_CLIMB_002: `10 0 0 0 0 0 0 0 0 1 8`. Preserve the resource value rather than hardcoding availability.

## Armageddon consumer and pause

INSTR: full 0x4423D2, attach functions, overlay, hatch and result functions retained in evidence.

- Entry 0x4423FE-0x442414: if loaded byte game+0x5139=0, set game state=1 and return.
  Otherwise save dt at +0x5120, update scene/effects, and reach the request block. There is **no additional pause gate**.
- While request+0x5138 is set: when short +0x5308=0, create a five-second countdown overlay if +0x539C=null and
  +0x50D3=0; initialize +0x53A0=0, do a zero-dt presentation update. Re-read reverse-begin into cursor+0x5304
  on every call while nothing has been attached.
- If 0x445436 counts at least one active-list entry whose type +0x4C is not 3, call 0x444A34 and zero
  remaining-to-release +0x5098. 0x444A34 uses 0x425D6D/0x425D89 to zero positive remaining hatch counts +0x98.
  This is **conditional**, not an unconditional action inside the request helper. With no non-type-3 entry yet,
  request alone does not stop hatches immediately. Hatch update 0x425A95 runs later and can release an entry.
- Overlay 0x413F15 subtracts dt from its five-second timer; completion means timer<=0, subject to floating arithmetic.
  It uses a local dt copy. Completion detaches/deletes the overlay and sets +0x53A0/+0x50D3.
- If overlay-complete, cursor is not reverse-end, and double credit+0x5128<0:
  attach one fresh state from ctor0x416910 to the cursor's entity, delete its old primary state if replaced,
  advance cursor through node+8, increment short+0x5308, add 0.05 seconds to credit.
  Then subtract dt from credit on every request update, including countdown calls.
  It is **at most one attachment per update call**, not a catch-up loop or a fixed 50 ms schedule.
  Event slices and fast-forward passes can give several calls per rendered frame.
- DERIVED pause behavior: 0x45DD63-0x45DD76 still calls this update with dt=0 while paused/not playing.
  The loaded gate still applies. Countdown cannot consume time, but release-stop/setup can run.
  Once countdown is complete, negative credit can permit one attachment per zero-dt call until credit is nonnegative
  or the cursor reaches end. Each such call adds 0.05 with no dt subtraction. Do not claim all command behavior freezes.
- After the command block, normal hatch/entity/effect updates continue. 0x4431AE calls results 0x445498 before
  the normal epilogue. The prologue/tail are no longer unread dependencies.

## Entity order, replacement and removal

The list at game+0x1EC8 has **two sentinel nodes**, not one circular sentinel:
ctor0x4466B0 links start+4 to end=start+0xC, end+8 (list+0x14) to start.
0x4262C0/0x426360 insert a 12-byte node after start:
node.payload=*arg, node+4=old first, node+8=start; old first+8=new; start+4=new.
Normal begin0x4090F0 reads list+4 and normal end0x409110 compares list+0xC.
Reverse begin0x446790 reads list+0x14; reverse end0x4467B0 compares list itself.
Thus, for hatch entries inserted through 0x426180, Armageddon walks **oldest insertion first** via +8;
ordinary updates walk newest first via +4. This is not ID sorting.

0x42E696 calls new-state attach slot+8, then checks new-state byte+0x11 through0x430A50.
Ctor0x416910 makes +0x11=0, kind+0x14=0x20, vtable0x4B0A58: it always takes primary replacement,
calls old-state detach slot+0xC, and stores new primary at lemming+0x340.
No old-state eligibility veto exists in this path. The caller destroys the replaced old state.
Attach0x41695C skips its animation call only for type3; it still installs the state.
Do not conflate lemming type+0x4C with behavior state.

Update0x41699E advances animation, applies its terrain/fall path, zeroes the dt argument, and when animation
finishes installs terminal state0x4081B0 (vtable0x4B0550). Terminal update0x408227 creates effects and returns2.
Entity update0x42E2BF at0x42E40F-0x42E417 translates state-return2 to false.
Game iteration0x442C1C onward caches node+4 before the entity update, then false causes removal registration
and0x408FC0. The unlink writes prev.next=next and next.prev=prev, then destroys the node.
There is no saved-count increment on this removal path.

Cursor lifetime limitation is now explicit: the unlink routine does not repair game+0x5304.
A retained disassembly search for +0x5304 found only initialization, command read/advance, and reset.
No guarantee that an unvisited cursor node survives other entity updates is established.
Reproduce the evidenced ordering with stable identities in a future design; do not intentionally copy a dangling-pointer hazard
or invent an original skip/death policy. Interleaved removal before visitation needs a separate stated adapter policy and review.

### Detach coverage

REA reports 31 direct callers of base-state ctor0x418390. Each caller's derived vtable was extracted from pseudocode,
then its six slots read as bytes; all 11 distinct detach targets were instruction-read.
This covers those observed direct ctor families; it is not a claim of exhaustive discovery of every indirect/subclass construction.
The machine-readable ctor/vtable/detach map is in evidence.json.

| Detach target | INSTR behavior |
| --- | --- |
| 0x401100,0x401320,0x4137BD,0x41A5E5,0x41A6C8 | No-op RET4. |
| 0x40E0DF (vtable0x4B07B8) | Clears lemming+0x128 state via0x4683E9(0,0); derives terrain coordinates and clears bit0x800 along the inspected vertical span (AND0xF7FF). Must preserve terrain cleanup when replacing this state. |
| 0x40EA98 (0x4B07D8) | Clears lemming byte+0x60. |
| 0x40EE9A (0x4B0810),0x4168B4 (0x4B0A38) | If state-kind0x2D exists via0x42ECC9, remove it via0x42E7F3. That helper invokes its detach, destroys it, and removes primary/queued membership. |
| 0x413EC8 (0x4B0990) | If owned +0x24 handle exists, calls0x4145B0 on manager0x4E3398, then zeros handle. |
| 0x417DD7 (0x4B0AF8) | Subtracts integer lemming+0x44 from double+0x10 and integer+0x40 from double+8. Positional cleanup must not be omitted. |

These hooks are side effects of replacement, not permission checks. Their called helpers retain their existing native behavior;
full independent reconstruction of terrain/effect helpers is outside this command prerequisite.

## Results and reset

INSTR 0x445436 counts active-list entities with type!=3. 0x41818A increments per-type saved and total
game+0x509C on exit; exploded entities do not count as saved.
0x445498 finishes on remaining seconds+0x50A4==0 or counted-active + remaining-release+0x5098==0.
It sets success from signed saved>=needed(+0x50A8), then advances the existing end delay:
timer+0x5130 starts7.0; below3.0 begins effect; below1.0 enters result state3/4 and teardown.
Armageddon does not replace this pass/fail predicate.
0x4458B0 decrements the seconds counter; no direct callers were resolved by REA, so its indirect scheduling is not newly proved.

Reset0x444BC2 zeroes request, loaded, countdown flags/pointer, credit, cursor, attachment count and saved/release counters;
sets result timer7.0 and game state1. +0x53A0 is initialized at overlay creation before use in the first request.
Retained loader analysis (DECOMP0x4432C1/0x4413E7) ties this to level load/restart and clearing the loaded gate on teardown.
The independent review should preserve that evidence-strength distinction.

## Exact parked dependency and next tasks

**No new timing modification is authorized.** Inspection is authorized; D1/D3/base-clock/hatch/digger and all other parks remain.
The source consumer currently advances only through 200 ms World ticks; paused RuntimeSimState::advance returns immediately.
Original Armageddon uses continuous dt, event-sliced call count, and zero-dt paused updates, so simply calling World::nuke
or one attachment per 200 ms tick is not a faithful integration.

Two decisions must not be merged:
1. Evidence: the formerly unread trigger/prologue/removal/direct-family detach dependencies are now traced. Cursor-invalidated
   behavior, full type3 content coverage, and full original timer scheduling are not declared proven.
2. Product/timing policy: either keep the timed consumer parked (default), or explicitly authorize a coarse existing-clock
   approximation with named deviations. Exact original slicing would require revisiting the parked clock/integration boundary.
   This pass chooses neither approximation nor a parked fix. Trigger scaffolding alone must not be advertised as working Armageddon.

After independent research acceptance:
- **fastforward-impl-001 first**: connect plate8 in the actual main/HUD/runtime path for MM_CLIMB_002; preserve existing normal timing.
  Planned public-input demonstration: before plate8 does nothing; after press/hold-over latches extra existing advances;
  release does not unlatch; pause preserves latch but prevents elapsed advancement; resume applies it;
  restart/load/result clears it. Negative: no stock consumption, no actor assignment through the plate, no changed normal-speed cadence.
  Test frame budgeting and completion boundaries without presenting injected outcomes as whole-game fidelity.
- **Armageddon remains parked for timed production integration**. A future proposal must select a timing policy, map explode/terminal
  states and detach cleanup to actual World behavior, specify stable-identity handling for interleaved removals, and correct the existing
  HUD/end adapter's remaining-release calculation after hatch cancellation.
  Planned MM_CLIMB_002 public-input cases: stock1 then0, zero-stock selection/rejection, release-pulse timing boundary,
  selected-vs-hover guard, pre-first-hatch request, oldest-first visitation, already-exploding/cleanup states,
  paused countdown and post-countdown credit, correct saved/needed result and restart reset.
  These are planned checks, not performed or passed.

## Validation and limits

This recovery changes reports/evidence and queues independent review, not production code, controls, models, tests or timeouts.
Prior recorded verification: main103/103 and recovery84/84 passed. Those are retained prior results, not a new run of this revision.
The controller must verify current content before independent review; no fingerprint or acceptance is forged.
The read-only census source/CTest registration is retained unchanged.
See current report for exact artifact paths and actual recovery commands. All earlier failures and retry history remain retained.

