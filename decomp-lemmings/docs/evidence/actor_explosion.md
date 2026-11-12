# Explosion behavior type 2

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

## Static reading

| Routine | Bytes | Evidence |
| --- | ---: | --- |
| `0x4081B0` | 48 | `ev_62510352342841a062909b05180622c7137a87f3910246f04eb92a8f38831265` |
| `0x4081FC` | 43 | `ev_1eb60836ec9ac047eed8a1456a53d8b0bf246a664eff5c1887227abaa0cbee74` |
| `0x408227` | 1193 | `ev_363f91922ae50e4763946b6bbd486e8e98c9412a082f9aaab97c17f188de3516` |

The constructor selects type 2, elapsed zero, phase zero and primary behavior.
Vtable `0x4B0550` contains destructor `0x4086D0`, update `0x408227`, entry
`0x4081FC` and the shared empty leave `0x401320`. Entry sends the signed actor
ID and global event value `0x4E32C0` to `0x44535B` with flag zero.

Update computes the integer-position sample through `0x42F183`, then calls the
nearby effect with radius 3. Only afterward does it copy the live actor position.
It offsets X by 1 and Y by 1.5, calls world notification `0x43B31E`, subtracts
1 from the possibly modified X, then calls terrain blast `0x43F510` with the
current time budget. Integer constants 2 and 3 divided by double 2 supply the
original offsets; those image constants are pinned.

The live actor category chooses effect constructor `0x4072A0` for category 3 or
`0x4018E0` otherwise. Both use allocation size 0x128, count 6, captured position,
pair (4,0), mode 6, direction from the transformed live actor position toward
the transformed captured position, then advancement by the current budget.
The captured position selects a world sample. Every current child receives
that sample, with child count re-read each loop. Finally an entity is constructed
at the captured position, inserted by reference, and remaining time is cleared
to positive zero before returning 2.

The API preserves mutable references for the nearby/world/terrain calls and
reuses the terminal state's effect interface. Unused interface methods are not
called. Effects and child objects must be nonnull; null entity allocation is
passed through to insertion as in the original. Allocation, graphics, terrain,
world notification and nearby effects are explicit dependencies. The separate
terrain and world modules can implement these callbacks; this state does not
claim their internal behavior. Exceptional allocation cleanup is not recovered.

[Assembly](actor_explosion.asm) records all 1,284 constructor/entry/update bytes;
the shared empty leave is pinned elsewhere. The runner and main verifier pin
the complete executable identity, mapped functions and required constants.

## Unit checks

The unit test checks constructor metadata, entry/update/leave, terminal result,
positive-zero remaining time and mutation at each observable callback stage.
Another 114 cases alias remaining time with actor X/Y/Z, either sample offset,
or behavior elapsed storage; null entity and negative child counts are checked.

## Native comparison

20,000 complete constructor/entry/update traces pass at x86 O0 and O2. The
original update, integer-position sampler, subtraction and x87 conversion
helpers run unchanged. Graphics/world/nearby/terrain/allocation dependencies
are adapted and every call argument is compared in order. Complete actor and
behavior buffers and final remaining-time bits are compared too.

Coverage includes both category branches, callback changes to live actor
position/category/time, direct mutation of captured position by world/terrain
calls, changing and negative child counts, null entity allocation, remaining-time
aliases with actor position/sample pairs and behavior elapsed, negative/fractional and
overflowing coordinate conversion, signed zero, quiet NaNs and infinities.
Computed callback NaN arguments are normalized; untouched object/time bits are
compared exactly. Signaling-NaN arithmetic, FPU traps and invalid object lifetimes
are outside the claim. This is offline original-byte execution, not a live-game
rendering or terrain-effect comparison.
