# Recovery folder tracking repair

Implemented 2026-10-29, America/Denver, at the user's request. The user clarified
that the files matter; the old recovery repository history is not required.

`decomp-lemmings/` is now an ordinary folder in the parent repository's index.
The broken gitlink was removed from the index, and 643 source/evidence files
were added. No commit or push was created. Review the staged import and commit
it normally when ready; future clones will then contain these files directly.

## Preservation

- The moved and standalone trees initially contained 644 identical files outside
  generated/build folders. There were no source differences to merge.
- All moved files remain in place. `hooks/.clangd` remains locally available
  and ignored, accounting for the difference between 644 local and 643 staged files.
- The old `C:/progs/decomp-lemmings` copy is untouched. No old history was
  imported, bundled, merged or rewritten.
- Original file bytes and the pre-repair parent index are saved in
  `build/recovery-tracking-repair/20261030T031358Z/`, including
  `moved-source-files.zip`, `original.index`, file hashes and formatting changes.
  The timestamp is UTC; these are local ignored backups, not committed artifacts.
- 45 files received whitespace-only normalization: LF line endings, legacy
  trailing spaces or extra blank lines at EOF. Non-whitespace bytes were checked
  against the backups. No C++ behavior was changed.
- Generated `build/` files, original binaries and game assets are excluded from
  the import. Unrelated existing working-tree changes were left alone.

## Verification

All 643 staged files were materialized into a separate ignored directory and
compared byte-for-byte with the working files. The index contains ordinary file
entries, no `decomp-lemmings` gitlink. The standalone source hashes still match
the original inventory. `git diff --cached --check` passed. The recovery GCC
build and all 76 source/asset tests passed after normalization.

## Repo-local reference and assets

The canonical reference is now `Lemmings Revolution/Lemmings Revolution.exe`,
relative to this repository root, with `lemmings.box` and `patch.box` beside it.
These files already existed inside the repository directory. Their SHA-256
hashes match the former external installation, so nothing needed moving or
overwriting. The registry stores relative paths; the verifier resolves them
against the repository root even when invoked from another working directory.

The whole local `Lemmings Revolution/` directory is now explicitly ignored for
new files. The reference EXE and both `.box` archives are not tracked or staged.
Previously tracked installation files remain unchanged as a separate cleanup
debt; adding an ignore rule does not remove already tracked files.

Legacy harness scripts that still embed external paths must be migrated or
given explicit repo-local paths before use. The supervised setup and current
plan use the repo-local files. The source-built application's default and the
Makefile's default now use this same folder; the root README examples agree.
The rebuilt application successfully rendered one headless frame from the repo
root without `--game-dir` or a `WEBOLUTION_GAME_DIR` environment override.
No original executable or original-byte oracle was run during this repair.
