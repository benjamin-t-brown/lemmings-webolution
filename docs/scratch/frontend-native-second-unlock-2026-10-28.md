# Natural win and second map unlock state — 2026-10-28

**Bounded B3 PASS:** fresh and naturally completed level-select/map frames pass
the unchanged full640×480 gate; normal map selection reaches `MM_CLIMB_002`.
This is not complete frontend, physical-input, gameplay-rendering or timing
acceptance. [B2's abandonment/footer route](frontend-native-return-complete-2026-10-28.md)
and its [earlier failures](frontend-native-return-2026-10-28.md) remain separate.

## Native observation

Packet: `build/tmp/enacter-native-second-unlock-20261028/`. Prediction and
authorization were written before launch. One owned stock process, PID9712,
through ordinary New Game and first map node; no D3 level-table swap, timing
watcher, actor/grid/progress/timer writes, pause or fast-forward.

The retained `assign_skill.assign` helper used guarded shim panel/hover/click
input. Ten distinct actors received Climber, each checked for attribute0→1 and
one stock decrement, total10→0. Native result: saved10, required1. This is a
natural solution, not a completion-flag fixture or a timing-fidelity replay.

Observed counter/page/UI sequence:

| Page | Counter | UI mode | Page vtable |
| --- | ---: | ---: | --- |
| Menu | 3 | 3 | 4B2528 |
| Confirm | 4 | 3 | 4B3F38 |
| Fresh map | 5 | 3 | 4B3EB8 |
| First level | 5 | 0 | frontend list empty |
| Win results | 7 | 4 | 4B1C30 |
| Continue → completed map | 8 | **4** | 4B3EB8 |

All102 native completion DWORDs use `4E36E8 + 1B58*group + 118*slot`
(hex strides; decimal7000/280). Fresh flags are all0; win and completed map
flags are `[1] + [0]*101`. Completed map states are `[2,3,3,3,3] + [1]*97`:
node0 complete, nodes3/4 newly unlocked, no unrelated progress change.
Raw map records, flag bytes, address list, landing samples and bracketed
page/cursor snapshots are retained. The initial structural menu focus is row0.

All seven native images were visually inspected: menu, confirm, map, right-held
map, level entry, win results and completed map. They show the declared pages,
normal versus held center cursor, result counts and changed map lamps. The
level-entry image is retained context, **not** a matched gameplay-frame score.

## Source comparison

`test_levelreturn` runs the actual SDL-dummy application through natural10/10
completion, then Continue. It asserts counters7/8 and the complete102-element
flag/state arrays; real SDL motion and button-up settle the pointer to320,240
before exporting composed frames. No synthetic outcome or progress mutation is
used for these two images. Later natural loss/retry/Quit/reentry checks remain.
The source test's2-second frame step is acceleration, not native clock evidence.

Six comparisons use `tools/harness/compare_camera_frame.py --roi 0 0 640 480`:
normalized RGB absolute difference, no exclusions, resizing or registration.

| Frame | Error | Source origin |
| --- | ---: | --- |
| Menu | 0.00495742698120915 | Production `FrontendRenderDump` |
| Confirm | 0.0022420002723311547 | Production dump |
| Fresh map | 0.007834584354575163 | Production dump |
| Right-held fresh map | 0.007842690461601307 | Explicit held-cursor production fixture |
| Win results | 0.00282856328295207 | Actual SDL natural-completion export |
| Completed map | 0.00783466520288671 | Actual SDL Continue export |

All pass0.02. `*-score.json` retain image hashes and sums; native, candidate and
diff PNGs remain in the packet. Final rebuilt dumps and natural exports are
byte-identical to the scored candidates (`check-final-source.sh` / log).
The map page is the level-select page (selector23, ctor49AA90, background named
`level select background.bmp`); no distinct intermediate selection screen was
observed or established by the retained code. B3's two-state image/first-node
requirements are met in this bounded sense; broader hover/input is not inferred.

## Dispatch-mode correction and regression

Native UI mode is independent of displayed page identity. `45DF33` writes UI4;
Continue `486090/48609E → 4886E4 → 48726F` replaces the page, without resetting
it. UI3 writer `45D743` is gated by Quit. The actual returned Map8/UI4 confirms
this static distinction. Byte checker/findings: `build/tmp/enacter-win-map-ui/`.

`shell::Frontend` previously returned UI3 for every ordinary frontend page.
It now retains dispatch mode across page-only replacement. Full initialization
models the caller's mode write; level entry/retry, pause and menu dismissal
write their respective modes. `test_results` distinguishes Continue-retained
UI4/UI5 from Quit-reset UI3, including footer/menu/confirm page replacements
and level reentry. Loss-mode retention is READ/source-tested, not newly observed.
This accessor is diagnostic/test state, not a new render or input dispatch path.

Independent mutations each fail `results`: force Continue toUI3; retainUI5
through loss Quit. Both were removed and the three implementation/test files
compared byte-for-byte to their saved baseline. Final UCRT64 build and retail
CTest: **70/70 PASS,229.45s** (`final-full-suite.log`); no skipped-asset claim.
Packet verifier, structural menu replay, byte checker and final export equality
also pass. These checks do not replace the native observations above.

## Restoration and remaining scope

Window18:54:51Z–19:09:40Z; capture finished and owned process closed before
independent verification at **18:58:10.730484Z**. Save, config, exe and shim hashes
match their originals; no stock-like process remained. Desktop was released.
Executed scripts/manifests, backups and receipts are immutable spent evidence,
not permission for another launch. No restoration debt, commit or push.

B2's nonzero-highlight Return quirk and remaining return branches still need
bounded native comparison. Exact cursor colors, physical input/focus equivalence,
frontend/gameplay pointer handoff, gameplay-held cursor, result focus/movie
behavior and full C2/C3 remain open. D1/D3/digger timing stays user-parked;
browser work remains last. Correct10/10 saved does not close D3's end-time gate.
