# Installed `.box` archive index

The archive layout is supported by both the installed assets and the
executable's loader. No complete game asset files are stored in this
repository.

`0x0047E30C` opens an archive path, reads six bytes, and compares them
with `LEMBOX` (98-byte body at file offset 516876, SHA-256
`30a3c4504482ff81fb7624c95be9d5a65fe7ad74a29d161d008eec64d25b94c2`,
REA `ev_93e5b42caefbe6a735fbee34cc5dab3cd78964c0b043ca4d83bb6870911d5e08`).
`0x004805B8` reads the index after opening the archive: entry count,
name-table byte length, each length-prefixed name, then two integer
arrays (497 bytes at offset 525752, SHA-256
`d01bb208d306fd11e6cc4fb2eba044b707b83a58c66ee431acc792cd5bff024a`,
REA `ev_1fbc076f0fe1cd86b3f461ad75b4e3aa7f8e7d025c554e751b96f37d8b0abc1`).
Its array reader `0x0043B640` reads a count and then that many four-byte
values (89 bytes at offset 243264, SHA-256
`5c6afd69245b0ad2c4fd7a58fd31bc29ae65a0077ae00f0f2ba3099d22f58ac3`,
REA `ev_b9be9816c3b061861ac67034b52ee9057b3e3c6ce161efe114fb123f4c55c98f`).
The verifier pins these source bytes.

Both `lemmings.box` (322,893,635 bytes) and `patch.box` (8,533,284 bytes)
start with ASCII `LEMBOX`, then little-endian 32-bit entry count and name
table byte length. The name table contains a 32-bit byte length followed
by that many bytes for each name. After the name table are a repeated
32-bit count, one 32-bit absolute file offset per entry, another repeated
count, and one 32-bit byte size per entry. The data begins immediately
after these tables. `tools/inspect_box.py` checks all entry bounds and
lists or hashes entries without extracting them.
`read_box_archive_index` is the C++ port of this logical index layout;
the original's allocation layout is outside its claim.

The installed `lemmings.box` contains 10,661 entries, including 102
`COMPILED\*.LVL` entries. `patch.box` contains 311 entries, including
three `COMPILED\*.LVL` entries. All 105 observed level entries start
with `08 00 00 00` followed by ASCII `Level 23`, matching the loader's
length-prefixed marker read at `0x00443318`. This confirms the marker
against stored level data, but does not identify later fields.

For a repeatable example, `COMPILED\MM_CLIMB_002.LVL` in `lemmings.box`
starts at archive offset 47,132,210 and occupies 699,117 bytes. Its
SHA-256 is `95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567`.
Its first 48 bytes are:

```text
08 00 00 00 4c 65 76 65 6c 20 32 33 63 00 00 00
92 c6 12 00 92 c6 2d 00 00 b0 01 00 00 b0 01 00
4a 28 07 00 01 00 00 00 63 00 00 00 09 00 00 00
```

These bytes supply a fixture for later decoding of `0x004432C1`; their
field meaning remains undetermined. Across all 105 observed `.LVL`
headers, the third 32-bit value exceeds the second by exactly 1,769,472,
and the fourth and fifth values are equal. The first value ranges from
90 through 129; the loader multiplies it by 58 (`0x3A`). These patterns
are observations, not field definitions. The archives have 299 overlapping
entry names.

`0x0047E23B` registers `PATCH.BOX` and then `LEMMINGS.BOX` from the
working directory, with a possible third `LEMMINGS.BOX` from the CD path
(184 bytes at offset 516667, SHA-256
`39e5410639bff1f5232ae1a9d1af1fbf75cc763f0412b1315ebb6add088b899b`,
REA `ev_dd9fbd075ad0ad9562db61c98f0ec8f9b57bac3cd9369c9ea5509ef52b2689a4`).
`0x0047E069` appends each present archive to the registry (156 bytes at
offset 516201, SHA-256
`c426999a1019d4fd192c9578944d96f2529c8a58d23d5191774d91697a68cec7`,
REA `ev_ab8eb2c07127d712294816fadf1f53758f642401990e2c9ebe82024a7b9028a3`).
`0x0047F759` scans archives and then their entries from index zero, stopping
at the first matching name. It removes a drive prefix through the character
after the first colon, uppercases the requested name, and compares with
`strcmp` (647 bytes at offset 522073, SHA-256
`3030c16fdbab94490af0899b4c0595a047b1f85cf8fe680e178196c9109f85b6`,
REA `ev_d4f12f0577373b7f85d33eeba4141a9397b502aef81bcbfe9edda372d3aecdb7`).
The uppercase helper is `0x0049E274` (204 bytes at offset 647796,
SHA-256 `57f975fd17367c02ad55f1eef7d4702160f5c37285816b111aa72a199af8274a`,
REA `ev_a33576e03e01f60e882ea027da3b1368e3b86ba56cb1676b95ed8a4a516cae47`).
The C++ lookup models the ASCII path used by installed entry names;
locale-specific uppercase behavior and CD registration remain unported.

With the two user-owned archives supplied at CMake configuration time,
the optional installed-fixture test compared every name, offset, and size
for all 10,972 entries against the C++ index reader on x64 and x86. It
also compared a sampled level payload from each archive byte for byte.
The C++ lookup selected patch entries for all 299 overlapping names and
handled base-only, patch-only, mixed-case, drive-prefixed, and missing names.
These are fixture comparisons and deductions from static control flow; the
original loader was not executed.
