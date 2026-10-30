# Essential lessons

The stock 640×480 `Lemmings Revolution.exe` is the reference. SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

This page is the short startup path, not a status report or a replacement for evidence.
Use [STATUS](STATUS.md) for current implemented/verified scope and open gates. The
[documentation map](README.md) explains canonical references and where to look next.
The [dated research excerpts](lessons/research-history.md) retain historical findings;
read the relevant topic/evidence on demand. Earlier conclusions can be superseded by
later entries. Historical procedures are not authorization to run them.

## Essential rules

1. **Match the stock executable, not classic Lemmings or plausibility.** A claim needs
   an address and bytes read there, or a reproducible script; decompiler prose alone
   is not a measurement.
2. **Keep evidence status and scope attached.** Distinguish READ, MEASURED, DERIVED,
   inference, hypothesis, and pixel fit. A bounded ROI pass is not full-scene acceptance.
3. **Keep negative results and corrections.** Do not silently discard failed gates,
   retractions, or later qualifications; follow the dated evidence links.
4. **One cell is one world unit.** Do not multiply world distances by `PIXELS_PER_UNIT`.
5. **Collision and rendering are different data paths.** The collision grid is stored;
   surface/render geometry is not a substitute collision mask.
6. **Respect coordinate conventions.** File-grid Y is up, sim terrain Y is down;
   native world-to-cell conversion truncates rather than floors.
7. **Identify the right runtime object.** Historical billboard-quad probes produced
   plausible but wrong actor measurements; use the actor list, not a convenient quad.
8. **Test one change at a time.** A mutation should independently break the behavior
   under test; a green test suite alone does not establish native fidelity.
9. **Do not generalize from one level, pose, camera, or process.** Preserve each
   finding's stated scope and unresolved conditions.
10. **Follow current safety and authorization instructions** before any live-game work.
    Old recipes, budgets, or historical approvals are not permission.
11. **Do not aim gameplay with frontend pointer feedback.** `drive.cursor` reads
    frontend globals; gameplay uses screen+16A0/16A8 doubles. Guard the mode before
    input; see the [failed and corrected loss capture](scratch/frontend-loss-retry-quit-2026-10-28.md).

## Find details by topic

- [Camera, tube, and sprite findings](lessons/camera-tube-sprites.md)
- [Actor rendering](lessons/actor-rendering.md) and [scene rendering](lessons/scene-rendering.md)
- [Frontend and navigation](lessons/frontend.md); [HUD and skill panel](lessons/hud-and-panel.md)
- [Data formats, simulation, and gadgets](lessons/data-and-simulation.md)
- [Methods and environment](lessons/methods-and-environment.md)
- [Historical research and retired workflow](lessons/research-history.md)

## The data formats — solved

See [data formats and simulation findings](lessons/data-and-simulation.md#the-data-formats--solved).
This historical heading is retained as a link target, not a claim of complete project acceptance.

## The simulation — measured, and worth keeping

**Retraction:** the old “5 Hz, measured three ways” claim is withdrawn. The later evidence
finds support against discrete 5 Hz stepping; 200 ms bookkeeping is not a confirmed native
step rate. Continuous-stepper implementation and recheck are not full simulation acceptance.
See [the full simulation findings](lessons/data-and-simulation.md#the-simulation--measured-and-worth-keeping)
and [D1m](scratch/phase-d1-stepper-clock.md), [D1n](scratch/phase-d1-continuous-stepper-clock.md),
and [D1o](scratch/phase-d1-continuous-live-recheck.md).
