# Simulation and save comment pass

Edited comments in 11 files. `src/sim` covers `RetailLevel`, `Terrain`, and `World` (`.cpp` and `.h`). `src/save` covers `SavedGame.h`, `SaveStore.cpp`, `SaveStore.h`, `SoundSettings.cpp`, and `SoundSettings.h`. The pass removed development history and retained descriptions of behavior, units, ordering, original-binary references, and provisional or clone-specific labels. No comments required a history exception.

The hatch-arm comments now describe the provisional `MM_CLIMB_002` delay as shared by all installed levels. The common loader assigns it and `World.cpp` uses it to arm the elapsed release scheduler. `World.h` again records `cmp eax, [ebp-8]` at `0x0041B21E` and `fsubr [edx+8]` at `0x00413A79`. [Loader](../../src/sim/RetailLevel.cpp#L417) · [Scheduler](../../src/sim/World.cpp#L1295) · [Archived instructions](../../build/comment-pass-sim-001/inputs/src/sim/World.h)

The baseline is the working tree at task start, including five files with pre-existing edits. Registered MinGW GCC preprocessing and a byte-preserving comment stripper produced identical before and after outputs for all 11 changed files. The scoped whitespace, LF, and comment history checks passed. [Comparison manifest](../../build/comment-pass-sim-001/comparison.json) · [Verification log](../../build/comment-pass-sim-001/verification.log) · [Task diff](../../build/comment-pass-sim-001/comments.diff)

The reference inventory pairs exact references with their before and after line numbers. Twelve sampled references matched, including both instruction addresses above. The prior supervisor run passed 95 main smoke tests and 84 recovery tests. [Reference inventory](../../build/comment-pass-sim-001/references.tsv) · [Prior supervisor run](../../build/agent-runs/20261120T090001-40472-1a9e6e63/verification.log)

**Handoff.** `comment-pass-sim-001` is `needs_review`. Review the task diff and archived comparisons.\
**Gate.** Supervisor smoke for this revision is pending.

**What to check.** On `MM_CLIMB_002`, let the hatch release, assign Climber, and watch the lemming climb and hoist. Release, movement, and skill behavior should match the pre-pass clone on this and every other level.
