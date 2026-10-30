# Rendering method: mechanism first, pixels last

Approved direction: user, overlord review 2026-10-25, following the discussion of
`PIVOT.md`. This refines that proposal; it does not approve its live methods or budget.
[PLAN.md](../PLAN.md) records milestones/gates. The former directives are historical;
enacter follows the user's current request. Existing evidence is reused, not discarded
or globally re-certified.

## Frame map before local fixes

Maintain one top-down map of a level frame, linking established evidence and production
consumers: asset/object identity, scene hierarchy, transforms/camera/projection,
visibility/clipping, submission order, texture/material state, depth/blending and final
composition. Map tube, scenery, actors and overlays, including uncertain ordering edges.
Mark READ, DERIVED, OBSERVED and unknown/conditional connections explicitly. A static path
is not proof that the runtime took it. Do not fill gaps by choosing what looks best.

Trace a defect to the earliest unsupported or disagreeing stage on its path. Pixel-error
regions still show impact; they cannot determine a transform, material or state value.
Prefer a shared mechanism correction with a predicted effect on multiple objects/views
over object-specific offsets or switches. Integrate and render small complete paths;
do not postpone all implementation until the entire renderer is decompiled.

## Comparable submissions, not necessarily identical API calls

The preferred observation boundary is the original's graphics submissions before wrapper
translation, if feasible. A D3D11 capture downstream of dgVoodoo observes the translator's
output; extra passes, conversions and batching may require an explicit mapping. Neither
RenderDoc nor a forwarding DLL is assumed compatible or authorized by this method alone.

Define the comparison record before interpreting a mismatch. Preserve raw provenance and
record primitive topology/indices, coordinate space and vertex attributes, texture identity
and content version, effective state at each submission, and relevant ordering. Track
resource creation/updates and state established before the selected frame where needed.
Unknown is not a device default. Include clears/viewport or other operations when they
affect the comparison. Cite how a submission is associated with an object; a guessed name
or matching screen position is not identity evidence.

Our software triangles need not match native call counts. Normalize only documented,
semantics-preserving differences, retaining order and raw records so normalization cannot
hide a defect. Numeric tolerances need precision evidence, not selection by pixel score.
Compare a bounded path first; full-frame coverage is a later claim with an explicit census.

## Capture trust and validation

- Synthetic forwarding/state/comparison tests prove tooling behavior only. Independently
  test missing/dropped records and discriminating single-field changes; preserve resource
  lifetimes, returns and inputs. A screenshot match alone does not prove faithful forwarding.
- A native capture records executable/wrapper/logger identities, method, frame/state,
  coverage, omissions and raw artifacts. Establish non-interference under matched state;
  disclose timing/behavior perturbation. Installation and live-method approval are separate.
- An observed mismatch prompts checking route, inputs, boundary and logger integrity before
  retracting a read. A valid disagreeing live read supersedes the corresponding conditional
  DERIVED value; a synthetic expectation or a different wrapper boundary does not.
- Keep source-supported fixes even when the aggregate image worsens. Demand a discriminating
  intermediate regression test where available, plus the unchanged screenshot report. Label
  self-consistency tests honestly; they cannot independently prove the native mechanism.

## Generalization and acceptance

Preselect diagnostic and validation fixtures before tuning: the current pause, different
camera views and another level. Holdout captures are validation, not parameter search;
record predictions before examining them and document if a failed holdout becomes diagnostic.
One frame establishes only its observed paths/state, not all renderer behavior.

Screenshots remain end-to-end acceptance gates and diagnostics. Existing C3 and C1 gates,
masks, thresholds and accepted scopes do not change. Additional submission and holdout checks
are evidence of mechanism/generalization, not automatic new milestone gates. Only the user
can amend acceptance; neither a passing image nor a passing draw comparison alone proves
complete rendering equivalence. A feasibility pass may legitimately move no pixel gate.
