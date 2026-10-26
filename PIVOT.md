# Pivot: diff the draw calls, not the pixels

Proposed by the overlord, 2026-10-25, at the user's request ("I'm worried that we're
overfitting"). **This is a proposal, not a directive.** It authorizes nothing.
`docs/DIRECTIVE.md` stays in force until the user approves a replacement.

## The question

Is diffing our frame against a screenshot of the original, then "fixing" the difference,
overfitting? Should we read how the original's renderer works and match that instead?

## Where we are

Static reading is already the primary method (`PLAN.md`, "Method change: read the code,
diff the pixels"). The screenshot is the oracle. What is wrong is how the reading is
organised, and that the screenshot is the only check on it.

**Overfitting has happened, mostly in the gates:**
- Gates have moved to fit results. A6 was restated once, A7 amended twice (caption, then
  the skill panel), and C1 amended three times (footprint, then gated views twice).
  Each change had a reason, but the pattern is "the gate moves when it fails."
- Some values came from the image, not the code. The B1 panels were "measured from the
  reference, not read," and B1f glyph fitting was retired ("do not revive the fitting
  method").
- The "do not choose by score" rule was the right guard. It kept C3al, C3an and C3ao,
  values read from the code that made the score worse.

**The bigger problem is that the reading is bottom-up and blind.** C3 has run about 40
rounds (C3a–C3ao). Each round reads one function, wires that value in and re-scores the
whole frame (latest 0.0635482 against the 0.02 gate). There is no top-down map of the
original's frame: every draw call, in order, with its geometry, states and texture.
So when a correct value makes the score worse, nobody can tell whether it is wrong or
something upstream is wrong (the leads are scene pose and mirroring; neither is
established). A full-frame pixel score can't locate an error. It only says the frame
is wrong.

The live arm has also stalled. C3 used 0 of 3 stock starts over two runs: every attempt
was rejected by its pre-launch checks.

## Precedent

Round 10 found that the screen-space tracker had been following something that wasn't a
lemming for nine rounds. The fix was to stop matching screen quads to lemmings and read
the game's own actor list at `0x004CBB18`. Since then every per-tick claim compares
against the actor list (`measure_steppers.py --compare`), not the image. Rendering needs
the same move: compare against the original's own draw stream, not its pixels.

## The plan

1. **Map the frame top-down (offline).** One document listing what the original draws
   in a level frame, in order: which code issues each draw, with what geometry source,
   render state, texture stage state and texture. Fill it from what C3a–C3ao and the
   A, B and C1 rounds have already read, and mark every gap UNREAD. Later value rounds
   fill gaps in this map instead of picking the next function on their own.
2. **Build a draw-call logger for the original (offline, dry-checked).** The game draws
   through `IDirect3DDevice3`, with dgVoodoo translating to D3D11. Log every draw at one
   of these boundaries:
   - a pass-through stand-in DLL in front of dgVoodoo that logs calls and forwards them,
     the same technique as `tools/dinput-shim`; or
   - RenderDoc capturing the D3D11 calls dgVoodoo emits.

   Per draw: primitive type, vertex format and data, render states, texture stage
   states, bound texture (dimensions, format, content hash), viewport and transforms if
   set. Dry-check before any start decides which boundary to use.
3. **Make our renderer write the same log.** Same fields, same order, from
   `src/app/` (TubeScene, SceneDispatch, SceneRaster, SkillPanel and the HUD).
4. **One start to capture the original.** Load the repair4 pause frame and log every
   draw. In the same start, do what the current directive asks: re-capture the repair4
   reference with the cursor clear of the hatch, the full actor list and the HUD
   counters, and read the pause values (`[game+0]`, `[game+0x5139]`, `0x58d498`, the
   effective texture format and binding). One good capture should also answer most
   open state questions (the C3 texture formats, C2 caption blend, C1 alpha format).
5. **Diff draw by draw.** Mirroring, the orange discs, the tube ends, draw order and
   blend state become yes/no differences between two logs, not pixel-error areas to
   guess about. Each fix is one brief with one draw-level acceptance check.
6. **The screenshot is the acceptance gate only.** The C3 gate (≤ 0.02 full frame, the
   floor ring included) stays as signed off. Report it after each landing. Don't amend
   it to fit a result. A gate change needs a draw-level reason.

## Evidence standard

A logged draw from the stock executable is an OBSERVATION of the call stream, labelled
OBSERVED, with the capture file and the logger build cited. It can disagree with a READ
value, in which case the round says which one it trusts and why. A disagreeing live read
replaces a DERIVED value, as `.claude/rules/match-the-original.md` already requires. The
logger must forward every call unchanged, and the round must show it did: a screenshot
taken through the logger matches one taken without it.

## Decisions the user must make

1. **Approve the capture method.** Draw-call logging is not on the current list of
   approved methods (normal `Popen`, menu attach through the REA CLI, read-only hooks,
   no debugger).
   - The stand-in DLL changes an install file. It would be hash-checked and restored,
     as other install files are.
   - RenderDoc changes no install file, but it injects into the process at launch.
2. **Replace the active C3 directive.** Approve a directive built from steps 1–6. Its
   budget would be about 6 hours or 16 rounds, the existing pool of 3 starts, 45-minute
   rounds, Sol tier. The same stop points apply, plus a stop if the logger can't be shown
   to forward calls unchanged.
3. **Scope.** C3 only, or apply the same method to the parked C2 caption blend and C1
   alpha format once the capture exists.

## What does not change

- The 0.02 metric, denominator, region partition and floor rule.
- "Do not choose by score": keep read values that worsen the screenshot until a draw log
  says otherwise.
- C1 is done. Its frozen `zoom1` / `elev_down` gates must keep passing.
- Every guardrail in `docs/AGENT_SAFETY.md` and the foreground and restoration rules in
  `.claude/rules/match-the-original.md`.
