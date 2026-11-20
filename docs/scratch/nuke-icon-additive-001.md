# nuke-icon-additive-001

[Playtest 21 L5](../playtest/2026-11-19-playtest-21.md) reported a black rectangle behind the nuke icon. I reproduced it on **2 sides to every story** (node 30, `M08_KF_1`). The clone sampled opaque black from `Sheets\~Additive0` and replaced the plate RGB through its straight-alpha icon renderer.

## Original blend

The registered x86 `Lemmings Revolution/Lemmings Revolution.exe` has SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. Ghidra 12.1.4 pseudocode and instruction excerpts agree on this path.

- In `0x45E51F`, `0x45E7AB..0x45E7BD` registers `AdditiveIcons` (string `0x4C0D5C`) and stores its handle at game `+0x1C910` (absolute `0x4E3318`). Panel init reads that handle at `0x45A8A5` for slot 9 and sets that icon quad's first word to 2 through `0x406240` at call site `0x45A8D8`. [Setup records](../../build/nuke-icon-additive-001/static/setup.json)
- Panel draw calls `0x47A75D` at `0x45B784`. Its selected frame reaches `0x47AB96`, which binds the texture and calls `0x414920`. That quad draw passes the first word to `0x414D80` at `0x41494F`. Selector 2 enables blending and writes 2 to both blend render states. In Direct3D terms these are `SRCBLEND = ONE` and `DESTBLEND = ONE` (`0x414DA5..0x414DEA`). [Draw records](../../build/nuke-icon-additive-001/static/draw.json) · [Blend records](../../build/nuke-icon-additive-001/static/blend.json)

REA closed without a snapshot. The instruction trace establishes the blend factors. Original color precision and full-frame fidelity remain unverified.

## Clone change and checks

Desktop and browser share `runApplicationLeg → drawRuntimeSkillPanel → RetailSkillPanel::draw → drawSkillPanelIcon`. The texture loader already tags the `~` sheet as additive. The panel sampler now uses that tag to clamp each RGB sum to 255. All other icons follow their decoded straight alpha. The clone's current Armageddon pose is temporal entry 6 of the installed sheet's 13 entries in each state.

The complete production sheet-use inventory is the in-level Armageddon plate 9 and the opt-in headless `--skill-panel` draw through the same `RetailSkillPanel`. `RuntimeHud` draws the outline and status text separately. The HUD counters use `^OUT`, `^SAVED`, and `^TIME` BMPs, and `RuntimeCursor` uses the `_cur*` BMPs.

- The native clone's SDL framebuffer showed the plate through all 144 previously black sprite pixels in each of four observations. Exactly 270 RGB pixels changed in `x=29..43, y=377..394`; the other nine plate crops were byte-identical. The first press/release pair armed without spending stock, the second spent the one nuke, and a third left the countdown running. The observer used a private temporary save path. [Before crops](../../build/nuke-icon-additive-001/before/) · [After crops](../../build/nuke-icon-additive-001/after/) · [Pixel comparison](../../build/nuke-icon-additive-001/pixels.tsv) · [Before input trace](../../build/nuke-icon-additive-001/observer-before.log) · [After input trace](../../build/nuke-icon-additive-001/observer-after.log)
- The [native build](../../build/nuke-icon-additive-001/native-after-build.log) succeeded. All 25 promoted recordings passed with a win and normal completion. The latest Playtest 21 win, `E06_KF_1`, passed at step 27785 with 6/5 saved. [Every replay path, step, result and log](../../build/nuke-icon-additive-001/replay-results.tsv)
- Emscripten 5.0.3 linked a matched `webolution.js` and `webolution.wasm` under `build/nuke-icon-additive-001/browser-dist/`. The isolated build used the registered Python and the existing SDL2 cache and port files. The preset directory still holds the mismatched pair from the earlier loader write denial. The loopback server returned the new distribution files with the required isolation headers. [Configure](../../build/nuke-icon-additive-001/browser-dist-local-configure.log) · [Link](../../build/nuke-icon-additive-001/browser-dist-local-build.log) · [Earlier write denial](../../build/nuke-icon-additive-001/wasm-build-registered-python.log) · [HTTP check](../../build/nuke-icon-additive-001/browser-serve-check.log)
- The matched pair ran all 25 promoted replays under the registered Node runtime with wins and normal completion. Its headless `--skill-panel` render of `M08_KF_1` matched all 270 desktop RGB pixels in the icon rectangle and showed the plate through the 144 formerly black sprite pixels. These checks cover the Wasm renderer and replay path. [Wasm replay results](../../build/nuke-icon-additive-001/wasm-replay-results.tsv) · [Wasm frame](../../build/nuke-icon-additive-001/wasm-headless-panel.png) · [Pixel comparison](../../build/nuke-icon-additive-001/wasm-headless-pixels.txt)
- Headless Chrome and Edge each requested the page, then timed out before requesting `webolution.js`. Both used private profiles, and the page used a private in-memory save path. The three in-browser canvas states remain unverified. To serve the matched production page on loopback, run `& 'C:/Program Files/Python313/python.exe' build/nuke-icon-additive-001/browser-check.py --serve` and open the URL it prints. [Chrome HTTP log](../../build/nuke-icon-additive-001/browser-canvas/http.log) · [Chrome process log](../../build/nuke-icon-additive-001/browser-canvas/chrome.log) · [Edge HTTP log](../../build/nuke-icon-additive-001/browser-canvas-edge/http.log) · [Edge process log](../../build/nuke-icon-additive-001/browser-canvas-edge/edge.log)
- The prior supervisor gate passed 95 main smoke tests and 84 recovery tests. [Prior gate](../../build/agent-runs/20261120T051137-44600-3ea9ae7b/verification.log)

**Handoff** — `needs_review`. Supervisor `verify.py --profile smoke` pending.  
**Next action** — Verify the three canvas states in an interactive browser using the isolated build and review the new smoke result.

**What to check** — Open **2 sides to every story**, look at the bottom plate, click it once, then click again within half a second. Expect the plate to show through the nuke icon at rest, while armed and after use. The gold confirmation outline and countdown should still appear.
