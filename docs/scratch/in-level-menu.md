# Connected in-level Escape menu (2026-10-28)

Offline READ/DERIVED implementation of the bounded native menu. Packet:
`build/tmp/enacter-level-menu/findings.md`, raw assembly, assets and `verify.py`.
Main independently reran the checker. No original process/desktop, registry or
installed-save access was used. SOURCE tests are not native input/pixel acceptance.
Standalone `tools/harness/read_level_menu.py` retains31 raw instruction anchors
and the menu table against the stock SHA256, without scratch imports.

## Recovered behavior

Unpaused cancel/Escape opens UI2 through45D40A/selector1F. Loaded game mode2
remains; suspension sends dt0 through45DD63 instead of normal gameplay time.
Scene and drawable entry cue remain visible, while camera/cue clocks freeze.
This is not evidence every incidental native field is immutable.

48F51B builds table4C4228={44,48,43,3,0}, yielding UK **Resume / Try Again /
Quit / Options** at Y100/129/158/187. No background is bound and no dim overlay
was found in this path. First row Resume receives initial programmatic focus.
The fifth label is empty, with inverted panel Y geometry; this adapter does not
invent a label, a normal blank panel or keyboard access for that artifact.

Actions are distinct:

- Resume48F5D2 or menu cancel48F599 sets the request consumed at45D80C: **playing**,
  even if the menu was entered from pause.
- O/options-toggle while UI2 tears down the menu and writes **paused UI1** at
  45D575. This is not Resume. While paused, O opens the menu; paused Escape/P
  retain their resume behavior.
- Try Again48F5E6 sets reload and intro preference0; main consumes it, releases
  the old level and loads a fresh one playing. The cue still starts1.5.
- Quit48F601 requests **Map23**, not results, main menu or process exit. It
  preserves progress and, unlike loss-results Quit, does **not** write the intro
  preference. Future map activation enables intro independently.
- Options goes to sound settings9; the empty row goes to control settings19.
  Their full value/input/persistence routes are outside this connection. Options
  is visibly rendered but explicitly consumed without opening a settings page.

Native hits use strict **content** bounds, not panel padding. For example Resume
accepts281<x<360,100<y<129; its painted border is(262,101)-(379,128). The other
nonempty rows reuse exact glyph widths and29px spacing. Existing panel/font
compositors supply SOURCE RGB composition, not a newly certified native image.

## Integration and ownership

`LevelMenu` owns four localized labels, strict hits and focus. It draws over the
composed level/HUD/cue, before the pointer. Opening resets focus to Resume;
explicit pointer motion over content updates it. Misses preserve focus as an
explicit SOURCE policy. No four-row keyboard traversal is claimed native.

Natural `--frontend` play now opens this menu on unpaused Escape instead of
closing. Standalone diagnostic Escape-close remains unchanged. Window close
and frame cap still exit without results. SDL mappings are explicit adapters,
not new physical DirectInput observations. All menu events, including misses,
unsupported actions, repeats and gameplay keys, are consumed before assignment,
skill-panel selection or camera input. A menu dismissal consumes remaining
gameplay events in that SDL batch; close/focus events still work. This prevents
queued menu input from leaking into a newly resumed level. No native event-batch
latency claim is made.

The runtime pause gate freezes World and pending completion; a separate menu
flag prevents painting the pause caption in UI2. O-close leaves pause set;
Resume clears it. Transition clears pending camera deltas, relative mouse grab
and camera-clock backlog, and resets the real-frame baseline. The scene remains
rendered throughout. Menu cursor uses focused NoLemming, not actor-hover or Grab;
native frontend MouseDown artwork/predicate is still unsupported.

A typed `LegDisposition` separates Completed, RetryLevel, ReturnToMap and Exit.
Only Completed publishes `LevelOutcome`. Retry destroys local World, timers,
pending-end state and UI, then reloads through the ordinary level path with
retained selected ownership and intro disabled. Abandon clears selected level/
node/outcome/results and returns the prepared map without rebuilding progress or
fabricating a loss. The outer loop no longer treats every no-result leg as exit.

## Regression scope

- `levelmenu`: literal labels/strict hit boundaries/focus, no fifth normal row,
  no background replacement, closed no-ops and SOURCE composition checks.
- `results`: guarded retry/abandon session actions, preserved completed-node map
  pixels and flags, unchanged intro preference on abandonment, stale-result
  rejection and fresh different-node completion ownership.
- `levelmenuwindow`: actual SDL dummy app opens Escape menu, consumes gameplay
  bindings and unsupported clicks, freezes time, resumes via focused Return,
  enters from pause, distinguishes O-close, retries a fresh first level, returns
  to map without any result, selects another root level, retries it and abandons
  again. Menu RGB samples and a whole-frame retry composition check verify that
  scene/cue remain beneath the menu without a backdrop or pause caption.
- `levelreturn`: the real timeout Retry flow now briefly opens the menu during
  Pending and checks the latched countdown freezes before resuming to results.

Read-only review found a queued-input fall-through risk after menu dismissal;
the event-batch ownership guard and simultaneous-dismissal/P regression were
added before final validation. Native menu pixels, blank-row raster/focus,
pressed pointer, shared transition latency/fades and settings remain unqualified.

Targeted baselines pass: `levelmenu`0.15s, `results`1.01s, strengthened actual-app
`levelmenuwindow`6.39s and pending-menu `levelreturn`105.23s. Test-only tick-counter
signedness warnings were corrected before final regression.

Eight isolated regressions fail: old unpaused Escape-close; missing suspension;
O incorrectly resumes; queued dismissal input leaks to gameplay; old no-result
leg exits the app; omitted menu draw; abandonment erases progress; padded rather
than strict-content hits. All production hashes match `pre-mutation.sha256`
before the final full build/CTest. The packet retains each mutation log.

Restored full build/CTest: **70/70 PASS**,216.50seconds; `levelmenu`0.15s,
`levelmenuwindow`5.89s and pending-menu `levelreturn`103.63s. Log:
`build/tmp/enacter-level-menu/full-suite.log`. No native pixel/input/timing
acceptance is implied by these SOURCE passes.
