# Resource slot registration

Authority: installed `Lemmings Revolution.exe` SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The executable identity was independently confirmed when opening the child REA
session. [REA records](resource_registration_metadata.json) retain function
identities, evidence IDs, bodies, pseudocode, callees and limitations. They are
static observations, not recovered source. Complete returned evidence is also
retained locally in `build/resource_registration_full_evidence.json`.
[Original byte disassembly](resource_registration.asm) corroborates all writes,
branches and call order.

| Routine/span | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Registration | `0x446040` / `0x46040` | 312 | `74cf544a3840fb929bfc3ab58fea77a73847fb8e67ffa03b689c331d3767435f` |
| Named entry construction | `0x449170` / `0x49170` | 51 | `a3a639d2e2b30828ddefe7022bd134452b827c4b38a59d6ae9d2d49e8352db70` |
| Array append/growth | `0x449DB0` / `0x49DB0` | 363 | `1dd4eaf471b7356df2af50e9e6bb1b88df4b441504fcca1ddae49b44fa5ce8ca` |
| Entry assignment | `0x448F50` / `0x48F50` | 64 | `a11452cb9eb4b8606f13b3de42a75166cc3725424bda3421088dc248db8443ef` |
| String construction | `0x41D990` / `0x1D990` | 34 | `601f9b14fddb7b8835707d6e64f8286deed8105db40677f59e03f2d7469e2f1e` |
| String assignment | `0x403730` / `0x3730` | 29 | `8e80e9e25c15e694e4d6d739cf16c6fc36a6602d56a2280629b070498887476c` |
| String-object assignment | `0x4036F0` / `0x36F0` | 33 | `d521bf6126450a43521d6cd9e20dee9184fa73048b0363c5196086931fcbf8e2` |
| Entry deleting destructor | `0x44BA30` / `0x4BA30` | 115 | `5827cf1987a02e0cb1ea9bcc7387a551f88db5acdd97c04aa573d5fb21e89b91` |
| String data accessor | `0x403720` / `0x3720` | 14 | `befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c` |
| CRT string-copy execution span | `0x49C750` / `0x9C750` | 240 | `de277c53bfccd0f688de14be2f3a7258615d8c770e4bd3f7cc88a8e29afb91b9` |
| Default entry construction | `0x44BE60` / `0x4BE60` | 32 | `b396891f78951760185388c076c55d644a0b91531e86d08d9e53bd8713f25701` |
| Empty entry destructor | `0x4491B0` / `0x491B0` | 19 | `57b58fd6b21542d7a72f5cffde310144904f79a92bb46025d7650143a817bc13` |
| Count accessor | `0x406640` / `0x6640` | 16 | `8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936` |

The main REA records are registration
`ev_bdbb18b00a01c4ad76a18856ba8e1c6147e3cea750c6140388c217dda79cb54a`
and array growth
`ev_252d5e26ed79b4296a9e63b19577860609f3709f7445ec40c4f71e2fbd45221b`.
Ghidra reports only the 7-byte entry jump as the body of `49C750`. The oracle
instead pins the complete `49C750..49C83F` execution span: the entry jumps to
`49C7C1`, whose branches end at `49C83F`. This span also contains the intervening
`strcat` entry and destination-scan code, which is not called or claimed here.
The local assembly, rather than the truncated function extent, establishes the
loaded byte coverage.

## Static behavior recovered

The manager contains an inline vector header at `+30`: signed count, capacity,
growth increment, then backing storage pointer at `+3C`. Each original entry
is `0x88` bytes: 128 name bytes, a cache pointer at `+80`, and an active byte at
`+84`. The other three bytes at `+85..87` remain explicit unknown bytes.

`register_resource_slot` (`446040`) scans from index zero to the first entry
whose active byte is zero. Occupied entries are not compared by name: duplicate
names can be registered. It constructs a named temporary and assigns it to the
selected inactive entry, or calls `449DB0` to append it. It returns the index
selected before any allocation callback. Temporary destruction is empty.

The entry helpers perform partial writes:

- `44BE60` writes name byte zero and active byte zero, leaving the cache pointer
  and every other byte untouched.
- `449170` first clears name byte zero, copies the supplied NUL-terminated name,
  then clears the cache pointer and sets active byte one.
- `448F50` copies the name through its NUL, the cache pointer, and only the low
  active byte. Neither the name suffix nor the three unknown bytes are copied.

`append_resource_entry` preserves the original growth sequence:

1. If signed count is at least capacity, add growth to capacity with x86
   32-bit wrap and store it. Capture that new capacity locally.
2. Request `capacity * 0x88 + 4` bytes, with 32-bit size arithmetic. Write the
   captured capacity into the returned array cookie, then default-construct
   that many entries. Mutations of live capacity during allocation do not
   change the cookie or initialization count.
3. Copy entries using the live count and live old backing pointer. The
   allocator can change these fields before copying starts.
4. If the live old backing pointer is nonnull, run its empty entry destructors
   and release the allocation. `44BA30(...,3)` reads the old array cookie and
   destroys entries in reverse order through `49C655`; those destructors make
   no observable writes. The source omits that effect-free iteration.
5. Store the newly allocated backing pointer after release returns. Assign the
   appended entry at the live count, then increment the live count. Changes to
   count or source entry during release therefore affect the append.

When spare capacity exists, no allocation, initialization or release occurs.
When an inactive slot is reused, count does not change.

## Typed projection and explicit boundaries

The implementation reuses `ResourceCacheEntry` and `ResourceCache`.
`ResourceRegistration` references that cache and supplies the three vector
metadata words. It is not an ABI-compatible replacement for the original
manager. The original stride remains `0x88` for allocation-size calculations,
regardless of the host pointer size or `sizeof(ResourceCacheEntry)`.

The allocator hook represents original `49C1A8` and returns typed entry storage
plus a writable cookie pointer. The source performs the cookie write and
partial entry construction. The deallocator hook represents original
`49C13D`; its typed entry pointer identifies the original allocation header at
four bytes before the entries. This translation avoids imposing the original
x86 allocation layout on a 64-bit host. The allocator and deallocator internals
are not recovered in this slice.

Inputs must be NUL-terminated names of at most 127 bytes, with live accessible
source and destination storage and no partial overlap. Initial counts and
array cookies must be nonnegative, and growth/allocation plus any callback
mutations must leave every accessed element valid. Allocation must succeed;
original null allocation is followed by unchecked copying/assignment and is
not converted into a new error behavior. Unbounded original string copies,
corrupt counts, undersized allocations, integer-wrap-induced invalid storage,
exceptional unwind and concurrent mutation are outside the supported contract.
The model preserves defined 32-bit metadata arithmetic but does not claim a
safe recovery strategy for those invalid inputs. C++ member defaults are not
evidence that the original initializes the corresponding bytes.

## Unit and native evidence

Units pass at x64 O0/O2 and x86 O2. They check constructor partial writes,
assignment of the low active byte, duplicate-name registration into the first
inactive slot, no allocation on reuse, captured new capacity despite allocator
mutation, input-name capture before allocation, copying before release, and
live count/source/backing-pointer behavior after release.

At **both x86 O0 and O2**, the native oracle passes:

- **32,768** registration/append comparisons, split equally between complete
  original `446040` and direct original `449DB0` calls.
- **8,192** original `44BE60`, `449170`, or `448F50` entry operations.

The runner validates the executable SHA-256 and each loaded span before native
execution. It runs all listed routines unchanged, plus previously pinned
`4307A0`, `41A490`, `41A4B0`, `49C5D3`, `49C63D`, `49C655`, and `49C6BD`. Only
`49C1A8` and `49C13D` are replaced with controlled allocation/release adapters.
Normal-return SEH setup/restoration runs; exception unwinding is not exercised.

The fixture varies empty through 127-byte names, four source alignments,
non-ASCII name bytes, zero through six entries, spare capacity versus growth,
null old backing storage for empty arrays, first/middle/last inactive slots,
and nonzero active-byte values including their sign bit. Dirty storage checks
that all 128 name bytes, cache pointer bits, and unknown high bytes are preserved
when unwritten. All three allocation groups and their cookies are compared.
Eight allocator/release scripts vary live count, capacity, growth, backing
pointer, input name and direct append source. Traces compare allocation byte
requests, pre-callback metadata, release target identity, new cookie and the
partially initialized/copied entries visible at release.

The release adapter retains backing buffers so the oracle can compare their
post-call contents; it does not emulate heap reuse. Controlled callback
mutations test call ordering, not a claim that the original heap allocator
mutates resource metadata. This is original-byte registration evidence with
allocator boundaries, not a full resource-loading or game-runtime test.

```powershell
python tools/run_native_resource_registration_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_resource_registration_oracle.exe
python tools/run_native_resource_registration_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_resource_registration_oracle_o2.exe
```
