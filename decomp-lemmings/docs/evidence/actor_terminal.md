# Terminal behavior type 0x1A

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Its exact gameplay name remains unknown. The falling resolver constructs this
type for a terminal transition; that context does not establish a broader name.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Constructor | `0x40B3A0` / `0xB3A0` | 48 | `4d91ce609376caf9a4d1b52e8178680d6a4317cb979143c19204d8fd30d1fa5f` |
| Entry | `0x40B3EC` / `0xB3EC` | 1542 | `bcae2c0c12e932e925309cf5c18207c38cadc831ae92c237711c8d8d8bf7955e` |
| Update | `0x40B9F2` / `0xB9F2` | 104 | `f5737cd3bb0aed25041d2a5462df5d47394decffea281a32467e1fd112c08fdb` |
| Empty leave | `0x40BA60` / `0xBA60` | 13 | `8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a` |
| Constructor base | `0x4010D0` / `0x10D0` | 31 | `38fd038f41a342d0889685b8ddb612588565f0a9b1c5bdfe2a2cdb905e7103de` |
| Behavior base | `0x418390` / `0x18390` | 47 | `254616efe4eeecb6423131c308a44bd17d051f5651e8a0adebbe4d242ca65772` |

Complete original instruction bytes and local disassembly are retained in
[actor_terminal.asm](actor_terminal.asm). The central verifier and native runner
pin their exact extents, PE offsets and hashes. REA static dossiers:

- Entry: `ev_24d3af1900bf0214e435a6937c92e03d428d9ddc9d56c4088f060e57068663af`
- Update: `ev_af3d1a47b3865926382fa5910c4b0061a84a17d2607f1b9cbbe3927a64b551e8`

## Static behavior

Constructor metadata initializes elapsed `+08` to zero, phase `+10` and extra
flag `+11` to zero, and type `+14` to `0x1A`. The source initializer expresses
these fields without assigning a host vtable. Unknown object bytes stay unknown.

Entry copies actor position, then clamps the copy's Y to zero and the world
height. The first x87 test also replaces unordered Y with zero. It forms a
second position by adding the signed, wrapping 32-bit value `direction_44 << 2`
to Y. Actor byte `+28` is cleared before the world event receives the signed
actor ID, current event global `0x4E32E0`, and zero flag. The source receives
the world height and event global explicitly.

After the event, category `+4C == 3` selects effect constructor `0x4072A0`;
other values select `0x4018E0`. Both first effects receive count 6, the clamped
position, pair `(4,0.5)`, mode 5, and the difference between world-transformed
offset and original positions. The original position is transformed first.
The offset X/Y are truncated with the low-32-bit x87 conversion semantics to
sample a triple. Each first-effect child receives that triple. Child count is
queried again on each loop test. The effect is wrapped in an entity and inserted.

The second effect uses `0x402870`, count 10, triple `(255,127,127)`, the offset
position, the same pair/mode/direction calculation, and a second entity whose
position is the original clamped position. Entity allocation failure still
passes null by reference to insertion. Effect allocation failure is outside
the source API's valid-handle domain: the original dereferences it unconditionally.
Actor position itself is not rewritten by the routine.

Update advances the inline actor model by the complete input double, then reads
elapsed and remaining again, accumulates them, clears remaining to positive
zero, and queries model completion. Any nonzero low return byte produces 2;
otherwise it returns 0. Leave has no effect.

## Unit checks and native comparison

Unit checks cover constructor metadata, category mutation by the event, copy
clamping, both transformed-position pairs, child values, null entity insertion,
and update callbacks that change elapsed and remaining before their next reads.
They pass on x64 and x86.

The native oracle executes unchanged entry/update/leave, constructor and base
instructions from the identified executable, with explicit adapters for effect
allocation/construction/setters, world event and coordinate/sample operations,
entity construction/insertion and model advance/completion. It checks 10,000
entries, constructors and updates at both O0 and O2, comparing ordered callback
arguments, actor flag/category and elapsed/remaining/result. Cases include all
effect branches, signed IDs, both directions, shift wrapping, zero through six
children, null entities, zero/negative/infinite/NaN values, and callback mutation.
NaN payloads in callback traces are intentionally projected to one NaN value;
this does not claim exceptional FPU status equivalence.

These are controlled original-byte comparisons with synthetic objects and
explicit graphics/model boundaries. Full game runtime and effect constructors
are not verified by these tests.
