# Frontend menu hover

**needs_review · Playtest 14 D6.** On the startup main menu, SDL motion updated the pointer coordinates while the previously selected row stayed highlighted. This happens before level selection. The same frontend serves the installed Swings Both Ways route (node 23, `MM_E1_5_SKILLS`). [Human playtest](../playtest/2026-11-18-playtest-14.md)

## Original pointer behavior

Static Ghidra inspection of the registered x86 `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. At `0x48F9BD..0x48F9E8` the manager reads pointer Y/X, picks a widget and focuses a hit before it tests the buttons at `0x48F9F0`. A miss jumps past the focus call. The picker at `0x489447` subtracts the widget anchor. The comparisons at `0x48686F/7A/8A/9B` require the pointer strictly inside all four content edges. `0x48F7A1` checks both eligibility bits and page approval before replacing manager+0x10. [Pointer polling](../../build/menu-hover-001/asm-poll.json) · [Hit test](../../build/menu-hover-001/asm-486860.json) · [Focus setter](../../build/menu-hover-001/asm-focus.json)

The traced pages approve focus through vtable slot +0x24 (`0x4860E0`). The label's gain/loss callbacks at `0x489DC8/0x489DDE` refresh its panel. `0x489ADC` selects RGB (224,128,0) for focus and (224,10,0) otherwise. **Hover is silent on this path.** Directional navigation separately calls the sound wrapper `0x487B34` in `0x48896A`. The evidence is static. Ghidra was closed and the original game was never run. [Decoded callbacks](../../build/menu-hover-001/callbacks.json) · [Panel instructions](../../build/menu-hover-001/asm-489adc.json) · [Directional branch](../../build/menu-hover-001/asm-48896a.json)

| Screen | Original controls and clone behavior |
| --- | --- |
| Main, selector 7 | `0x48ECDF` builds all five labels from `0x4C4214`. All five take hover focus. The third, Language Selection, has an original activation route to selector 0x0C. Its clone action is still unsupported. |
| New Game confirmation | `0x49BB6B` builds Yes and No plus a separate question. Both choice panels highlight. |
| Win / loss results | `0x485E2B/0x486335` build the rows and clear the first widget's draw bit. Win exposes Continue. Loss exposes Try Again, Continue and Quit. The clone starts with no visible row highlighted. |
| In-level Escape | `0x48F51B` builds Resume, Try Again, Quit, Options and a zero-width empty label. The four visible rows already use strict content bounds for hover and clicks. |
| Sound options, either caller | `0x48E5B4` creates Music, SFX and Accept. The volume arrows are separate widgets with texture-sized hit boxes and empty focus callbacks. Mapping either arrow to its owning row's highlight is the existing provisional clone choice. The meters have no clone action. |

[Builder cross-references](../../build/menu-hover-001/row-builders.json) · [Page decompilation](../../build/menu-hover-001/pages.json) · [Installed UK labels](../../build/menu-hover-001/labels.txt) · [Arrow focus](../../build/menu-hover-001/focus-effects.json) · [In-level geometry](in-level-menu.md)

## Clone change and checks

`runFrontendWindow → FrontendSession::hoverMenu → menuPanelAt → Frontend::setHighlight` handles focused SDL motion. Main, confirmation and results use the existing half-open click panels, including their padding. This deliberately preserves the clone's click targets. The renderers read the same focus, and `ResultsScene::image` refreshes its cached frame when focus changes. Off-row motion preserves the selection. Responding to motion lets Up/Down keep focus while the pointer is still. The original polls the pointer in its update loop. Its stationary-pointer and keyboard precedence remain unverified. [Task-only source diff](../../build/menu-hover-001/implementation.diff)

The registered UCRT64 build passed. Three focused tests passed with private LocalAppData. The existing `frontendwindow` smoke assertion sent Up from row 0, then motion to (320,363). It formerly required Quit (row 4) to stay focused until click, the reported D6 failure. It now requires Play Current Game (row 1) immediately after motion, with the same page and construction counter. An isolated clone composition check verified the fill colors for every visible win/loss choice. [Build](../../build/menu-hover-001/build.log) · [Focused runs](../../build/menu-hover-001/focused.log) · [Results colors](../../build/menu-hover-001/results-panels.log)

All **21 promoted recordings** returned exit 0, `PASS`, `won=1` and `completed=1` with the rebuilt `build/agent-native/main/webolution.exe`. Swings Both Ways reproduced 16/16 at step 12922. The log records each command, filename and outcome. [Per-recording results](../../build/menu-hover-001/promoted-replays.log)

**Handoff** — Ready for independent review. Supervisor smoke is pending.  
**What to check** — On the main menu, move from Play Current Game to Options without clicking. Expect the orange highlight to follow silently. Keep the pointer still and press Up. Language Selection should take focus. On Swings Both Ways, hover Continue after a win and click it to return to the map.
