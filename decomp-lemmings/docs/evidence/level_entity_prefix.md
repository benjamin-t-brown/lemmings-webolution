# Level entity serialization

Reference: the SHA-256-pinned installed executable identified in
`tools/verify_reference.py`. This note covers the file reads in
`0x004432C1` immediately after `0x00437D48`.

The loader first reads a four-byte registry allocation count. It then calls:

| Address | Serialized fields |
| --- | --- |
| `0x00447960` / `0x00452612` | Counted groups: flag byte, counted records, counted integers, capacity-128 name |
| `0x004526F0` / `0x00452B2F` | Each group record: double, integer, two pairs of doubles (44 bytes) |
| `0x00447A40` / `0x00420AAE` | Counted name pairs: flag byte, two capacity-64 strings |
| `0x00448490` / `0x0042FB5F` | Counted actor records, 110 serialized bytes each |
| `0x00448550` / `0x004265E8` | Counted subclass records, the same 110 bytes followed by one flag byte |

Both actor readers call `0x00423291` first: XYZ doubles, doubles at object
offsets `+0x20` and `+0x30`, flag bytes at `+0x38` and `+0x28`, and a
signed 16-bit registry ID at `+0x3A`. The remaining actor fields are two
integers, four flags, an integer, two flags, and three double pairs.
Field names retain object offsets where their gameplay meanings are unknown.
Boolean fields normalize any nonzero byte to true. Floating-point fields
retain serialized bits; this does not emulate x87 handling of exceptional
floating-point payloads.

REA evidence includes `ev_df4d75cf8f6f9c295d5752a9e8a66b006a710886d6e58766eeca7e7e47cca5ab`
for `0x0042FB5F`, `ev_712d7d4dab52667632dd1c0384b665bf935b526506dfa9b5ded4d9cc43851e11`
for `0x004265E8`, and `ev_0429dff3003b1602c23d32b7339eb475acac2535383a921586a89caf6fa170fd`
for the shared base. Exact mapped byte ranges and SHA-256 digests of all
eleven readers are recorded and checked by the reference verifier.

The C++ reader and independent Python layout parser match every field hash,
count, and final cursor for all 105 installed levels on x64 and x86.
These are installed-data fixture comparisons, not native execution of the
complete loader. Resource lookup, allocation side effects, registry writes,
and constructed actor state remain unrecovered. Further entity lists follow.
