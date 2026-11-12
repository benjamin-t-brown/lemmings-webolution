# Spawned actor monitor, behavior 0x2F

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The actor constructor attaches this extra behavior. The name describes that
observed context; terrain bits and region/state types are not given speculative
gameplay names.

## Static recovery

`0x4151F0` calls the common behavior constructor, then an empty pair constructor
at `0x4821D0`. It sets phase `+10=1`, extra `+11=1`, and type `+14=0x2F`.
Elapsed `+08` is initialized by the common constructor; the pair at `+18` is
left untouched. Entry `0x409310` and leave `0x401320` are empty. The recovered
update does not read the behavior's pair or elapsed fields.

`0x41527B` performs these operations in order:

1. Call `0x42F183`, which truncates the actor's X and Y independently through
   x87 signed64 conversion, keeps each low32 interpreted as signed, adds the
   corresponding `pair_360` component, and stores binary64. Z is copied.
   This differs from `0x42F22E`, which adds the pair to the untruncated position.
2. Truncate the sampled X, call the cylinder wrap, and truncate again. Truncate
   sampled Y, subtract direction `+44` with 32-bit wrapping, clamp Y, and
   truncate again. The sample coordinates are captured before later callbacks.
3. Write positive zero to remaining time, then call `0x42F6EC`. A false low
   return byte returns result 2 immediately. The source exposes this boundary
   as `can_continue_42f6ec`; it can be composed with the separately recovered
   `actor_sample_y_in_world` from `actor_terrain_probe.hpp`.
4. Read the collision cell. Bit `0x40` takes precedence over bit `0x100`.
   For `0x40`, search the region collection at `0x4C89D8`, querying its count
   on each iteration and stopping at the first successful `0x420A52` call.
   Each point passed by value is `(sampleX, sampleY, 0)`.
5. If a region matched and state `0x2E` is absent, construct `0x406C20` and
   attach it. If no region matched and state `0x22` is absent, clear the primary
   behavior through `0x42E725`, construct `0x408700`, and attach it. Return 0.
6. With no `0x40` but with `0x100`, compare sampled XY with actor pair `+68`
   using `0x415800`. This uses x87 equality bits: unordered components count
   as equal, and the result is true only if a component is ordered unequal.
   When different, allocate/construct a `0x20`-byte request through `0x415710`,
   passing global `0x4CBF88 + 0x190`. Set request pair `+10`, enqueue via
   `0x43F0C0`, copy the **local** pair into actor `+68`, and set byte `+5F=1`.

The request callback returns a portable `SpawnMonitorRequest` with its known
pair field. The source does not reconstruct the scheduler base or the request
constructor's unknown fields. The request must be nonnull: the original writes
through the pointer immediately. State constructors, region tests, primary
cleanup, attachment, and request scheduling remain explicit effect boundaries.

The request update `0x41565C` is recovered separately: truncate its pair, clear
the corresponding collision cell, notify `0x43B0A1(x,y,x,y)`, and return zero.
The cell accessor wraps X to eight bits and clamps Y; notification receives
the original truncated coordinates. The native fixture executes this body too.

## Byte evidence

Complete original instruction bytes and disassembly are retained in
[actor_spawn_monitor.asm](actor_spawn_monitor.asm). The native runner's `PINS`
contains the verified VA, PE-mapped offset, byte extent, and SHA-256 for all
12 new bodies. Existing scalar/coordinate helpers are loaded using the central
verifier's pins. Constants for zero, cylinder wrap epsilon, and 256 are checked
against original bytes before native execution.

| Routine | VA | Bytes | REA evidence |
| --- | --- | ---: | --- |
| Constructor | `0x4151F0` | 111 | `ev_40dfb118a0db7f16d0ff35f6a258aece823152cad353a26114b4a61a568d6e88` |
| Update | `0x41527B` | 993 | `ev_8dcf1a911e0789c346cd1617e7f3f3f7a7323de27bddc7eddfaaa778caa1e109` |
| Empty entry | `0x409310` | 13 | `ev_fc374ac8a649d9053622c7df032789074c249e092ce169a1778257337068dc4c` |
| Empty leave | `0x401320` | 13 | `ev_29a685327ef06f5aa338a285c442dd2ebe9d3ed2ce8cacf30e96529bab488d3f` |
| Integer-position sample | `0x42F183` | 171 | `ev_b82f347ff633cb0c59e6bb4aa2e9eedb1919460bb2462cf51a3dd1e002254832` |
| Pair comparison | `0x415800` | 70 | `ev_227cfb4e0b2e8bbb27645d82a2d39eb460bb8b1942f32e71e6633b62295cb7e7` |
| Deferred clear update | `0x41565C` | 119 | `ev_4285f1cfe46ebafb7f4906870f648c23ee50da90e74108a311fc2e1971a7f6a8` |

Additional original helpers executed unchanged are the empty pair constructor
`0x4821D0` (14 bytes), pair-value constructor `0x404D40` (45), triple-copy
constructor `0x4066D0` (36), empty triple constructor `0x401B80` (14), and
triple assignment `0x406700` (68). The last two were verified directly from
local instruction bytes and disassembly.

## Unit and native verification

Unit tests pass on x64 and x86. They cover constructor metadata and untouched
pair fields, fractional-position truncation, unordered comparisons, both
region outcomes, existing-state suppression, bit precedence, callback order,
request enqueue mutation, cached-pair suppression, gate result 2, and deferred
grid clearing with notification coordinates.

The x86 native oracle passes at both **O0 and O2**:

- **20,000** monitor updates with ordered callback traces and complete mapped
  actor bytes compared. Cases include both bit branches, no bits, existing
  states, first-match region search, gate mutation of actor/time after sampling,
  last-pair NaNs, wrapping direction subtraction, and mutation of the queued
  request before the actor's cached pair is updated.
- **20,000** complete constructor byte comparisons, empty entry/leave calls,
  pair comparisons, and deferred clear updates. Unknown constructor bytes
  remain the original sentinel value; grid contents and notification traces
  match after deferred updates.
- **20,121** sample-helper comparisons, including fractional coordinates and
  isolated signed32/signed64 bounds, low32 wrapping, infinities and NaNs.

The main monitor scenarios use finite positions with bounded X wrapping;
extreme sample-helper cases are tested separately. NaN values in sample
comparisons are compared as a class; exceptional FPU status is not claimed.
Original constructor/update/sample/comparison/deferred-clear bodies execute
unchanged. The range gate, region collection/test operations, state effects,
allocation, request construction/queue, and terrain notification use explicit
adapters. These tests do not establish full game runtime or those adapted
implementations.
