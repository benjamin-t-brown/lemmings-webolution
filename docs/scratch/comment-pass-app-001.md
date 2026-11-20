# Application comment pass

The pass covers 65 files in `src/app`. This revision updated comments in 21 of them. It restores the `climb` SPR clip in `TubeScene.h`, the `kCellFlag0x400` door and gate stamp bit in `TubeScene.cpp`, and the `SELECTARG1` alpha operation in `BackdropScene.cpp`. The earlier `rand()/32767.0` and fast-forward pass-count corrections are present. [Task diff](../../build/comment-pass-app-001/final-reference-check/comments.diff) · [Final revision diff](../../build/comment-pass-app-001/final-reference-check/revision.diff)

Registered MinGW GCC preprocessing and a byte-preserving comment stripper matched all 65 files across HEAD, the original working-tree inputs, the reviewed revision and the final worktree. The LF and whitespace checks passed. The history scan found four uses of “fixed,” all describing a constant value or extent. Zero development-history comments were retained. [Comparison](../../build/comment-pass-app-001/final-reference-check/comparison.json) · [Verification log](../../build/comment-pass-app-001/final-reference-check/verification.log)

The mechanical audit matched 1,410 HEAD comment entries across its reference patterns, including all 19 reference samples. It records each match with its original and final line numbers. The broader scan produced 418 reviewed candidates such as clone identifiers, source paths and development labels. Zero original-reference misses remain. Original unlimited and out-of-range skill-stock formatting remains unresolved. [Inventory](../../build/comment-pass-app-001/final-reference-check/references.tsv) · [Sample](../../build/comment-pass-app-001/final-reference-check/reference-sample.tsv) · [Candidate dispositions](../../build/comment-pass-app-001/final-reference-check/excluded-candidates.tsv)

**Handoff.** `comment-pass-app-001` is `needs_review`. Review the final revision diff and archived checks.\
**Gate.** Supervisor smoke is pending for this revision.

**What to check.** Open any installed level, such as `MM_CLIMB_002`, and assign Climber after the hatch releases. Input, rendering and gameplay should behave as before on every level.
