# Behavior lookup and removal by type

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Complete original bytes and local disassembly are retained in
[actor_behavior_removal.asm](actor_behavior_removal.asm).

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Remove | `0x42E7F3` / `0x2E7F3` | 212 | `05d08169889a56bf462db385c68e81f287fd2cd5a11d61ee0021aa72206f9f40` |
| Find type | `0x42ED4D` / `0x2ED4D` | 128 | `7489b69dcd94fe9bb072ea93b9ef01d0e38db74713ae7fa5d0301095ff94354b` |
| Find node by identity | `0x4301B0` / `0x301B0` | 77 | `4135f72767e6100f8a64e843517ff962f260dd139ebc4932b910fc50a8d94535` |
| Unlink and release node | `0x42FEF0` / `0x2FEF0` | 83 | `7bc9223189ff9857ef8bcb775ae6a6aec1dffe2f66e3631cb95f3c7d32d675d5` |

REA static evidence, respectively:

- `ev_8627cd12a987c91d224042c847a9aefc070293fd7a02ef5fff0dd99e1206e2d1`
- `ev_2ae8fe83d4c9493efc36d401364849319df430b2d896dd367e3f11e84c124cf9`
- `ev_7765f815d9edace0f580c4a99e3228afda3b869109fe4ec7ed329bb949dcd7b8`
- `ev_20b8e11140a7f883faab1fcc5a234573b2ad53bf0c7fbf836ff52f9bfad78833`

## Static behavior

`find_actor_behavior` checks primary type first, then extra behaviors in forward
list order, returning the first matching behavior or null. Unlike the previously
recovered membership query at `0x42ECC9`, this function unconditionally calls
type accessor `0x430A70` on primary, so its contract requires nonnull primary.
The accessor directly reads `+14` (static evidence
`ev_bf6d343c7be55e7bd191b99aba55c550b2e06a3b80c328b8d140d233a5109afc`).

Removal returns false for an absent type. Otherwise it invokes selected state's
leave with the actor, then compares that cached pointer with the actor's current
primary. If equal, it invokes the current primary's deleting destructor with
flags 1 and clears primary unconditionally, even if destruction replaced it.

If unequal, it searches the current extra list again, this time by pointer
identity. It destroys that node's behavior with flags 1, then reads the node's
links and unlinks it: previous.next is written before next.previous. It releases
the node through `0x42FF50(node,1)` and returns low byte true. Node links are not
cleared before release. A leave that moves the state in the list or changes
whether it is primary affects the chosen branch; destruction can change the
links subsequently used for unlinking.

The source exposes the existing logical actor/state/list views and deleting
destructor/leave callbacks. Node release uses `ActorBehaviorClearHooks` after
unlinking. Valid traversal and lifetime are required. After leave, the selected
state must be current primary or still reachable by identity in the extra list:
the original dereferences an absent-node search result. No guessed missing-node
or initially-null-primary behavior is introduced.

## Unit tests and native comparison

Unit checks cross sizes 0 through 16, six query types and eight callback modes,
including duplicates, absent types and primary priority. They verify false with
no callbacks for absent type and leave-before-destroy/release for selected state.
They pass on x64 and x86.

The native actor-helper oracle runs the unchanged four routines, original type
and list accessors, with explicit leave/destructor/node-release adapters. Its
816 lookup/removal cases match source at O0 and O2 in selected pointer, low
return byte, complete callback trace and final primary, sentinel, node and type
snapshots. Mutation cases move primary into extras with a replacement or null
primary, promote an extra to primary, change the selected type during leave,
move a selected extra to the tail, insert a node during destruction, replace
primary during destruction, clear node payload during destruction, and change
released-node links. Adapters retain object storage so callback-time pointer
observations remain valid. Actual heap release and full-game runtime are not
claimed. The existing 20,000 Y moves and 85 two-pass cleanup comparisons still pass.

## Clear primary behavior

`clear_actor_primary_behavior` recovers `0x42E725` (101 bytes, file offset
`0x2E725`, SHA-256
`c5cf8bc9b8e06c427119c28fe34ca99f56383d36896ff33105f0297a01841942`).
REA evidence: `ev_332b722b66290f4c4764583f763faac90f1d9b48d833703d0b54589c1a3deb6f`.
It leaves the initial primary, rereads the primary after that callback,
destroys the current primary if nonnull, then clears it unconditionally.
Consequently a leave that replaces the primary causes the replacement to be
destroyed; a leave that clears it skips destruction. A destructor-installed
replacement is discarded by the final clear. Initial primary must be nonnull.

The helper oracle adds 136 original-byte comparisons across list sizes and
mutation modes, checking callback order, selected state identities, deleting
flags, and complete list/primary snapshots. Synthetic units separately check
the ordinary, replacement, null-after-leave and replacement-during-destroy
cases. Real state destructor effects remain adapters.
