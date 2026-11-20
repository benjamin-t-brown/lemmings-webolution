# Platform and asset comment pass

Reviewed 70 files and revised comments in 63 across `src/assets` (34), `src/audio` (6), `src/gfx` (3), `src/shell` (8), `src/tools` (6), `src/web` (5, including both JavaScript files), and `src/Makefile` (1). [Full comment diff](../../build/comment-pass-platform-001/light-map-revision/comments.diff)

The latest revision changes one comment in `CollisionGrid.h`. In `FUN_00403F40`, `y * 128 + (x & 0xff) / 2` selects a byte from the map at `+0x664`. The load at `0x00403F7E` supplies the index of a 24-byte palette record at `+0x678`. The retained instructions scale the byte at `0x00403F80` and address the record at `0x00403F86`. [Revision diff](../../build/comment-pass-platform-001/light-map-revision/review-revision.diff) · [Static evidence](../../build/comment-pass-platform-001/light-map-revision/static-evidence.txt)

The pass also preserves the `0x486819` lookup operands `table[lang][0]` and `table[lang][idx + 1]`. The `flag5f` observation in `MM_CLIMB_002` depended on injecting `0x100` into two floor cells. The effect on the authored level remains unestablished. [Retained collision observation](../../old/docs/scratch/phase3-round15-acid.md#mm_climb_002--paint-0x100)

All 63 comment-stripped pairs match, and all 59 C++ and include pairs also match under registered MinGW GCC preprocessing. LF and scoped `git diff --check` passed. Every changed file has a confirmed comparison. [Per-file comparison](../../build/comment-pass-platform-001/light-map-revision/comparison.json) · [Verification log](../../build/comment-pass-platform-001/light-map-revision/verification.log)

The HEAD comment audit matched 1,683 candidates, including all 44 indexed operands and 23 sampled references. The remaining 311 candidates have recorded false-positive dispositions. The seven history-scan matches use technical terms such as fixed stride and numeric rounding. Development-history exceptions total zero. [Reference inventory](../../build/comment-pass-platform-001/light-map-revision/references.tsv) · [Sample](../../build/comment-pass-platform-001/light-map-revision/reference-sample.tsv) · [Token dispositions](../../build/comment-pass-platform-001/light-map-revision/candidate-dispositions.tsv) · [History dispositions](../../build/comment-pass-platform-001/light-map-revision/history-dispositions.tsv)

The preceding supervisor run passed 95 smoke tests and 84 recovery tests. [Prior run log](../../build/agent-runs/20261120T164834-47744-71b77fd6/verification.log)

**Handoff.** `comment-pass-platform-001` is `needs_review`. Review the diff and archived checks.\
**Gate.** Supervisor smoke is pending.

**What to check.** Open `MM_CLIMB_002` or any installed level, wait for the hatch, and assign a skill. Input, rendering, audio and gameplay should behave as before on every level.
