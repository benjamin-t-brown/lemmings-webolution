# Complete bounded native return capture — 2026-10-28

**Observed route pass, not full frontend acceptance.** One owned stock process
completed New Game → map → `MM_CLIMB_002` → menu Quit → returned map → footer →
main menu. Nine unscaled640×480 images and stable native state brackets are in
`build/tmp/enacter-native-return-20261028-1818/`. The corrected102-node progress
reader confirms no completion or unlock-state change on this abandonment route.

The [earlier failures and corrections](frontend-native-return-2026-10-28.md)
remain retained: missing root construction, assumed cursor center, misread
decimal strides and ambiguous heap highlight scan. They are not successful runs.
In particular, earlier102-word flag vectors remain retracted; this new correctly
addressed sample does not repair them retrospectively.

## Authorization and cleanup

After the bounded retries, the user explicitly authorized continuing until a
full capture. This packet's fixed window was18:18:55Z–18:33:45Z, with one launch
per immutable packet. Its claim was recorded18:27:37Z. No further launch was
needed under that grant; no timing research or second unlock-state run occurred.

The unchanged owned-job envelope verified the stock SHA256, one owned child,
hidden-worker foreground, unoccluded640×480 client geometry and stable brackets.
Save/config were backed up; only dgVoodoo fullscreen mode was temporarily changed.
Owned stock PID39964 closed with a signaled handle; input snapshot restored.
Supervisor required verified worker termination and no stock-like process before
restoring installed files. Independent main-agent checks at **18:28:11Z** found
no stock-like process and exact save/config/exe/shim hashes. The user was told
the desktop was free immediately, before offline scoring/build work.

## Native observation

| Image | Counter4C416C | Page vtable | UI | Frontend flag / cursor |
| --- | ---: | --- | ---: | --- |
| Menu | 3 | 4B2528 | 3 | 1 / 1 |
| Confirmation | 4 | 4B3F38 | 3 | 1 / 1 |
| Fresh map | 5 | 4B3EB8 | 3 | 1 / 1 |
| Right-held map | 5 | 4B3EB8 | 3 | 1 / 10 |
| Level entry | 5 | empty | 0 | 0 / 1 |
| In-level menu | 7 | 4B26B0 | 2 | 1 / 1 |
| Returned map | 9 | 4B3EB8 | 3 | 1 / 1 |
| Footer hover | 9 | 4B3EB8 | 3 | 1 / 1 |
| Returned menu | 10 | 4B2528 | 3 | 1 / 1 |

All backend bytes were0. Scored frontend pointer positions are320,240; footer
hover/click is320,455. The pointer was explicitly established before captures,
not inferred from the static initializer. First-node206,204 loaded the expected
compiled path. Main-agent inspection of all nine images confirms game pixels,
English labels and unobstructed frames. The two level images have no matched
camera/clock sample and cannot score simulation or full-level rendering.

Correct native addresses are `0x4E36E8 + 0x1B58*group + 0x118*slot`, DWORD width.
The saved address list matches the mapping table and both encoded multiply
immediates. All102 flags were0 on fresh map, at level entry, in the menu and on
return. Fresh/returned map states both have nodes0–2=3 and remaining99=1. Returned
raw records/flags and the screenshot were saved before the equality assertions.
The returned-map image is pixel-identical to the fresh map; returned menu is
pixel-identical to initial menu. These are native-to-native comparisons, not
proof of source-renderer equivalence.

## Structural focus, not a heap-search guess

The successful script follows `page=[5D59E4] → manager=[page+10] → focused row=
[manager+10]`. The last value is a **pointer**, whose index among five rows at
page+40 gives the highlight. It validates root/page/row vtables, owners, label
IDs125/126/120/3/43, circular row links and stable duplicate reads. Focus0 was
recorded around both menu screenshots and immediately before initial RETURN.
No DOWN tap was needed; nonzero-focus input behavior was not exercised here.

Static41-pin evidence and synthetic corruption tests are in
`build/tmp/enacter-menu-highlight/`. The packet records every structural read,
including address/size/raw bytes; `replay_menu_snapshots.py` reproduces the exact
predicates offline from those reads. This is the first live validation of the
complete structural chain, not a relabeling of the two ambiguous heap candidates.

## Source correspondence and limits

`test_levelmenuwindow` runs the actual application with SDL dummy through the
same counter sequence. Its return trace now queues absolute SDL motion, balanced
footer button down/up, and writes the composed frames at Map9 center, footer
hover and Menu10 center. No fixture replaces runtime progress, route ownership
or construction counters; the level clock is an own-app test clock, not a native
timing match. CTest owns the artifact directory via `WEBOLUTION_TEST_OUTPUT_DIR`.

The packet verifier checks nine snapshot records, exact executed-source/image
hashes, correct progress addresses, flags/states, focus and restoration receipts.
It passed; image inspection was performed separately.

Full640×480 normalized absolute RGB difference, unchanged0.02 threshold, no
exclusions, shifts or resizing:

| Frame | Error |
| --- | ---: |
| Menu | 0.00495742698120915 |
| Confirmation | 0.0022420002723311547 |
| Fresh map | 0.007834584354575163 |
| Right-held map | 0.007842690461601307 |
| Returned map, actual SDL application route | 0.007834584354575163 |
| Footer hover, actual SDL application route | 0.007817976409313726 |
| Returned menu, actual SDL application route | 0.00495742698120915 |

The first four production composition dumps were regenerated byte-identically
from the current tool; the last three are the connected application's exported
frames, not replacements with fresh-menu/map fixtures. Scoring uses
`tools/harness/compare_camera_frame.py --roi 0 0 640 480`; source PNGs, difference
PNGs and hash-bearing score JSONs are retained in the packet. Passing averages
do not remove the known exact-cursor-color residual.

Windows UCRT64 build and full retail-configured CTest **70/70 PASS,216.85s**.
After a comment-only trace-label update, the focused `levelmenuwindow` rebuild/
test passed again and all three exported frames remained byte-identical. The
native-packet verifier and structural byte replay also passed. No production
behavior was changed for these retries; only the source test/export wiring and
offline capture readers changed. All work remains uncommitted; user edits were
preserved. No later native launch is needed for this completed capture task.

**Remaining:** B2's broader retained route/negative-input coverage is not closed
by this single positive sequence; the gate's4C416C value is the construction
counter, not a new unobserved screen-ID field. B3 still needs its second unlock
state. No results/retry/confirmation-No, physical-device binding, timing, exact
cursor RGB, pointer handoff or in-level held-cursor acceptance is inferred.
