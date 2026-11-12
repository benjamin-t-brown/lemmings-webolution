# Climbing entry, timing, collision and update

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The runner verifies the executable, every copied procedure and every constant.
No original game content is committed.
Full original instruction bytes are recorded as text in
[actor_climbing.asm](actor_climbing.asm); no executable or asset is retained.

| Routine | VA / file offset | Bytes | SHA-256 |
|---|---|---|---|
| Constructor evidence | `0x413750` / `0x13750` | 55 | `35c7883489ced993577b3a1b5458ce6a5e8002e955fe538d17cf09c190b53f3b` |
| Entry | `0x4137A3` / `0x137A3` | 26 | `762efafb3ef0779159d4b79982c59fbc9025ee40f096341e11f8a2083ba3609c` |
| Update | `0x4137CA` / `0x137CA` | 346 | `0f6b9ef825ea04454fd9fc8e5d46dbac3001069b7c2f67a0bf6a8a51ff25f8d6` |
| Boundary time | `0x413924` / `0x13924` | 155 | `d98558c3558d0206d7e4876b80c309bb202a10d1180e62bcd2c468af4cfbe984` |
| Collision | `0x4139BF` / `0x139BF` | 615 | `7c9f158b967a796755cb2c142ea292c794b3a839746e9c254464dc4c189d3069` |
| Sample +350 | `0x42F35F` / `0x2F35F` | 149 | `030bc891f4fdb30afd4e9b0585aa25e9595eaf36a05c2c657942554beda300bd` |
| Topping constructor evidence | `0x417C80` / `0x17C80` | 48 | `c4a6045c0b25e3464c8c09698b888785bbf81fda93192af4ea6cbdddf39c0b6c` |

REA evidence IDs:

- Entry: `ev_927f3092c7512d3a8b8781513819c84383095b3c5ccc2e09b1d953e2f19dbb80`
- Update: `ev_320314e632cdb3ff9a3065667677d28d3395c61ef25cf834e2e93597c7e03324`
- Timing: `ev_1c529ea849bc5c1755f535b42d0108fd7c0a4a80dc6e352c29dc80e486d349c5`
- Collision: `ev_b10541f238a84598d6d9fbf927b9eba6e9e228e44f888828bc89845d6488c848`
- Sample: `ev_04b167ef015fdbcbd7516aa145d469a63d32abbbb8e2ca4eda3d5eebec3d117b`

Local disassembly establishes the x87 conditions, exact full-width model time
argument, integer wrapping, procedure extents and constructor fields.

## Static behavior

Vtable `4B0960` has update `4137CA`, entry `4137A3`, and empty leave `4137BD`.
The constructor establishes type zero, extra-state flag `+11 = 1` and update
phase `+10 = 1`. It does **not** initialize timing flag `+18`. The typed view's
zero default is an explicit fixture choice, not a claim about original heap
contents; the native fixture varies that input byte.

Entry selects `climb`. Timing is the next integer Y boundary divided by five,
with signed 32-bit conversion/wrapping and direction exactly one selecting the
forward path. A nonzero timing flag replaces zero or unordered timing by 0.2.
Mode three halves the time budget. Each step adds direction times five times
step to Y, clamps it, advances the model, subtracts step from remaining time,
then at a boundary logs position and runs collision. Unlike walking, this loop
does not truncate actor Y again before collision. It writes timing flag one
after collision (including after callbacks), or zero for a partial step. A zero
result forces remaining time to positive zero. No progress guard is added.

Collision samples position plus `pair_350`, retaining Z. It first probes the
cell at sample X minus horizontal direction and sample Y, with the original
wrap/truncate/clamp sequence. A nonzero cell or truncated sample Y strictly
greater than grid height makes the actor step back in X, log the head hit,
reverse horizontal direction and attach a newly allocated falling state
(`415FA0`, type `0x13`, size `0x28`). The Y comparison is strict: equality with
height does not alone select this branch.

Otherwise it probes original sample X and truncated sample Y minus vertical
direction. An empty cell logs topping out and position, creates and attaches
`417C80` (type `0x1E`, size `0x18`), then logs completion. A nonempty cell returns
zero to keep climbing. All nonzero terrain values count as occupied here.

## Verification and effect boundaries

Unit tests cover entry selection, exact boundary steps, topping log/attach
ordering, post-attach flag overwrite, head-hit X step and direction offsets,
the continue path, zero/NaN boundary replacement and NaN budget reset.
The x64 and x86 tests pass. Timing assertions compare stored binary64 bits to
avoid x86 excess precision in decimal comparison literals.

The native oracle executes entry, update, collision, timing, sample, both
transition constructors, constructor base, coordinate/grid helpers and
direction reversal without modifying those routines. Adapters replace only
animation selection, model advancement, logging, allocation and state attach.
The allocator returns owned memory and the original constructors execute;
attach verifies their type, phase and extra-state fields. Scripted animation
can reverse vertical direction; scripted attachment can change actor X and
the timing flag to verify subsequent reads and overwrite order.

21,568 cases compare complete ordered traces, exact coordinates and remaining
time, sampled points, boundary time, directions, sample offsets, return values,
state flag and absence of other writes. They include negative/zero/NaN time,
NaN Y, both directions, clamping, empty/occupied/unrelated terrain masks,
fractional offsets, raw timing bytes, random grids and independent collision
calls. Another 72 helper-only cases cover infinities, NaNs and signed 32/64-bit
conversion limits. O0 and O2 native runs pass. Original constructor execution
also confirms that an initial sentinel byte at `+18` remains untouched.

Collision X inputs remain finite and within a terminating wrap domain; native
CRT exceptional absolute-value handling is not loaded here. These comparisons
establish climbing behavior with explicit animation/creation/attachment effects,
not the full renderer or the attached falling/topping state's own update.

Audit on 2026-10-29: all nine procedure byte listings in `actor_climbing.asm`
match the pinned executable byte for byte. Fresh x64/x86 unit builds and both
native optimization variants passed. The `+350` sample helper is shared in
`actor_geometry.cpp`; its native evidence remains included in this call chain.
