# Actor behavior attachment and type query

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
These routines extend the typed actor scheduler described in
[actor_update.md](actor_update.md).

## Static evidence

| Routine | REA pseudocode evidence |
|---|---|
| Attachment `0x0042E696` | `ev_79c33bc80e8f8fa604b13dcdc6d8c2be86158ce404fd5f4ba5ed42b2ba6f3790` |
| Type query `0x0042ECC9` | `ev_e539b939ca8ffe0bfd177bf52e059780ab4489031ee5521c003efa23baedeeec` |
| Extra-state flag `0x00430A50` | `ev_b6c7366d08297454c322b6777e81916ad38ff8a4b3a81221d42b702c701f9ee4` |
| Type ID `0x00430A70` | `ev_792358cbd909cda1b0a3fd52b810a32f2a7c76a6bba6ad5aa4c82c3185c205b0` |

Local disassembly corroborated these routines and followed attachment's list
insertion through `0x00430290`, `0x00430590` and `0x00430630`. This establishes
that extra states are **prepended**, not appended.

| VA | File offset | Length | SHA-256 |
|---|---:|---:|---|
| `0x0042E696` | 190102 | 116 | `7fd9a9d9b7942dc958956a910a5379f17c4a52236d10be1a8f4334bc918fa9fb` |
| `0x0042ECC9` | 191689 | 132 | `0a76be8749b49bf144bcd4af0cac7fff7af37d37cb78dea6a7d663fa20c6a453` |
| `0x00430A50` | 199248 | 17 | `6e3d11629cf3a32c1e772d65313b8c31bf1b78d21c58555f949dd329a5326f5d` |
| `0x00430A70` | 199280 | 17 | `d3bc3e852e9ca92d62e0d140c341815bf8b7b6e7b043f9f8b0a5c45665694124` |
| `0x00430290` | 197264 | 25 | `0782df8156e4ec4a9fad514bc8874375d7a7e5edcbacd2f747e9634fb380e29f` |
| `0x00430590` | 198032 | 147 | `92bafe77b038dc356a5bb3e5af77bf2e4cb7aae2eaf08666c61c06803d4550b5` |
| `0x00430630` | 198192 | 44 | `5d03e6ed2be1fdea3fb7309a5e517ebec66f39ab63a01cbe660434509585702c` |

All ranges are pinned in `run_native_actor_update_oracle.py`, verified against
the whole-file hash and PE source-byte mapping, and supplied from the installed
executable at test time. No original executable bytes are stored in the repo.

## Recovered behavior

`attach_actor_behavior` calls the incoming state's entry callback (virtual
slot `+8`) before inspecting its byte field `+0x11`. The callback can therefore
change how the state is routed.

If that byte is nonzero, attachment allocates a list node, writes its behavior,
next and previous pointers, then updates the old first node's previous pointer
and the begin sentinel's next pointer. The current primary is unchanged.
Allocation remains a callback so node ownership is explicit; a nonnull result
is required, matching the original's subsequent dereference.

If the byte is zero, the current primary's leave callback (slot `+0xC`) runs
when the primary is nonnull, and the incoming state is then assigned as the
primary. The current primary is read after the entry callback. Attachment does
not destroy the old primary. The scheduler separately handles destruction
when its result is 1.

`has_actor_behavior` returns true when either the optional primary or any
extra state has a matching 32-bit field at `+0x14`. It checks the primary first
and otherwise walks extra states from the list front. The original compares
the complete 32-bit ID; negative values and duplicate IDs are not special.

## Verification

The portable unit test checks missing queries, primary replacement, null
previous primary, noncanonical extra flags, prepend order and all list links.
It passes on x64 and x86.

The existing scheduler oracle was extended to execute the original attachment
and query functions plus their flag, type-ID and insertion helpers unchanged.
Only node allocation is adapted for this path; entry and leave virtual calls
use deterministic state callbacks. Its 10,000 attachment scenarios compare
ordered traces, primary identity, exact extra-state order and bidirectional
list links after seven attachments each, with nine queries after each attach:
**70,000 attachments and 630,000 queries** passed on 2026-10-29 UTC.

Cases include null/non-null initial primary; flags 0, 1, 2 and 255; entry
callbacks that toggle the routing flag before the routine inspects it; signed
and random 32-bit IDs; repeated IDs; and missing IDs. The scheduler's existing
20,000 comparisons now also use the original attachment routine in forced
transitions, eliminating the prior attachment-body adapter.

Allocation failure, actual state entry/leave effects, state-specific update
behavior, game-world execution and exception handling remain outside these
claims. This is offline native-byte differential execution with declared
adapters, separate from the static reading and portable unit tests.
