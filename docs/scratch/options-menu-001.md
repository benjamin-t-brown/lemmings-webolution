# Options opens sound settings

**needs_review** · Playtest 14 D2. The clone ignored main-menu row 3 on click and Return. During a level, the Escape menu displayed Options and consumed its action. Both now open the sound page. The reported failure applies across levels, including the installed Swings Both Ways route (`MM_E1_5_SKILLS`, node 23). [Playtest entry](../playtest/2026-11-18-playtest-14.md)

## Original screen

Static Ghidra reads of `Lemmings Revolution/Lemmings Revolution.exe`, x86 SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. The row comparisons and pushes at `0x48EE59..0x48EE6C` and `0x48F610..0x48F619` select page 9, passing return selectors 7 (main menu) and 0x1F (level menu). Accept dispatches the stored selector at `0x48E976..0x48E99E`. Cancel takes the common return path through `0x48896A` and `0x48EB70`. O in UI2 tears down the frontend and writes paused UI1 at `0x45D575`. [Routing decompilation](../../build/options-menu-001/routing.json) · [Event instructions](../../build/options-menu-001/sound-events-asm.json)

Page 9 builds **Music**, **SFX**, and **Accept Changes And Exit** from UK strings 11, 12 and 24. Music starts focused. The 640×480 frontend uses MAIN_LOGO and row tops 320/378/436. In-level, the rows start at 100/158/216 over the suspended scene. `0x48E5B4` builds the rows and arrow controls. `0x490CAE/0x490D46` lay out sixteen 8×10 green/red segments at 11-pixel intervals. The installed left/right arrow textures are 16×16. [Builder instructions](../../build/options-menu-001/asm-48e5b4.json) · [Widget geometry](../../build/options-menu-001/widgets.json) · [Installed labels and art](../../build/options-menu-001/composition.log)

The audio manager at `0x58CB88` holds two doubles at +0x1E0 and +0x1E8, both initialized to 1.0 by `0x498890`. Page 9 previews through setters `0x498993/0x4989F9` and requests the Changeop sample for SFX. I traced storage to process memory. The settings registry routines `0x45BFED/0x45C3D3` have no volume entry, and the `0x45F94D` progress writer has no traced volume field. Original persistence across launches remains unestablished. [Registry paths](../../build/options-menu-001/registry-values.json) · [Save format](save-format-research.md)

Selector **0x19**, reached by the empty fifth level-menu widget, offers Skill Names Enabled/Disabled, Cursor Reversed/Not Reversed, Fixed/Dead zone/Free Floating Cursor, and Accept Changes And Exit. Selector **8** contains Audio (9), Game Options (0x19), and Return (7). Both assigned Options rows bypass it. These additional pages remain unimplemented. [Page handlers](../../build/options-menu-001/pages.json) · [Label/selector table](../../build/options-menu-001/control-table.json)

## Clone behavior

`FrontendWindow` and `runApplicationLeg → LevelMenu` use the same `FrontendSession` sound editor. Up/Down chooses a row. Left/Right or the installed arrows adjust it. Clicking a volume label or pressing Return on it lowers that bus. Accept returns to the calling menu. Escape restores the entry volumes and returns there. In-level O closes directly to pause. The running level, cue clock and selection stay under the existing menu pause gate. [Sound page](../../src/app/SoundMenu.cpp) · [Session](../../src/app/FrontendSession.cpp) · [Level input](../../src/app/main.cpp)

`CuePlayback → Mixer::fill` applies the gains to the current music and both effects mixing paths. A track selected while muted advances and can be unmuted mid-track. Accept writes the versioned, three-byte **provisional** `SoundSettings.dat` alongside the clone's `SavedGame.dat` through the guarded, temporary-file replacement path. Startup applies it before playback starts. New Game and level completion continue to own progress writes. [Mixer](../../src/audio/Mixer.cpp) · [Sidecar format](../../src/save/SoundSettings.cpp)

Provisional choices include the prior clone defaults of 30% music and 100% effects, one meter segment per SDL adjustment, keeping edits while navigating, and saving the live values on O-close. The original handler uses a time-dependent increment and restores the entry snapshot on a handled common event at `0x48E903..0x48E928`. Native held-input timing, arrow visibility and physical output remain unverified. Ghidra was closed without a snapshot. No original process ran. [Clone widget render](../../build/options-menu-001/sound-main.png)

## Checks

The registered UCRT64 build passed. Four focused tests passed. The PCM test covered a muted music start and live changes to active effects. The sidecar test reopened both values and preserved all 1,504 bytes of a private progress image. Two existing smoke expectations encoded D2. `frontendsession` previously required row 3 and its click at (320,421) to leave the screen unchanged. It now requires page 9 and preserved progress. `levelmenu` now expects the sound action at the same strict row bounds. [Build log](../../build/options-menu-001/final-build.log) · [Test commands and results](../../build/options-menu-001/final-focused-checks.log)

All **21 promoted replays** passed individually with exit 0, `won=1`, and `completed=1` using `build/agent-native/main/webolution.exe` and private LocalAppData. The log gives each filename, final step and saved/required count. No recording needed retirement. [Per-recording results](../../build/options-menu-001/promoted-replays.log) · [Changed files](../../build/options-menu-001/changed-files.txt)

**Handoff** — Options is implemented from both callers. Supervisor smoke is pending.  
**Next action** — Independent review and human playtest after the gate.

**What to check** — From the main menu, change Music and SFX and select Accept. Restart and reopen Options to hear the saved balance and see the same meters. On Swings Both Ways, press Escape → Options, try a different balance, then Escape to cancel. Expect the prior balance and the level menu with Resume focused. O from the sound page should leave the level paused.
