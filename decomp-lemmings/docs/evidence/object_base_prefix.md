# Next object after the world data section

The reference executable is `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
After `0x00439FD3`, the level loader `0x004432C1` calls `0x00437D48`.
That routine first calls `0x004749BF`, which consumes the length-prefixed
`Object 09` marker and a four-byte class tag. The installed levels use
class tag zero. Its constructed vtable at `0x004B19A0` points slot `+0xC`
to `0x0046BEF0`, which delegates to the base reader `0x0046BB73`.
That reader consumes twelve doubles, one capacity-128 string, two more
doubles, and a child count. The first child is tag one in all installed
levels. Its class-one reader consumes two counted arrays of three-double
records and a named reference table before the polygon count and first
polygon header. `read_object_base_prefix` continues through all children,
including every polygon and the following twelve doubles, name, two
doubles, and nested-child count in each child.

The type-five dispatch at `0x00469B49` calls `0x0046A140`, which reads
each record through `0x0046B0D0` and three vertices through `0x00479FAD`.
The type-seven dispatch at `0x00469B8D` calls `0x0046A1E0`, which reads
each record through `0x0046B3D0` and four vertices through `0x0047A077`.
Both vertex readers call `0x0047A130`. On disk a record starts with a
four-byte value; each vertex contains another four-byte value followed
by six eight-byte floating point values. Thus type five consumes 160
bytes per record and type seven consumes 212. The last reader also scales
the first two doubles after loading; the source currently retains their
serialized bits and does not claim the constructed in-memory geometry.

`0x00437D48` is 487 bytes at file offset 228680, SHA-256
`83175d294a7eb14b4210a02894b8907d6d66ad65df75a8f0bdbdd1804c31aee2`
(REA `ev_292a274435735b6956d7261b4af86428882724aff5842e354ba222f72ab2f553`).
`0x0046BEF0` is 41 bytes at offset 442096, SHA-256
`4bad1f613f7c8764a0348c725990ed1f1be67fe0d44ea0dfff35a49a25f48e22`
(REA `ev_f30aba6007244e4a7bda57717ad3008a3ca589a104f6ae94f0250da32cd07940`).
The verifier pins both bodies; the other relevant readers are pinned in
the [level loader note](004432c1.md).
It also pins the dispatch, record, and vertex readers named above.

All 105 installed levels have the `Object 09` marker and class tag zero.
The base name is `null2` in 76 entries, `Spooky_Forest` in 19, and
`thenull` in 10. The child count is three in 102 entries and 25 in three.
The first child uses a `PolyList 09` header with type five in 13 entries
and type seven in 92. Across all 381 children there are 16,552 type-five
and 9,537 type-seven polygons. Every child has zero nested children.
The optional fixture test compares each serialized polygon, the following
fields, and the stream cursor on x64 and x86. `0x004749BF` then reads two
one-byte flags. When the second is set, `0x00474EB0` reads an auxiliary
record count. Both flags are set in all installed levels; the auxiliary
count is zero in 102 entries and one in three. For each auxiliary record,
`0x00476277` reads a double; three counted arrays; a capacity-128 string;
and a counted integer array. The first array's elements contain an integer
and a nested array of 89-byte records (`0x004775B0`, `0x00477B30`,
`0x0047CEB1`). The second array's elements contain an integer, a nested
array of 40-byte records, another integer, and two integer arrays
(`0x0047C93D`, `0x0047CB20`, `0x0047CF5C`). The third array's elements
contain two integers and a double (`0x0047CDE7`). The source consumes
these shapes and preserves their complete serialized bytes; individual
nested values still need named in-memory structures. All three nonempty
records match their installed byte ranges and the following stream cursor.
The next outer flag in `0x00437D48` is set in all 105 installed levels.
The following `Object 09` marker and class tag zero also match every
entry. The second object's first twelve doubles, capacity-128 name, two
doubles, and child count now match all 105 entries. Its base name is
`thenull` in 10, `Spooky_Forest` in 19, and `null2` in 76. Child counts
are 29 in 10, 28 in 19, 85 in 73, and 53 in three. Its complete child
trees contain 5,411 class-one nodes and 1,794 class-two nodes, with 19
nested children. Their polygon lists contain 4,385 type-five and 4,369
type-seven polygons. The second object's two flags are set and its
auxiliary list contains one record in every installed level. Both object
trees and their auxiliary payloads now reach the end of the file-read
path in `0x00437D48`, with matching values and cursors on x64 and x86.
Original object construction, name resolution, and transformed geometry
are still outside this serialized-data recovery.

## Class-two nodes

`0x0046BCA4` dispatches tag two to constructor `0x0047B320`, which sets
the vtable at `0x004B1AD8`. Slot `+0xC` is `0x0047B44C` (REA
`ev_55eda52e1ff707442e5bc6c0c9a894caff3e306ac49c8860232057e084b26325`).
That reader consumes three doubles and two one-byte flags. If the second
flag is clear, it reads a capacity-260 name and four doubles. If set,
`0x00468BF2` reads four integers and a capacity-255 name (REA
`ev_0d4f1a1922ccf70ccbf22eb28782becbf7e9207572401c8ff3011c8d46a1cc10`).
Both branches then read one integer and delegate to the shared base reader
`0x0046BB73`. The installed data exercises the clear branch 398 times
and the set branch 1,396 times. The first flag is set in every case.

`0x0047B44C` has 440 bytes at file offset 504908, SHA-256
`c7a2932fc546e533af18fd4870eb7b07643b1cc0d3e43335c6bf2f85d806418a`.
`0x00468BF2` has 209 bytes at offset 429042, SHA-256
`01704458c72c42234396c393ee82c4689221f8d7bb529a7efca4c6b724ef3f29`.
The verifier checks the PE section mapping for each recorded address and
offset, as well as its byte digest. This executable's code section has
equal RVAs and raw offsets; no extra header bias is added.
