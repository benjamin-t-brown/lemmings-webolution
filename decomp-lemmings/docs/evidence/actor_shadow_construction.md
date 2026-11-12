# Actor shadow construction

## Identity and scope

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The executable remains outside the repository. Exact instruction bytes, addresses,
PE offsets and SHA-256 digests are in [the byte ledger](actor_shadow_construction.asm).

Recovered entry: `0x457F40`, 557 bytes, PE offset 360256, SHA-256
`c1930a02a6306f4e888904a7996871933e77d195934a375d63461431f4aa72ae`.
REA evidence: `ev_b605c91f48f52b00453ef39d4fb1aeb2fabe1d7736e0cf14ebb9cb3663925c2c`.
Implementation: `initialize_actor_shadow` in `src/actor_shadow_construction.cpp`.
The logical `ActorShadow` view is not a binary layout or a claim about the
original class name.

## Static findings

The constructor invokes array construction for twelve 24-byte vertex entries
with `0x4011A0` and `0x401B80`. The element constructor performs no writes.
It constructs five 128-byte quad groups using `0x459940 -> 0x459960`, which
invokes `0x401B80` and constructs four corner entries through
`0x4400B0 -> 0x4821D0`. These element constructors also perform no writes.
Ghidra's library identification as `CSize::CSize` is not used to infer a class
or dimensions; the instructions establish only no-op construction.

The supported writes, in execution order, are:

1. Zero the three doubles at `+0x3B0` through `0x4066A0`.
2. Set byte `+0x3AC` to zero. A temporary `(0,1,0)` vector is constructed
   through `0x4017F0` and copied only between stack locals.
3. For each group `q=0..4`, starting at `+0x128+q*0x80`:
   - Set corner indices to `[2q, 2q+1, 2q+3, 2q+2]` through `0x4135A0`.
     Indices are at group `+0x20+corner*0x18`.
   - Set UV pairs to `[(0,0),(0,1),(1,1),(1,0)]` through `0x413560`,
     at group `+0x28+corner*0x18`.
   - Set three color doubles at group `+0x00` to 255 through `0x4135C0`.
   - Set alpha at group `+0x18` to 255 through `0x459B00`.
4. Set integer `+0x120` to zero and copy the supplied actor pointer to `+0x3A8`.
5. Return the input object pointer.

The twelve vertices, bytes `+0x124..127`, each corner's four bytes after its
index, visibility byte `+0x3AD`, bytes `+0x3AE..3AF`, and intensity at
`+0x3C8` are preserved. There is no actor dereference, allocation, resource
lookup or callback in this chain. The public initializer accepts null and
preserves all preexisting logical fields not written above. Default member
initializers in the logical view remain caller conveniences; they are not
attributed to this original constructor.

Helper evidence IDs:

| Routine | REA evidence |
|---|---|
| `0x4011A0` | `ev_82a54b24268228063f5645e24ed7e2971f32e43d6f5c3983211ac7d1ed12b325` |
| `0x459940` | `ev_32e24d1e5fa328576b06f9b68f645155d4e76c6111924b8ff48e5d760977d9a4` |
| `0x459960` | `ev_6572a3a746e823f41fb7539a4c2e4ebbdddc749cd8638ba4ff56875cebeed616` |
| `0x4400B0` | `ev_8b404965ef1865cd711a3879ee99325ec5b703b4ba24f098d56ed0cb363da2ce` |
| `0x4821D0` | `ev_8aaa4c9bd8448c0e0581f99bf9d7691de537909ca0c09c64d07f819c6e039683` |
| `0x4135A0` | `ev_7bf913b80db329a77c833bd88a45d9ad18a19f6fd75a2fad874c296d62799a29` |
| `0x4135C0` | `ev_d3ecda2bc61d6941bff55f0f2fede806cd07ddfd409fe3e4fbc265ce7e467b78` |
| `0x459B00` | `ev_695389f3a2814d650aa0341d21e16d0e342952cb95a535deca4b9d0c69a694c6` |

## Verification

- **Static:** REA pseudocode cross-checked against the original instructions;
  all thirteen routine spans used by the native chain are pinned in the byte
  ledger and Python runner. The runner verifies the complete executable first.
- **Unit:** `test_actor_shadow_construction` passed x64 `-O2` and x86 `-O0`.
  It checks exact topology, UVs, colors, alpha, returned reference, zero position,
  null actor support, repeated initialization, and preservation of nonzero
  visibility/intensity and vertex NaN payloads.
- **Native comparison:** `native_actor_shadow_construction_oracle` passed
  **10,000 complete object and guard comparisons at each of x86 `-O0` and `-O2`**.
  It executes the unchanged original constructor and its complete normal call
  chain, with **no adapters or patched routines**. Inputs include all 256
  uniform byte fills and 9,744 deterministic random byte fills, arbitrary
  floating representations, null actor values and arbitrary pointer bit
  patterns. Every byte of the `0x3CC`-byte object plus eight guard bytes on each
  side is compared, as is the returned pointer. No NaN normalization is used.

The recovered C++ omits dead stack temporaries and compiler SEH bookkeeping.
Exception unwinding after invalid memory accesses is not claimed. Helper
behavior is validated here for the constructor's calls, not all possible
standalone argument combinations. This recovers construction of the effect
view; it does not establish rendering or a playable game.

Example after building the x86 oracle:

```powershell
python tools/run_native_actor_shadow_construction_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_actor_shadow_construction_oracle.exe
```
