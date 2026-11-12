# Native entity serialization comparison

## Reference and scope

Reference: the installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No executable bytes or level assets are saved in this repository. The runner
verifies the complete executable and each selected routine, derives file
offsets from the PE section table, then sends bytes to an owned 32-bit process.

The compared routines are the 44-byte shared record reader `0x00423291`, the
110-byte actor reader `0x0042FB5F`, and its 111-byte subclass `0x004265E8`.
Their bodies execute unchanged at their original addresses. This is an offline
native-byte comparison with adapters, not execution of the complete game.

Static REA records:

- Shared reader: `ev_0429dff3003b1602c23d32b7339eb475acac2535383a921586a89caf6fa170fd`.
- Actor reader: `ev_df4d75cf8f6f9c295d5752a9e8a66b006a710886d6e58766eeca7e7e47cca5ab`.
- Subclass: `ev_712d7d4dab52667632dd1c0384b665bf935b526506dfa9b5ded4d9cc43851e11`.

Local `objdump -d -Mintel` of these exact address ranges corroborated calling
conventions and the calls listed below. This inspection also covered the two
small helpers not previously needed by native adapters:

| VA | File offset | Length | SHA-256 |
|---|---:|---:|---|
| `0x00423360` | 144224 | 31 | `d7bd98a68723f3516226dd2566681c2455b28a0642c9bccc1005665faadfee3a` |
| `0x00423380` | 144256 | 25 | `66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be` |

The runner pins these helpers locally; the main `verify_reference.py` ledger
pins the three readers and the remaining scalar/vector helpers. `0x00423360`
reads a 16-bit value through `0x0047DAC8`. `0x00423380` computes a registry
element address from its data pointer at offset `+0xC` and the supplied index.

## Controlled environment

The original helpers `0x00402540`, `0x00402560`, `0x00402580`, `0x00423360`,
`0x00423380`, `0x0047DAC8`, `0x00480BB0`, and `0x004821DE` also execute unchanged.
The stream's low-level read at `0x0047EEF7` is replaced with a bounded memory
copy. All tested records are complete; original behavior on short reads is
not claimed.

The shared reader increments the actual mapped 16-bit counter at `0x004C55EC`
and stores the destination pointer through the original registry accessor at
`0x004CBB18`. A 65,536-element backing allocation, centered at index 32,768,
makes all signed 16-bit indices valid for this test. This permits checking
negative IDs without claiming negative IDs are valid in an installed level.

The following non-stream call boundaries have explicit adapters:

| Address | Adapter behavior |
|---|---|
| `0x0049C1A8` | Counts allocation calls; alternates null and stable local storage |
| `0x00415FA0`, `0x0040BAC0` | Count constructor calls and return their input pointer |
| `0x00430200` | Counts the auxiliary reset call; does not modify the object |
| `0x0043B517` | Counts the terrain registration call; does not modify the object |
| `0x0042E696` | Counts the subclass attachment call; does not modify the object |

These adapters isolate serialized reads and immediate stores. The test does
not establish original allocation, constructors, auxiliary reset, terrain
registration, attachment, exception handling, or resulting game behavior.
No exceptions are raised through the original MSVC exception frames.

## Floating-point distinction

The source `ActorRecord` retains serialized binary64 bit patterns. The original
reader loads each value through x87 `FLD` and stores through `FSTP`. With the
test process's masked floating-point exceptions, signaling NaNs become quiet
NaNs. The oracle therefore explicitly projects each source bit pattern by
setting the quiet bit when the value is a NaN, and compares that projected
value with native memory. It preserves and compares sign and payload bits.
For finite values, signed zero, infinity, and already quiet NaNs, the projection
does not change the source value.

This distinction is intentional: the current API describes the file, not a
fully constructed runtime object. The comparison does not claim raw signaling
NaN encodings survive the original runtime reader unchanged.

## Results

On 2026-10-28, `native_entity_base_oracle` passed **30,000 comparisons**:
10,000 cases for each reader. The source side uses the public
`read_level_entity_prefix` API with empty preliminary tables and one actor.
For the shared reader, its first 44 serialized bytes and corresponding source
fields are compared independently of the actor extension.

The deterministic corpus checks:

- All 256 byte values through each Boolean field, including values above 127.
- Signed-ID edges `0`, `32767`, `-32768`, `-1`, plus randomized IDs.
- Counter wrap from `65535` to `0`, plus randomized initial counters.
- Positive and negative zero, subnormals, smallest normal and largest finite
  values, infinities, quiet and signaling NaNs, and random binary64 encodings.
- Random 32-bit fields, cursor positions 44/110/111, every byte of the complete
  destination buffer, and preservation of untouched fields.
- Registry pointer writes and the expected number of adapted calls, including
  both allocation branches and both subclass-flag branches.

Command (32-bit executable built with image base `0x10000000`):

```powershell
python tools/run_native_entity_base_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_entity_base_oracle.exe
```

Output:

```text
native entity readers passed 30000 comparisons (10000 each for 44/110/111-byte records)
```
