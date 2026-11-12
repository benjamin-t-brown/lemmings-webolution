# Complete terrain blast orchestration

## Identity and scope

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The executable remains outside the repository.

Entry `0x43F510`, 1,308 bytes, PE offset 259344, SHA-256
`a45ce7e60823fc491696e2756b7f7c5966e5c139a60bbfe169b57c140c3f4c07`.
REA evidence `ev_a51d28613e7e953a8938c301e918c2f72ca1f1b59a071a0e6a55dcd3e3a51785`.
[The byte ledger](terrain_blast.asm) includes the complete routine, concrete
helper spans and data constants. `apply_terrain_blast` in
`src/terrain_blast.cpp` composes the previously recovered terrain mask loop
with the original effect and entity ordering.

## Static findings

Mode zero selects the 7×7 byte mask at `0x4B1528`; every nonzero mode selects
the 9×9 mask at `0x4B155C`. Initial Y and X subtract half the width in x87
extended precision, store binary64 arguments, then respectively clamp/wrap
and truncate to integers. Loop coordinates stay anchored at these initial
values; effect callbacks use the live caller position.

The existing `clear_terrain_blast_cells` implements the concrete loop:

- Apply only nonzero mask bytes.
- Preserve cells with any `0xA02` bits (`2`, `0x200`, `0x800`).
- For an eligible cell with bit 1, create debris when both loop coordinates
  are divisible by four. The original signed remainder-by-four tests have
  the same zero condition as the recovered low-bit tests.
- Clear the selected cell **after** the entire debris/entity sequence returns.
  A callback mutation of that cell is therefore overwritten by the clear.
- Increment X modulo 256, increment Y, and stop rows on equality with height.

At each debris site the routine:

1. Allocates `0x748` bytes and constructs through `0x40C960`, passing the
   live caller position, not the grid site's coordinates.
2. Calls `0x40C920` with binary64 `0.01`, virtual slot `+0x3C` with zero,
   and `0x40DBF0` with count 4.
3. Copies the current position, adds 2 to Z then 10 to Y, with a binary64
   store after each addition.
4. Maps the live position through `0x43D56C`, then maps the offset copy.
   Subtracts the first mapped result from the second through `0x401830`
   and passes the vector to virtual slot `+0x54`.
5. Allocates a `0x48`-byte entity, constructs it through `0x432080` with
   live position, effect handle and flag 1, then inserts the entity by
   reference through `0x401890` into list `0x4C88A8`.

After all mask rows, the routine calls `0x43B0A1` with the initial X/Y and
unclamped endpoint `(firstX+width, firstY+width)`. It then always constructs
the final particle effect, even when the grid contained no debris sites:

1. Copy the current caller position; add 40 to Y, then 10 to Z.
2. Allocate `0x158` bytes and construct through `0x402870`.
3. Set color `(255,32,32)` through `0x43FA30` on effect subobject `+0xB0`.
4. Set count 100 through `0x402CE0`, position through virtual slot `+8`,
   and pair `(2,0.5)` through `0x402D80`.
5. Map live position first, then the earlier offset copy; set their
   difference through virtual slot `+0x54`.
6. Set flag 1 through `0x403E10`, mode 7 through `0x402DC0`, and advance
   virtual slot `+0x2C` with the original elapsed argument.
7. Construct/insert a final entity through the same entity sequence.

Entity allocation failure yields null, which is still inserted. Effect
allocation failure is followed by unguarded effect calls in the original;
valid effect handles are a public API precondition. Factory callbacks combine
allocation with the corresponding constructor and retain the caller-visible
boundary. Insertion receives a mutable reference to the local entity pointer.
The implementation does not infer rendering or sound behavior from these calls.

## Verification

- **Static:** Full REA pseudocode checked against the original instructions,
  including x87 argument stores, original callback order, color subobject
  offset, and mutable local pointers. The runner verifies the complete
  executable, routine/helper digests, masks and numeric constants.
- **Unit:** `test_terrain_blast` passed x64 `-O2` and x86 `-O0`. It checks
  an ordinary small blast's clearing, debris/entity creation and rectangle;
  protected cells; nonzero negative mode selecting the large mask; final
  particles without debris; and insertion of null entities.
- **Native:** `native_terrain_blast_oracle` passed **4,096 complete routine
  traces at each of x86 `-O0` and `-O2`**, including **2,840 debris sites**
  per run. The original `0x43F510` executes through its normal return without
  an early-exit patch. Original wrapping, clamping, grid lookup, dimension
  access, integer conversion, vector construction/subtraction and no-op
  destructors execute unchanged. Effects, world mapping, factories, entity
  insertion and rectangle notification use explicit adapters.

Cases cover heights 1–16; small/large masks with positive and negative mode;
negative X, wrap seams and out-of-range Y; adjacent binary64 values around
half-width and wrap boundaries; all 128 mutation-script combinations;
protected/ordinary/empty terrain; and successful/null entity construction.
Callbacks mutate caller X/Y/Z, grid cells before the delayed clear, and the
entity pointer passed to insertion. A nonidentity world-mapping adapter also
mutates the live position, exposing map order and offset-copy timing.

The comparison checks ordered callback kinds/objects/arguments and position
snapshots, a grid hash at each callback, final cell arrays exactly, final
position bits and object counts. Allocations and constructors are paired
adapters for each public factory boundary; the original allocation-success
and entity-null branches execute. This is a full core control/data comparison
with synthetic effect boundaries, not execution of the real renderer.

## Limits

The public domain requires positive stable height, stable valid cell storage,
finite XY with wrapped/clamped values in signed32 conversion range, and valid
effect handles. Position and cell contents may change during callbacks.
Replacing cell storage, changing height during traversal, failed effect
construction, callback exceptions, FP exception flags and compiler SEH
unwinding are outside the claim. World mapping and effect implementations
remain callback boundaries, although a separately recovered world-coordinate
mapper can be used by callers. This is not a playable game claim.

```powershell
python tools/run_native_terrain_blast_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_terrain_blast_oracle.exe
```
