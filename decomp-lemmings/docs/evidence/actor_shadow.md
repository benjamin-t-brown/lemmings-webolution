# Actor effect geometry update

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Complete original instruction bytes are in [actor_shadow.asm](actor_shadow.asm).
The name "shadow" follows the effect's use; the numerical render mode fields
are not assigned inferred graphics meanings.

| Routine | VA / PE file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Effect update | `0x458186` / `0x58186` | 4775 | `f58dc109bbc1701770603607a5c7453613f11a85e3c20c57b6a174bc09fddc86` |
| UV pair setter, inlined in source | `0x413560` / `0x13560` | 59 | `e10637d1d1606b2cd36380cca7f81c8917aa7893dc17ffcb46e440a2bc70f821` |

REA evidence: update
`ev_15f664172f97fc1d89cbc25b207a4e8918f4efbaca068a4448ca3ec3b3fbd0f8`;
UV setter `ev_208c3ddaa89a7974ee842898e76f859d44559778b6bde8cf81603607c2acdf36`.
Supporting boundary evidence: segment construction `0x459557`,
`ev_2c95f562a5a2f88dba00f87b9d821cfa9508a33b958ad0844310668090d9fff6`;
resource activation `0x47261F`,
`ev_4ac7c3db29e060f88eeec837d2e774503baf6bee6716196a25e37e9f067686a6`;
list constructor `ev_1b3f2197644950f089ef32c22467dbf963566f6542b13983587bedc3d2bea976`,
resize `ev_352d6af46d6bb95ea0421b0a093bca3629c3e2ccfd32b8f4b51fc08d1de822be`,
destructor `ev_aa227496847f583c2669a36e341a2cfe0d3ea5c9b3ec626a6745420fab92bb0e`.

## Static behavior

The update constructs a temporary point list and resizes it to one before
copying the associated actor position into effect `+3B0`. Actor vertical
direction exactly -1, nonnull gadget `+54`, or membership in state `0x23`
clears effect visibility `+3AD` and destroys the list. These exits preserve
previous vertices, UVs, mode `+120` and intensity `+3C8`.

Otherwise the concrete `0x42E913` terrain contact query runs. Contact sets the
shared width at `0x4C05C8` to 2.5, visibility to one and intensity to 255. Without
contact, the concrete `0x42F788` scan supplies distance. Only ordered distances
strictly between zero and ten remain visible. This path subtracts the distance
from copied Y, stores it, then adds one and stores again. Intensity is the
low-32-bit x87 integer conversion of `255 - (distance / 10) * 255`, with no
intermediate double store. Width persists from the explicit global state;
the executable's initial data is zero and this routine overwrites it only on
contact. There is no recovered evidence for assuming a fresh width of 2.5 on
every call.

The actor position is copied again for the segment endpoints. Left/right X are
actor X minus/plus half the current width, individually stored to binary64
before the original cylinder wrapper. `0x459557` builds the terrain point list
at the explicit callback boundary. The update computes the absolute difference
between last/first X, then normalized X offsets for every list entry. Each
iteration fetches its point before fetching point zero, matching the original
pointer read order. It looks up global resource ID `0x4C05C0` in the resource
table at `0x58CB48` and activates the result before testing list size again.

With three entries, mode becomes one and eight vertices are constructed from
these `(X entry, Y entry)` pairs: `(0,0), (1,0), (1,2), (2,2)`. Each contributes
two vertices, using actor depth plus 0.25 followed by minus 0.25. With four
entries, mode becomes five and the pairs are `(0,0), (1,0), (1,1), (2,2), (2,3),
(3,3)`, yielding twelve vertices. The Y lift is binary64
`0x3FC851EB851EB852` (0.19). Depth is read and stored before the Y-list access;
lifted Y is stored before the X-list access. Every vertex goes through the
concrete recovered world mapping `0x43D56C` before assignment.

The UV setter writes two doubles at group `+28 + corner*18` (hex offsets),
leaving the other corner fields untouched. Each quad gets V values `0,1,1,0`.
For three entries only group zero is written, spanning normalized `u0` to `u1`.
For four entries the five groups span `[u0,u1]`, `[u1-1,u1+1]`, `[u1,u2]`,
`[u2-1,u2+1]`, `[u2,u3]`. Counts one/two activate the resource but retain old
geometry and UVs. A zero X span is not repaired; NaN/inf UV arithmetic is retained.
All normal exits destroy the temporary list.

## Verification

Unit tests pass on x64 O2 and x86 O0. They check disabled paths and list cleanup,
position copying, both mesh modes, UV endpoints, contact width/intensity,
airborne projection/intensity and preservation on later hidden updates.

O0 and O2 native runners each execute **12,000 full original update calls with
segment fixtures**:
7,785 visible results, including 1,864 three-point and 2,031 four-point mesh
updates. The complete original update bytes and UV setter run unchanged, along
with the original terrain-contact/scan, coordinate wrapper, world mapping and
vector helper chains. Explicit adapters cover state membership, point-list
lifecycle/access/segment construction, resource lookup and resource activation.
All code and constants are verified against the executable before execution.

The runner compares ordered callback traces with snapshots of visible state,
intensity, mode, copied Y, actor depth and global width; exact final actor/effect
storage including untouched bytes; and the shared width. Mesh coordinates and
finite UVs compare bit for bit. NaN UVs compare by classification rather than
payload. Mutation scripts alter actor position during list resize, state query,
resource activation and point accesses, and alter point zero during list
access. These check the original read order. Deterministic distance cases cover
zero, negative values, both sides of ten, exact ten, infinities and NaN, with
zero, positive and negative finite widths. Random cases include list sizes one
through four and zero spans.

A second pass at both optimization levels executes **12,000 further updates
with the original `0x458186 -> 0x459557` chain**. The source callback composes
`build_actor_shadow_segments`, and the original segment routine's entry is
restored to its verified bytes. Thus segment geometry is concrete on both
sides; only list/state/resource operations remain adapted. These cases produce
7,702 visible results, 2,674 three-point and 5,028 four-point meshes. Ordered
traces, full effect storage, actor mutations and width match, including list
access mutation scripts during actual segment generation. Total comparison
coverage is 24,000 full update calls per optimization level.

## Boundaries and limits

The callback API retains segment extraction `0x459557` as a composition point;
its concrete implementation and separate native evidence are now in
[actor_shadow_segments.md](actor_shadow_segments.md). Dynamic list allocation
and Direct3D-facing resource activation remain explicit boundaries. This does
not claim an observed rendered shadow. Resource initialization and actor/effect
allocation are also outside this slice. Original exception unwinding is not
reproduced for throwing callbacks.

The comparison requires valid actor/grid/mesh data, finite actor X/width/list X,
terminating wrapper and terrain scans, stable list lengths from one through
four, and list Y values accepted by the mapped mesh. Original code uses
unchecked stack storage/indexing beyond that list domain. Nonfinite width/X
would additionally enter exceptional CRT absolute-value paths, which are not
included in this native runner. NaN UV payloads, FP status flags and concurrent
mutation are not claimed.
