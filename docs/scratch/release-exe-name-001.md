# Desktop release executable names

The 2026-11-22 release naming request follows Playtest 25 Q3's standalone
Windows download. The prior CMake install copied the build target's basename
into both packages. The path under review is desktop packaging before level
selection. [Playtest 25 Q3](../playtest/2026-11-21-playtest-25.md) ·
[Prior packaging source](../../build/release-exe-name-001/before/)

Both release jobs build `webolution` and stage it with `cmake --install`.
The shared native install rule now installs the target file as
`lemmings_webolution` with the platform executable suffix. The Windows job
runs the staged exe before zipping the stage, and the Linux job runs `ldd`
and `--version` against the staged name and lists it in the tar command.
The package and public READMEs use the installed names. Debug and release
share a Windows version resource whose `OriginalFilename` records the
build-tree name, `webolution.exe`.
[Install rule](../../CMakeLists.txt#L126) ·
[Release workflow](../../release/main/.github/workflows/release.yml)

- Registered UCRT64 tools built `native/webolution.exe` with the `gcc-release`
  configuration and version `0.2.0-rc.1`. The install produced
  `windows-release/lemmings_webolution.exe`. Both printed
  `Lemmings Webolution 0.2.0-rc.1`, with a Windows-only PATH for the staged exe.
  The local ZIP contained the renamed exe, four runtime DLLs, README and
  LICENSE. Temporary binaries and the ZIP were removed after verification.
  [Build](../../build/release-exe-name-001/build.log) ·
  [Install](../../build/release-exe-name-001/install.log) ·
  [Version and ZIP contents](../../build/release-exe-name-001/windows-package.log)
- LLVM parsed the workflow as YAML. The static check confirmed both install
  paths, both version commands, the Linux `ldd` path and tar member list, and
  the Windows ZIP input. Linux and hosted packaging evidence is static.
  [Workflow check](../../build/release-exe-name-001/workflow-check.log)
- The initial full export failed on development-history references in the
  release-rate and terrain-debris comments. The revision describes the behavior
  at `RuntimeReleaseRateControl::advance` and `TubeScene::emitTerrainDebris`.
  The revision diff contains only those comments. The full
  `scripts/export_main.py` run scanned 231 files and exited 0. All six packaging
  inputs matched their exported copies.
  [Initial export](../../build/release-exe-name-001/export-main.log) ·
  [Revision diff](../../build/release-exe-name-001/export-revision-diff.log) ·
  [Successful full export](../../build/release-exe-name-001/export-main-revision.log)
- The previous supervisor run built `smoke_tests` and passed 95 smoke tests,
  including the installed-level sweep, and 84 recovery tests. The production
  change concerns package staging, outside the recorded level routes.
  [Previous smoke gate](../../build/agent-runs/20261122T173654-46400-d7092a8c/verification.log)

**Handoff** — `needs_review`. The supervisor's smoke gate for this revision is pending.  
**Next action** — Review the two comment edits, the successful export and the new smoke result.

**What to check** — Extract a desktop package, put the two boxes beside the
renamed executable, launch it, and select **Just Climb** (`MM_CLIMB_002`).
Expect the game to find the boxes and open the level from the map.
