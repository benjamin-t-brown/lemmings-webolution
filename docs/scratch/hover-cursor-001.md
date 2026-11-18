# Playtest 13 C7 hover cursors

**Needs review.** C7 reported lemming brightening and gold hatch brackets on
node 22, **One Way Ticket** (`MM_ALL_8_001`). Difficulty difficult, with
Opus recommended and the assigned Codex backend used. [Playtest ledger](../playtest/2026-11-17-playtest-13.md)

The white **E** means **entrance**. It is baked into the green bracket artwork
in `TEXTURES\_CURENT.BMP`. The hatch hit at `45525C` leads to state 9 at
`455378`, which binds the entrance sprite at `4562CD..4562E5`. The ordinary
lemming hit selects state 4 at `4553C5` and uses `TEXTURES\_CURLEM.BMP`,
which has green brackets and a yellow center dot. The screenshots show the
E under the hatch while the nearby lemming stands to its left. The original
also has a separate additive sprite redraw at `454ACA..454B5C`. The static
trace used the registered EXE SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
[Static packets, bitmap evidence and references](../../build/hover-cursor-001/evidence.md)

`runApplicationLeg` now passes the chosen hatch or actor from the existing
hover picker to `runtimeCursorState` and draws one pointer-centered cursor
through `RuntimeCursor::draw`. The picked actor supplies `markedActor` and
the same picker still supplies the click target. The cursor replaces
`TubeScene::drawHoveredWalker`'s half-strength redraw. Hatch hover calls
`drawHatchHover` for the white number above the mesh. The gold mesh brackets
are gone. The entrance cursor supplies the brackets at the pointer.

`454B95` reads the original hatch's release rate. The clone displays the
requested unreleased population through `World::remainingFromEntrance`.
One Way Ticket starts at 50 and the count falls with each release. Holding
a hatch button still shows the effective rate while adjusting it. The clone
uses the existing `^Font` and mesh-center anchor. Exact native font sampling
and texture filtering remain unknown. The 28×28 entrance bitmap uses
provisional nearest-neighbor expansion to the instruction-backed 32×32 quad.

The focused build and five existing checks passed after correcting the
initial 32×32 source bitmap assumption. A one-frame clone diagnostic on One
Way Ticket showed a white 50 and matched all 290 opaque entrance cursor
pixels. Tests used private LocalAppData. [Build](../../build/hover-cursor-001/build-2.log),
[focused checks](../../build/hover-cursor-001/focused-2.log),
[initial load failure](../../build/hover-cursor-001/focused.log),
[clone frame](../../build/hover-cursor-001/one-way-ticket-hatch.png),
[pixel measurements](../../build/hover-cursor-001/asset-pixels.log)

All **20 promoted replays** returned exit 0, `PASS`, `won=1` and
`completed=1`. One Way Ticket saved 46/40 at step 13812. The replay ledger
gives every recording's name, final step, count and log. [Replay results](../../build/hover-cursor-001/replays.tsv)

The smoke fixture formerly expected the normal crosshair and the old hatch
overlay. It now expects the entrance cursor and white count, with the same
full-frame assertion. Removed legacy `levelmenuwindow` registration because
its Retry fixture paints the hover cursor into the base for the next menu
frame, where the new E survives beneath the menu crosshair.

Handoff **needs_review** for Playtest 13 C7.\
Next action is the supervisor smoke gate, then independent review and human playtest.

**What to check.** Open One Way Ticket and hover the hatch, then move to a
separate walker with Climber selected. The hatch shows green E brackets and
a single white count that decreases after release. The walker shows green
brackets with a yellow dot. Clicking gives that walker one Climber.
