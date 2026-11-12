# Actor animation resource selection

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

## Static recovery

`0x42FD25` starts with the default resource ID indexed by actor category
`+0x4C` in global `0x4E335C`. It visits every override record in global
`0x4C5B10` in order. Each record holds a resource pointer and ID. If that
resource contains the requested exact C-string name, its ID replaces the
current selection. Therefore the **last matching override wins**, regardless
of actor category. It binds the selected ID through `0x46836D` on actor
`+0x128`, then invokes model `+0x78` virtual slot `0x30` with the original
name pointer. These two calls happen even if no override matches or the ID
was already bound.

`0x48408C` checks names in clip order using the original `strcmp`; it returns
true at the first match. Names are case sensitive and empty names can match.
The source helper shares `find_animation_clip`'s already recovered comparison.

The portable API exposes the global tables as spans and the binding/selection
boundaries as callbacks. Invalid categories, null override resources, and
missing callbacks are rejected rather than reproducing invalid pointer reads.
No texture acquisition, actor construction, or rendering is claimed here.

REA evidence:

- `0x42FD25`: `ev_cf92e042ce01a66d696d4f60b8f295d73d7bb7202a782b8bc8cf06694a2a613f`
- `0x48408C`: `ev_7717b998e95d1b705e3771b02a7e27b598ba6d9fcf9a2ddbc8200e8237716342`

Byte pins `(VA, PE file offset, length, SHA-256)`:

```python
    (0x0042FD25, 195877, 157, "bd3db9e2d45e8fccecce4bbefe963a322512f9e6c04fb7e314833b46cbaed439"),
    (0x0048408C, 540812, 103, "ff45973dd2bf698273bd545d7785aef0974f4fe3e9c671975263985953e9a32a"),
    (0x004303D0, 197584, 25, "4bc6a396629309176d0ee3fbabf45ff8ac1b1fcecd8a5b90aaa450d0480791d7"),
    (0x004303F0, 197616, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
```

## Verification

Unit tests on x64 and x86 cover empty resources, exact/case-sensitive/empty
names, missing names, multiple matches, signed IDs, default category choice,
and bind-before-select callback order.

The x86 native oracle executes unchanged `0x42FD25`, `0x48408C`, the original
override count/index helpers, clip count/index/name helpers, and `strcmp`.
Only `0x46836D` and the model virtual target are callback adapters. It compares
**20,000 resource selection traces and 160,000 contains queries**, with zero
through eight override records, zero through six clips, duplicate names,
random IDs, and all four default category slots. It verifies both callback
object offsets as well as IDs, names, and order. This verifies selection
behavior, not execution of resource acquisition at the adapted boundaries.

The native runner verifies the executable identity, PE mappings, and every
loaded function digest before executing bytes. Original assets and code
bytes are read in place and never stored in tracked files.
