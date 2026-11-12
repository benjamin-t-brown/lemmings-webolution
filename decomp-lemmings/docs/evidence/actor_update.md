# Actor behavior scheduler

## Authority and recovered boundary

Reference: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA pseudocode evidence:
`ev_ec96c00a75e3399667249363fa50fc1e53349e114c14a3049ba6e59a3858dc81`.
Local `objdump -d -Mintel` of the same executable established exact branches,
virtual-call slots, list accesses and jump-table destinations.

`update_actor_behavior_schedule` reconstructs the scheduler within actor
update `0x0042E2BF`, ending at the terminal branch `0x0042E58C`. The subsequent
animation, direction, resource and terrain operations remain a typed
`finalize` callback. This is not a complete actor update implementation.

Exact input-byte pins used by the native runner:

| VA | File offset | Length | SHA-256 |
|---|---:|---:|---|
| `0x0042E2BF` | 189119 | 983 | `816b9a400888304cddf787041e376fe009cbdbec0d426b38bf8f23be75d40ac8` |
| `0x00406A00` | 27136 | 18 | `45713e9071f67175de33bff8071f8358847bcf139043b6cfbb69f2f7744aaa47` |
| `0x0042FEF0` | 196336 | 83 | `7bc9223189ff9857ef8bcb775ae6a6aec1dffe2f66e3631cb95f3c7d32d675d5` |
| `0x00430170` | 196976 | 17 | `31f46434012fd7ba8bbf8b14308e9a805714de8400146ddb80ab47e40c27d964` |
| `0x00430190` | 197008 | 29 | `8e254846ffcac263c367901dff6450f94b61c566b4e29c0a90f464ecdeea2ffc` |
| `0x004B0338` | 721720 | 8 | `af5570f5a1810b7af78caf4bc70a660f0df51e42baf91d4de5b2328de0e83dfc` |

The 983-byte range includes 967 function bytes through `0x0042E685` and the
16-byte switch table at `0x0042E686`. The last row is the binary64 zero constant.
The runner verifies the executable hash and PE-derived mapping for every pin;
original game code is read in place and passed to the adapter over stdin.
Additional attachment/query pins and evidence are recorded in
[actor_behavior_attachment.md](actor_behavior_attachment.md).

## Static behavior

The initial actor field `+0x50` determines one pass, or two when it equals 2.
The pass count is captured before callbacks execute. A state's phase byte at
`+0x10` is tested exactly: 1 runs on every pass, 0 runs only on the final pass,
and other byte values skip updates. It is not a Boolean field.

Each pass gives the primary state at actor `+0x340` a fresh copy of the supplied
elapsed time. While that copy is greater than zero, the scheduler caches the
current primary pointer and calls virtual slot `+4` with a mutable time
pointer. Its result controls the following actions:

| Result | Primary state | Extra state |
|---|---|---|
| 0 | Retry with remaining time and current primary pointer | Advance to next node |
| 1 | Destroy cached state through slot `+0`, argument 1 | Call slot `+0xC`, destroy with argument 1, unlink and release node |
| 2 | Return false immediately | Return false immediately |
| 3 | Set remaining primary time to zero | Advance to next node |
| Other | Retry with remaining time and current primary pointer | Retry the same node with a fresh elapsed-time copy |

State replacement belongs to callbacks; result 1 alone does not install a
replacement primary pointer. Cached-state destruction happens after the
callback, even when that callback has changed the primary pointer.

Actor flag `+0x5D` takes precedence over primary phase gating. The observed
order is global event `0x0044535B` (signed actor ID, global `0x004E32CC`, zero
flag), allocation/construction of a new state, attachment `0x0042E696`, clearing
`+0x5D`, and destruction of the cached primary. The source exposes the event,
creation and attachment as distinct typed callbacks. A cached null primary is
accepted on this forced path because its destruction is guarded.

The extra-state list at actor `+0x374` is visited after the primary loop. Each
eligible extra state receives the original elapsed time independently, even
when it is zero, negative or NaN. Extra result 1 calls leave and destruction
before reading the next node; it then unlinks the current node and releases it.
The source preserves that order and uses an intrusive list with explicit
begin/end sentinels. No original object layout is asserted for the portable
`ActorBehaviorUpdate` representation.

The code does not impose a progress limit. A primary callback that repeatedly
returns 0 without reducing time or changing state can loop forever. An extra
callback that repeatedly returns a value outside 0..3 can also loop forever.
These original semantics are retained. Callbacks must keep referenced nodes
and states valid until their corresponding lifetime boundary.

## Unit tests

`test_actor_update` passes on x64 and x86. Hand-written trace expectations
check phase gating across two passes, cached primary destruction, extra
leave/destroy/unlink order, forced transition order, immediate false returns,
non-Boolean phase bytes, fresh extra-state time, and one retry for result 4.
Zero, negative and NaN elapsed time skip the primary loop while still updating
eligible extra states. A forced flag consequently remains set in these cases.

## Native differential comparison

`native_actor_update_oracle` passed **20,000 deterministic trace/state
comparisons** on 2026-10-29 UTC. It executes the original scheduler bytes,
original ID getter, original list begin/end helpers, original unlink helper,
original attachment routine and original function epilogue. It compares the Boolean result, ordered callback
trace including exact input-time bits, remaining list order, primary identity,
and forced flag against the source.

The corpus varies one/two passes; phase bytes 0, 1, 2 and 255; results 0..4;
0..5 extra states; ordinary and forced transitions; null cached primary on the
forced path; and positive, zero, negative, NaN and infinite elapsed times.
Scripted callbacks consume time or change their result after the first call,
so the comparison terminates without changing scheduler semantics.

Explicit adapters replace these unrecovered boundaries:

- Behavior virtual slots `+0`, `+4`, `+8`, `+0xC` use deterministic scripts and traces.
- `0x0044535B` records and checks the forced-event arguments.
- `0x0049C1A8` and `0x00416BF0` supply and return a prepared state; allocation
  failure and the real forced-state constructor are not compared.
- `0x0042E696` now executes unchanged. Its entry/leave virtual calls use the
  scripted state callbacks; the source side calls `attach_actor_behavior`.
- `0x0042FF50` records node release after the original unlink helper runs;
  the oracle keeps node storage alive for comparison.
- At `0x0042E58C`, 13 bytes are replaced with `mov ecx,[ebp-0x5C]`, a call to
  the finalization adapter, and a jump to original `mov al,1` at `0x0042E674`.
  Thus the terminal animation/terrain block is intentionally not executed.

The separate [finalization recovery](actor_finalization.md) now executes that
unchanged terminal block and its sample/model helpers in its own native oracle.
This scheduler oracle retains its original adapter to isolate scheduling.

No scheduler instruction before `0x0042E58C` or its switch table is patched.
Only the low return byte is compared; high EAX bytes are not a meaningful
Boolean result. Exceptions through original MSVC exception frames are not
tested. The comparison is offline native-byte execution with these adapters,
not an end-to-end running-game comparison.

```powershell
python tools/run_native_actor_update_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_actor_update_oracle.exe
```

```text
native actor scheduler passed 20000 trace/state comparisons; attachment/query passed 10000 scenarios (70000 attachments, 630000 queries)
```
